/**********************************************************************************************/		
/* Statement.cpp																			  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "GlobalVariables.h"
#include "Parser.h"
#include "Statement.h"
#include "Unicode.h"
#include "Utils.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Statement::Statement( 
	Parser&			parser,
	const wstring&	text )
:
// References
	mpDB	( parser.mVM.mpDB ),
	mpStmt	( NULL ),
	mpVM	( &parser.mVM )
{
	mpText = new wstring( text );
	
	Parse( parser );
	Prepare();

	if( !mpStmt )
	{
		const char* err = sqlite3_errmsg( mpDB );
		if( err && !strncmp( err, "near", 4 ) )
		{
			FROM_UTF8( err, parser.mErr );
			
			parser.mErr += L", at line " + IntToStr( parser.mLine );
			longjmp( parser.mJBuf, 1 );
		}
	}

	FINALIZE_STATEMENT( mpStmt );
}

/**********************************************************************************************/
Statement::~Statement( void )
{
	FINALIZE_STATEMENT( mpStmt );
	delete mpText;
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Statement* Statement::Clone( VMachine* vm )
{
	Statement* res = new Statement;
	
	// References
	res->mpDB	= mpDB;
	res->mpVM	= vm;
	
	// Cached data
	res->mBinds	= mBinds;
	res->mQuery	= mQuery;

	return res;
}

/**********************************************************************************************/
bool Statement::Execute( void )
{
	if( !mpStmt )
		Prepare();
	
	if( !mpStmt )
		return false;

	ApplyBinds();
	int res = sqlite3_step( mpStmt );
	
	FINALIZE_STATEMENT( mpStmt );
		
	mpVM->mVariables[ mpVM->mVarFound		]->AssignValue( sqlite3_changes( mpDB ) > 0 );
	mpVM->mVariables[ mpVM->mVarNotFound	]->AssignValue( sqlite3_changes( mpDB ) == 0 );
	mpVM->mVariables[ mpVM->mVarRowCount	]->AssignValue( sqlite3_changes( mpDB ) );
		
	return res == SQLITE_OK || res == SQLITE_DONE || res == SQLITE_ROW;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Statement::ApplyBinds( void )
{
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
void Statement::Parse( Parser& parser )
{
	wstring	query;
	wstring& text = *mpText;
	
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
				if( ch < '0' || ch > '9' )
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

				query += ':';
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
void Statement::Prepare( void )
{
	// Reset old statement
	FINALIZE_STATEMENT( mpStmt );

	const char *pq = &mQuery[ 0 ];
	SkipTabulation( pq );

	int qres = sqlite3_prepare_v2( mpDB, pq, -1, &mpStmt, &pq );
	if( qres != SQLITE_OK || !mpStmt ) 
		FINALIZE_STATEMENT( mpStmt );
}


//////////////////////////////////////////////////////////////////////////
// private constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Statement::Statement( void )
:
// References
	mpStmt( NULL ),
	
// Properties	
	mpText( NULL )
{
	// Constructor to be used for clone
}


/**********************************************************************************************/
SP_NAMESPACE_END
