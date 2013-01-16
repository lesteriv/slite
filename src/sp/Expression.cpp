/**********************************************************************************************/		
/* Expression.cpp																			  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "core.h"
#include "Expression.h"
#include "GlobalVariables.h"
#include "Parser.h"
#include "Unicode.h"
#include "Utils.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START



//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static sqlite3* GetTemporaryDatabase( void )
{
	static sqlite3* db = NULL;
	if( !db )
	{
		sqlite3_open_v2( ":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, 0 );	
		sqlite3_exec( db, "PRAGMA journal_mode=OFF", 0, 0, 0 );
		sqlite3_exec( db, "PRAGMA synchronous = 0", 0, 0, 0 );
		
		sp_init( db );
	}
	
	return db;
}

/**********************************************************************************************/
static bool IsStaticIdent( const wstring& id )
{
#define IDENT( x ) idents.insert( L ## #x );
	
	static hset<wstring> idents;
	if( idents.empty() )
	{
		IDENT( abs )
		IDENT( acos )
		IDENT( and )
		IDENT( as )
		IDENT( ascii )
		IDENT( asciistr )
		IDENT( asin )
		IDENT( atan )
		IDENT( atan2 )
		IDENT( avg )
		IDENT( between )
		IDENT( bitand )
		IDENT( bit_to_num )
		IDENT( case )
		IDENT( cast )
		IDENT( ceil )
		IDENT( chr )
		IDENT( coalesce )
		IDENT( concat )
		IDENT( cos )
		IDENT( cosh )
		IDENT( current_date )
		IDENT( current_time )
		IDENT( current_timestamp )
		IDENT( date )
		IDENT( datetime )
		IDENT( decode )
		IDENT( distinct )
		IDENT( else )
		IDENT( end )
		IDENT( escape )
		IDENT( exp )
		IDENT( floor )
		IDENT( glob )
		IDENT( greatest )
		IDENT( hex )
		IDENT( hextoraw )
		IDENT( ifnull )
		IDENT( in )
		IDENT( initcap )
		IDENT( instr )
		IDENT( is  )
		IDENT( isnull )
		IDENT( julianday )
		IDENT( least )
		IDENT( length )
		IDENT( like )
		IDENT( ln )
		IDENT( lnnvl )
		IDENT( log )
		IDENT( lower )
		IDENT( lpad )
		IDENT( ltrim )
		IDENT( match )
		IDENT( mod )
		IDENT( nanvl )
		IDENT( not )
		IDENT( notnull )
		IDENT( null )
		IDENT( nullif )
		IDENT( numtodsinterval )
		IDENT( numtoyminterval )
		IDENT( nvl )
		IDENT( nvl2 )
		IDENT( power )
		IDENT( quote )
		IDENT( random )
		IDENT( randomblob )
		IDENT( rawtohex )
		IDENT( regexp )
		IDENT( remainder )
		IDENT( replace )
		IDENT( round )
		IDENT( rpad )
		IDENT( rtrim )
		IDENT( sign )
		IDENT( sin )
		IDENT( sinh )
		IDENT( sqrt )
		IDENT( strftime )
		IDENT( substr )
		IDENT( tan )
		IDENT( tanh )
		IDENT( then )
		IDENT( time )
		IDENT( translate )
		IDENT( trim )
		IDENT( trunc )
		IDENT( typeof )
		IDENT( upper )
		IDENT( vsize )
		IDENT( when  )
		IDENT( zeroblob )
	}
	
	return idents.find( id ) != idents.end();
}

//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Expression::Expression( 
	Parser&			parser,
	const wstring&	text,
	EValueType		type )
:
// References
	mpDB			( parser.mVM.mpDB ),
	mpStmt			( NULL ),
	mpVM			( &parser.mVM ),
	
// Cached data
	mVariableIndex	( -1 )
{
	mpText = new wstring( text );
	mValue.mType = type;
	
	Parse( parser );

	// Check for errors
	if( mKind == kExpression )
	{
		Prepare();
		
		if( !mpStmt )
		{
			FROM_UTF8( sqlite3_errmsg( mPure ? GetTemporaryDatabase() : mpDB ), parser.mErr );

			parser.mErr += L", at line " + IntToStr( parser.mLine );
			longjmp( parser.mJBuf, 1 );
		}
		
		if( !mPure )
			FINALIZE_STATEMENT( mpStmt );
	}
}

/**********************************************************************************************/
Expression::~Expression( void )
{
	// Is we just clone
	if( mpVM->mVariablesNames.size() )
	{
		FINALIZE_STATEMENT( mpStmt );
		delete mpText;
	}
}


//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
const Value& Expression::get_Value( void )
{
	switch( mKind )
	{
		case kConstant	:
			/* Nothing to do. */
			break;

		case kVariable	:
		{
			static Value null;
			Variable* vl = GetVariable();
			return vl->mIsNULL ? null : vl->mValue;
		}
	
		default:
		{
			if( !mpStmt )
				Prepare();
			
			if( mpStmt )
			{
				ApplyBinds();

				if( sqlite3_step( mpStmt ) != SQLITE_ROW )
					mValue.mType = kUnknown;
				else
					ReadValue( mpStmt, mValue );			

				if( !mPure )
					FINALIZE_STATEMENT( mpStmt );
			}
		}
	}
	
	return mValue;
}

/**********************************************************************************************/
vector<Value> Expression::get_Values( void )
{
	vector<Value> res;
	
	switch( mKind )
	{
		case kConstant	:
			/* Nothing to do. */
			res.push_back( mValue );
			break;

		case kVariable	:
			res.push_back( GetVariable()->mValue );
			break;
	
		default:
		{
			if( !mpStmt )
				Prepare();

			if( mpStmt )
			{
				ApplyBinds();

				if( sqlite3_step( mpStmt ) == SQLITE_ROW )
				{
					int cols = sqlite3_column_count( mpStmt );
					res.resize( cols );
					
					for( int i = 0 ; i < cols ; ++i )
						ReadValue( mpStmt, res[ i ], i );
				}
				
				if( !mPure )
					FINALIZE_STATEMENT( mpStmt );
			}
		}
	}
	
	return res;	
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Expression* Expression::Clone( VMachine* vm )
{
	Expression* res = new Expression;
	
	// References
	res->mpDB = mpDB;
	res->mpVM = vm;

	if( mPure )
	{
		res->mPure = true;
		res->mpStmt = mpStmt;
	}
	
	// Cached data
	res->mBinds	= mBinds;
	res->mKind	= mKind;
	res->mQuery	= mQuery;
	res->mValue	= mValue;
	res->mVariableIndex	= mVariableIndex;

	return res;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Expression::ApplyBinds( void )
{
	sqlite3_reset( mpStmt );

	size_t count = mBinds.size();
	for( size_t i = 0 ; i < count ; ++i )
	{
		Variable* var = mpVM->mVariables[ mBinds[ i ] ];
		if( var->mIsNULL )
			sqlite3_bind_null( mpStmt, i + 1 );
		else
			BindValue( mpStmt, i + 1, var->mValue );
	}
}

/**********************************************************************************************/
Variable* Expression::GetVariable( void )
{
	return mpVM->mVariables[ mVariableIndex ];
}

/**********************************************************************************************/
void Expression::Parse( Parser&	parser )
{
	// Try to detect simple expressions
	if( ParseAsVariable( parser )	||
		ParseAsNumber()				||
		ParseAsString() )
		return;

	mPure = true;
	mKind = kExpression;

	wstring		query	= L"SELECT ";
	wstring&	text	= *mpText;
	
	size_t len = text.length();
	for( size_t i = 0 ; i < len ; ++i )
	{
		wchar_t ch = text[ i ];
		
		// Number
		if( ch >= '0' && ch <= '9' )
		{
			query += ch;

			while( ++i < len )
			{
				ch = text[ i ];
				if( ch == '.' )
					;
				else if( ch < '0' || ch > '9' )
				{
					--i;
					break;
				}

				query += ch;
			}
			
			continue;
		}
		
		// Separator
		if( strchr( "+-*/<>= ()|,:\t\n", ch ) )
		{
			query += ch;
		}
		// String
		else if( ch == '\'' )
		{
			query += ch;

			while( ++i < len )
			{
				ch = text[ i ];
				query += ch;

				if( ch == '\'' )
					break;
			}				
		}
		// Must be identifier
		else
		{
			wstring ident;
			bool qt = false;
			
			if( ch == '"' )
			{
				qt = true;
				while( ++i < len )
				{
					ch = text[ i ];
					if( ch == '"' )
						break;
					
					ident += ch;
				}
			}
			else
			{
				ident += ch;

				while( ++i < len )
				{
					ch = text[ i ];

					if( !isdigit( ch ) && !isalpha( ch ) && ch != '_' && ch != '%' && ch != '.' )
					{
						--i;
						break;
					}

					ident += ch;
				}
			}

			ToLower( ident );
			
			Variable* var = parser.FindVariable( ident );
			if( var )
			{
				mBinds.push_back( mpVM->Index( var ) );

				query.push_back( ':' );
				query += IntToStr( mBinds.size() );
			}
			else
			{
				var = FindGlobalVariable( ident );
				if( var )
				{
					query += var->mValue.ToString();
				}
				else
				{
					if( mPure && !IsStaticIdent( ident ) )
						mPure = false;
					
					if( qt ) query += '"';
					query += ident;
					if( qt ) query += '"';
				}
			}
		}
	}

	ConvertToUTF8( query, mQuery );
}

/**********************************************************************************************/
bool Expression::ParseAsNumber( void )
{
	wstring& text = *mpText;

	size_t len = text.size();
	if( !len )
		return false;

	wchar_t ch = text[ 0 ];
	bool valid = 
		( ch >= '0' && ch <= '9' ) ||
		( ( ch == '-' || ch == '+' || ch == '.' || ch == ',' ) && len > 1 );

	if( !valid )
		return false;

	bool dot = ch == '.' || ch == ',';
		
	for( size_t i = 1 ; i < len ; ++i )
	{
		ch = text[ i ];
	
		valid = ( ch >= '0' && ch <= '9' ) || ch == '.' || ch == ',';
		if( !valid )
			return false;
			
		if( ch == '.' || ch == ',' )
		{
			if( dot )
				return false;
				
			dot = true;
		}
	}
	
	mKind = kConstant;
	
	if( dot || mValue.mType == kDouble )
		mValue = wcstod( text.c_str(), NULL );
	else
		mValue = (int) wcstol( text.c_str(), NULL, 10 );
		
	return true;
}

/**********************************************************************************************/
bool Expression::ParseAsString( void )
{
	wstring& text = *mpText;
	size_t	 len  = text.size();
	if( len < 2 )
		return false;
		
	if( text[ 0 ] != '\'' || text[ len - 1 ] != '\'' )
		return false;
		
	wstring str;
	str.reserve( len );
	
	for( size_t i = 1 ; i < len - 1 ; ++i )
	{
		wchar_t ch = text[ i ];
		if( ch == '\'' )
		{
			if( i == len - 2 || text[ i + 1 ] != '\'' )
				return false;
				
			str += '\'';
			++i; // Skip next quate
		}
		else
		{
			str += ch;
		}
	}
	
	mKind	= kConstant;
	mValue	= str;
	
	return true;
}

/**********************************************************************************************/
bool Expression::ParseAsVariable( Parser& parser )
{
	wstring vname = Lower( *mpText );
	
	Variable* var = parser.FindVariable( vname );
	if( var )
	{
		mVariableIndex = mpVM->Index( var );
		mKind = kVariable;
	}
	else
	{
		var = FindGlobalVariable( vname );
		if( var )
		{
			mKind = kConstant;
			mValue = var->mValue;			
		}
	}
	
	return var != NULL;
}

/**********************************************************************************************/
void Expression::Prepare( void )
{
	// Reset old statement
	FINALIZE_STATEMENT( mpStmt );

	const char* pq = &mQuery[ 0 ];
	SkipTabulation( pq );

	int qres = sqlite3_prepare_v2( mPure ? GetTemporaryDatabase() : mpDB, pq, -1, &mpStmt, &pq );
	if( qres != SQLITE_OK || !mpStmt || sqlite3_column_count( mpStmt ) < 1 ) 
		FINALIZE_STATEMENT( mpStmt );
}


//////////////////////////////////////////////////////////////////////////
// private constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Expression::Expression( void )
:
// References
	mpStmt( NULL ),
	
// Properties	
	mpText( NULL ),

// Flags
	mPure( false )
{
	// Constructor to be used for clone
}


/**********************************************************************************************/
SP_NAMESPACE_END
