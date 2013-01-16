/**********************************************************************************************/		
/* Parser.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Unicode.h"
#include "Utils.h"
#include "Value.h"
#include "Type.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void BindValue( 
	sqlite3_stmt*	stmt,
	int				index,
	const Value&	vl )
{
	switch( vl.mType )
	{
		case kUnknown	: sqlite3_bind_null( stmt, index ); break;
		case kBLOB		: /* TODO : sqlite3_bind_blob */ break;
		case kDouble	: sqlite3_bind_double( stmt, index, vl.ToDouble() ); break;
		case kInt		: sqlite3_bind_int( stmt, index, vl.ToInt() ); break;

		case kText	:
		{
			vector<char> buf;
			ConvertToUTF8( vl.ToString(), buf );
			sqlite3_bind_text( stmt, index, &buf[ 0 ], -1, SQLITE_TRANSIENT );
		}
		break;
		
		default:;
	}
}

/**********************************************************************************************/
EValueType ConvertTypeNameToEnum( const wstring& rname )
{
	wstring lname = Lower( rname );
	
	if( lname.find( L"int"  ) != NPOS ||
		lname.find( L"bool" ) != NPOS )
		return kInt;
	
	if( lname.find( L"char" ) != NPOS ||
		lname.find( L"clob" ) != NPOS ||
		lname.find( L"text" ) != NPOS )
		return kText;
	
	if( lname.find( L"blob" ) != NPOS )
		return kBLOB;
	
	if( lname.find( L"real" ) != NPOS ||
		lname.find( L"floa" ) != NPOS ||
		lname.find( L"doub" ) != NPOS ||
		lname.find( L"num" ) != NPOS )
		return kDouble;
	
	return kText;
}

/**********************************************************************************************/
bool IsValidName( const wstring& name )
{
	size_t size = name.length();
	if( !size )
		return false;
		
	// Quoted identifier
	if( size > 2 && name[ 0 ] == '"' && name[ size - 1 ] == '"' )
	{
		for( size_t i = 1 ; i < size - 1 ; ++i )
		{
			if( name[ i ] == '"' )
				return false;
		}
		
		return true;
	}
	
	for( size_t i = 0 ; i < size ; ++i )
	{
		char ch = (char) name[ i ];
		if( ( ch < 'a' || ch > 'z' ) &&
			( ch < 'A' || ch > 'Z' ) &&
			( i == 0 || ( ( ch < '0' || ch > '9' ) && !strchr( "_$#", ch ) ) ) )
		{
			return false;
		}
	}

	return true;
}

/**********************************************************************************************/
EValueType GetColumnType(
	sqlite3_stmt*	stmt,
	int				index )
{
	const char* s = sqlite3_column_decltype( stmt, index );
	if( s )
	{
		wstring str;
		FROM_UTF8( s, str );
		return ConvertTypeNameToEnum( str );
	}

	return (EValueType) sqlite3_column_type( stmt, index );
}

/**********************************************************************************************/
EValueType GetColumnType(
	sqlite3*		db,
	const wstring&	table,
	const wstring&	field )
{
	EValueType res = kUnknown;
	wstring query = L"SELECT [" + field + L"] FROM [" + table + L"] LIMIT 1";
	
	sqlite3_stmt* stmt = SqliteSelect( db, query );
	if( stmt )
	{
		res = GetColumnType( stmt, 0 );
		FINALIZE_STATEMENT( stmt );
	}
	
	return res;
}

/**********************************************************************************************/
bool IsValidType( const wstring& name )
{
	size_t size = name.length();
	if( !size )
		return false;
		
	for( size_t i = 0 ; i < size ; ++i )
	{
		char ch = (char) name[ i ];
		if( ( ch < 'a' || ch > 'z' ) &&
			( ch < 'A' || ch > 'Z' ) &&
			( i == 0 || ( ( ch < '0' || ch > '9' ) && ch != '(' && ch != ')' && ch != ',' && ch != '_' ) ) )
		{
			return false;
		}
	}

	return true;
}

/**********************************************************************************************/
void ReadTableStructure(
	Type&			type,
	sqlite3*		db,
	const wstring&	query )
{
	type.mNotNULL = false;
	
	sqlite3_stmt* stmt = SqliteSelect( db, query );
	if( stmt )
	{
		int count = sqlite3_column_count( stmt );
		for( int i = 0 ; i < count ; ++i )
		{
			const char*	name = sqlite3_column_name( stmt, i );

			Member fld;
			FROM_UTF8( name, fld.mName );
			ToLower( fld.mName );
	
			fld.mNotNULL	= false;
			fld.mType		= GetColumnType( stmt, i );
			type.mMembers.push_back( fld );
		}

		FINALIZE_STATEMENT( stmt );
	}
}

/**********************************************************************************************/
void ReadValue(
	sqlite3_stmt*	stmt,
	Value&			vl,
	int				col,
	EValueType		type )
{
	if( type == kUnknown )
		type = GetColumnType( stmt, col );
	
	switch( type )
	{
		case kDouble	: vl = sqlite3_column_double( stmt, col ); break;
		case kInt		: vl = sqlite3_column_int( stmt, col ); break;

		case kText		:
		{
			vl.mType = kText;
			vl.mStr.clear();
			
			const char* s = (const char*) sqlite3_column_text( stmt, col );
			FROM_UTF8( s, vl.mStr );
		}
		break;
		
		case kBLOB		: /* TODO */
	
		default:
			vl.mType = kUnknown;
			break;
	}
}

/**********************************************************************************************/
int SqliteExecute(
	sqlite3*		db,
	const wstring&	query )
{
	vector<char> buf;
	ConvertToUTF8( query, buf );

	const char* pq = &buf[ 0 ];
	SkipTabulation( pq );
	
	sqlite3_stmt* stmt = NULL;
	int qres = sqlite3_prepare_v2( db, pq, -1, &stmt, &pq );
	if( qres == SQLITE_OK ) 
		sqlite3_step( stmt );
		
	FINALIZE_STATEMENT( stmt );
	return qres;
}

/**********************************************************************************************/
Cursor* SqliteQuery(
	sqlite3*		db,
	const wstring&	query )
{
	sqlite3_stmt* stmt = SqliteSelect( db, query );
	return stmt ? new Cursor( stmt ) : NULL;
}

/**********************************************************************************************/
sqlite3_stmt* SqliteSelect(
	sqlite3*		db,
	const wstring&	query )
{
	vector<char> buf;
	ConvertToUTF8( query, buf );

	const char* pq = &buf[ 0 ];
	SkipTabulation( pq );

	sqlite3_stmt* stmt = NULL;

	int qres = sqlite3_prepare_v2( db, pq, -1, &stmt, &pq );
	if( qres == SQLITE_OK )
	{
		int ccount = sqlite3_column_count( stmt );
		if( ccount > 0 )
			return stmt;
	}

	FINALIZE_STATEMENT( stmt );
	return NULL;
}

/**********************************************************************************************/
#define ADD_TOKEN( x, y ) list[ L ## #x ] = y;

/**********************************************************************************************/
EToken ToToken( const wstring& rs )
{
	wstring s = Lower( rs );
	
	static map<wstring,EToken> list;
	if( list.empty() )
	{
		ADD_TOKEN( alter		, kTokenAlter );
		ADD_TOKEN( analyze		, kTokenAnalyze );
		ADD_TOKEN( attach		, kTokenAttach );
		ADD_TOKEN( begin		, kTokenBegin );
		ADD_TOKEN( case			, kTokenCase );
		ADD_TOKEN( close		, kTokenClose );
		ADD_TOKEN( commit		, kTokenCommit );
		ADD_TOKEN( create		, kTokenCreate );
		ADD_TOKEN( cursor		, kTokenCursor );
		ADD_TOKEN( dbms_output	, kTokenOutput );
		ADD_TOKEN( declare		, kTokenDeclare );
		ADD_TOKEN( default		, kTokenDefault );
		ADD_TOKEN( delete		, kTokenDelete );
		ADD_TOKEN( detach		, kTokenDetach );
		ADD_TOKEN( drop			, kTokenDrop );
		ADD_TOKEN( else			, kTokenElse );
		ADD_TOKEN( elsif		, kTokenElsIf );
		ADD_TOKEN( end			, kTokenEnd );
		ADD_TOKEN( exception	, kTokenException );
		ADD_TOKEN( exec			, kTokenExec );
		ADD_TOKEN( execute		, kTokenExecute );
		ADD_TOKEN( exit			, kTokenExit );
		ADD_TOKEN( explain		, kTokenExplain );
		ADD_TOKEN( fetch		, kTokenFetch );
		ADD_TOKEN( for			, kTokenFor );
		ADD_TOKEN( goto			, kTokenGoto );
		ADD_TOKEN( if			, kTokenIf );
		ADD_TOKEN( insert		, kTokenInsert );
		ADD_TOKEN( into			, kTokenInto );
		ADD_TOKEN( loop			, kTokenLoop );
		ADD_TOKEN( not			, kTokenNot );
		ADD_TOKEN( null			, kTokenNull );
		ADD_TOKEN( off			, kTokenOff );
		ADD_TOKEN( on			, kTokenOn );
		ADD_TOKEN( open			, kTokenOpen );
		ADD_TOKEN( pragma		, kTokenPragma );
		ADD_TOKEN( print		, kTokenPrint );
		ADD_TOKEN( reindex		, kTokenReindex );
		ADD_TOKEN( release		, kTokenRelease );
		ADD_TOKEN( replace		, kTokenReplace );
		ADD_TOKEN( return		, kTokenReturn );
		ADD_TOKEN( reverse		, kTokenReverse );
		ADD_TOKEN( rollback		, kTokenRollback );
		ADD_TOKEN( rowtype		, kTokenRowType );
		ADD_TOKEN( savepoint	, kTokenSavePoint );
		ADD_TOKEN( select		, kTokenSelect );
		ADD_TOKEN( serveroutput	, kTokenServerOutput );
		ADD_TOKEN( set			, kTokenSet );
		ADD_TOKEN( subtype		, kTokenSubType );
		ADD_TOKEN( then			, kTokenThen );
		ADD_TOKEN( type			, kTokenType );
		ADD_TOKEN( update		, kTokenUpdate );
		ADD_TOKEN( vacuum		, kTokenVacuum );
		ADD_TOKEN( when			, kTokenWhen );
		ADD_TOKEN( while		, kTokenWhile );
		ADD_TOKEN( <			, kTokenLess );
		ADD_TOKEN( ;			, kTokenSemicolon );
		ADD_TOKEN( \n			, kTokenSeparator );
		ADD_TOKEN( \r			, kTokenSeparator );
		ADD_TOKEN( \t			, kTokenSeparator );
		
		list[ L" " ] = kTokenSeparator;
		list[ L"(" ] = kTokenBraceLeft;
		list[ L")" ] = kTokenBraceRight;
	}
	
	map<wstring,EToken>::iterator it = list.find( s );
	return it != list.end() ?
		it->second :
		kTokenUnknown;
}

/**********************************************************************************************/
void TrimLeft( wstring& s )
{
	size_t len = s.size();
	for( size_t i = 0 ; i < len ; ++i )
	{
		if( !isspace( s[ i ] ) )
		{
			if( i )
				s.erase( 0, i );

			return;
		}
	}

	s.clear();
}

/**********************************************************************************************/
void TrimRight( wstring& s )
{
	int i = (int) s.length() - 1;
	for( ; i >= 0 ; --i )
	{
		if( !isspace( s[ i ] ) )
		{
			s.resize( i + 1 );
			return;
		}
	}
	
	s.clear();
}


/**********************************************************************************************/
SP_NAMESPACE_END
