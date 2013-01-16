/**********************************************************************************************/		
/* Core.cpp																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SP
#include "core.h"
#include "CoreFunctions.h"
#include "Cursor.h"
#include "Field.h"
#include "Function.h"
#include "GlobalVariables.h"
#include "Unicode.h"
#include "Utils.h"
#include "checker.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
#define DB sqlite3_context_db_handle( ctx )

/**********************************************************************************************/
static void RegisterFunction( sqlite3* db, const wstring& name, const wstring& text );


//////////////////////////////////////////////////////////////////////////
// callbacks
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
FUNCTION_NC( sp_change )
{
	sqlite3* db = DB;
	
	STRING_FROM_PARAM( s1, 0 )
	STRING_FROM_PARAM( s2, 1 )

	if( !s1 || !s2 )
		RETURN_ERROR( "Invalid arguments! Must be two strings - name and text." );
		
	TO_WTEXT( name, s1 );
	TO_WTEXT( text, s2 );

	// Check arguments
	
	if( !IsValidName( name ) )
		RETURN_ERROR( "Invalid name!" );

	RESULT_INT( 1 );
	
	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
	{
		func->mCompiled = false;
		func->mText = text;		
		
		QuoteString( name );
		QuoteString( text );

		wstring query = L"UPDATE sqlite_sp_functions SET \"text\" = '" + text + L"' WHERE \"name\"='" + name + L"'";
		SqliteExecute( db, query );
		
		return;
	}

	RETURN_ERROR( "Function not found" );
}

/**********************************************************************************************/
FUNCTION( sp_compile )
{
	STRING_FROM_PARAM( s, 0 )
	if( !s )
		RETURN_ERROR( "Invalid argument! Must be function's name." );

	TO_WTEXT( name, s )

	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
	{
		RESULT_INT( 1 );
		
		func->Compile( ctx );
		if( c > 1 && INT( 1 ) && func->mCompiled )
		{	
			wstring warns = Checker().Check( func->mVM );
			if( warns.length() )
				RESULT_WTEXT( warns );
		}
	}
	else
	{
		RETURN_ERROR( "Function has been removed or doesn't exist!" );
	}
}

/**********************************************************************************************/
FUNCTION_NC( sp_create )
{
	sqlite3* db = DB;
	
	STRING_FROM_PARAM( s1, 0 )
	STRING_FROM_PARAM( s2, 1 )

	if( !s1 || !s2 )
		RETURN_ERROR( "Invalid arguments! Must be two strings - name and text." );
		
	TO_WTEXT( name, s1 );
	TO_WTEXT( text, s2 );

	// Check arguments
	
	if( !IsValidName( name ) )
		RETURN_ERROR( "Invalid name!" );

	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
		RETURN_ERROR( "Function already exists!" );
	
	// Register callback

	RegisterFunction( db, name, text );
	
	// Add in database
	
	QuoteString( name );
	QuoteString( text );
	
	wstring query = L"INSERT INTO sqlite_sp_functions( \"name\", \"text\" ) VALUES ( '" + name + L"', '" + text + L"' )";
	SqliteExecute( db, query );
	
	RESULT_INT( 1 );
}

/**********************************************************************************************/
static void DestroyFunction( void* ptr )
{
	Function::Destroy( ptr );
}

/**********************************************************************************************/
FUNCTION_NC( sp_drop )
{
	// Convert arguments to std::wstring

	STRING_FROM_PARAM( s, 0 );
	if( !s )
		RETURN_ERROR( "Invalid argument! Must be function's name." );

	TO_WTEXT( name, s );
	QuoteString( name );
	
	// Drop from database

	sqlite3*		db		= DB;
	const wstring	query	= L"DELETE FROM sqlite_sp_functions WHERE \"name\"='" + name + L"'";
	
	SqliteExecute( db, query );
	
	// Drop function itself

	Function* func = Function::FindFunction( db, name );
	if( func && func->mAttached )
	{
		func->mAttached = false;
		RESULT_INT( 1 );
	}
	else
	{
		RETURN_ERROR( "Function doesn't exist!" );
	}
}

/**********************************************************************************************/
FUNCTION( execute )
{
	Function* func = (Function*) sqlite3_user_data( ctx );
	if( func && func->mAttached )
	{
		RESULT_NULL;
		func->Execute( ctx, c, v );
	}
	else
	{
		RETURN_ERROR( "Function has been removed" );
	}
}

/**********************************************************************************************/
FUNCTION_NC( sp_explain )
{
	// Convert arguments to std::wstring

	STRING_FROM_PARAM( s, 0 );
	if( !s )
		RETURN_ERROR( "Invalid argument! Must be function's name." );

	TO_WTEXT( name, s );

	// Find function by name and call Explain() method

	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
		func->Explain( ctx );
	else
		RETURN_ERROR( "Function has been removed or doesn't exist!" );
}

/**********************************************************************************************/
FUNCTION_NC( sp_list )
{
	(void) v;
	sqlite3* db = DB;
	
	sqlite3_stmt* stmt = SqliteSelect( db, L"SELECT \"name\" FROM sqlite_sp_functions" );
	if( !stmt )
		RETURN_ERROR( "Unable to rad internal data!" );
	
	string res;

	int step = sqlite3_step( stmt );
	while( step == SQLITE_ROW )		
	{
		const char* text = (const char*) sqlite3_column_text( stmt, 0 );
		if( text )
		{
			if( res.length() )
				res += '\n';

			res += text;
		}

		step = sqlite3_step( stmt );
	}

	FINALIZE_STATEMENT( stmt );
	RESULT_TEXT( res.length() ? &res[ 0 ] : "" );
}

/**********************************************************************************************/
FUNCTION_NC( sp_load )
{
	sqlite3* db = DB;
	
	// Convert arguments to std::wstring
	
	STRING_FROM_PARAM( s1, 0 )
	STRING_FROM_PARAM( path, 1 )
	
	if( !s1 || !path )
		RETURN_ERROR( "Invalid arguments! Must be two strings - name and file's path." );
		
	TO_WTEXT( name, s1 )
	
	vector<char> buf;
	
	FILE* f = fopen( path, "r" );
	if( f )
	{
		fseek( f, 0, SEEK_END );
		size_t size = (size_t) ftell( f );
		fseek( f, 0, SEEK_SET );
		
		buf.resize( size );
		buf.resize( fread( &buf[ 0 ], 1, size, f ) );
		buf.push_back( 0 );
		
		fclose( f );
	}
	else
	{
		RETURN_ERROR( "Unable to open file!" );
	}
	
	TO_WTEXT( text, &buf[ 0 ] );

	// Check arguments
	
	if( !IsValidName( name ) )
		RETURN_ERROR( "Invalid name!" );

	if( text.empty() )
		RETURN_ERROR( "Empty function's text!" );

	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
		RETURN_ERROR( "Function already exists!" );
	
	// Register callback

	RegisterFunction( db, name, text );
	
	// Add in database
	
	QuoteString( name );
	QuoteString( text );
	
	wstring query = L"INSERT INTO sqlite_sp_functions( \"name\", \"text\" ) VALUES ( '" + name + L"', '" + text + L"' )";
	SqliteExecute( db, query );
	
	RESULT_INT( 1 );
}

/**********************************************************************************************/
FUNCTION_NC( sp_rename )
{
	sqlite3* db = DB;
	
	// Convert arguments to std::wstring
	
	STRING_FROM_PARAM( s1, 0 );
	STRING_FROM_PARAM( s2, 1 );
	
	if( !s1 || !s2 )
		RETURN_ERROR( "Invalid arguments! Must be two strings - current name and new function's name." );
		
	TO_WTEXT( oldn, s1 );
	TO_WTEXT( newn, s2 );

	// Check arguments
	
	if( !IsValidName( newn ) )
		RETURN_ERROR( "Invalid function's name!" );

	Function* oldFunc = Function::FindFunction( db, oldn );
	if( !oldFunc )
		RETURN_ERROR( "Function doesn't exist!" );

	// Create or attach new function
	
	Function* newFunc = Function::FindFunction( db, newn );
	if( newFunc )
	{
		if( newFunc->mAttached )
		{
			RETURN_ERROR( "Already exists function with this name!" );
		}
		else
		{
			newFunc->mAttached = true;
			newFunc->mCompiled = false;
			newFunc->mText = oldFunc->mText;
		}
	}
	else
	{
		RegisterFunction( db, newn, oldFunc->mText );
	}

	// 'Drop' old
	
	oldFunc->mAttached = false;
	
	// Change in database
	
	QuoteString( newn );
	QuoteString( oldn );
	
	wstring query = L"UPDATE sqlite_sp_functions SET \"name\" = '" + newn + L"' WHERE \"name\"='" + oldn + L"'";
	SqliteExecute( db, query );
	
	RESULT_INT( 1 );
}

/**********************************************************************************************/
FUNCTION_NC( sp_save )
{
	// Convert arguments to std::wstring
	
	STRING_FROM_PARAM( s1, 0 )
	STRING_FROM_PARAM( path, 1 )
	
	if( !s1 || !path )
		RETURN_ERROR( "Invalid arguments! Must be two strings - name and file's path." );
		
	TO_WTEXT( name, s1 )
	
	Function* func = Function::FindFunction( DB, name );
	if( !func || !func->mAttached )
		RETURN_ERROR( "Function doesn't exist or has been removed!" );
	
	FILE* f = fopen( path, "w" );
	if( !f )
		RETURN_ERROR( "Unable to open file to write!" );

	vector<char> buf;
	ConvertToUTF8( func->mText, buf );
	fwrite( &buf[ 0 ], 1, buf.size() - 1, f );
	fclose( f );

	RESULT_INT( 1 );
}

/**********************************************************************************************/
FUNCTION_NC( sp_show )
{
	// Convert arguments to std::wstring

	STRING_FROM_PARAM( s, 0 );
	if( !s )
		RETURN_ERROR( "Invalid argument! Must be function's name." );
	
	TO_WTEXT( name, s );
	
	// Get functions text

	Function* func = Function::FindFunction( DB, name );
	if( func && func->mAttached )
	{
		vector<char> buf;
		ConvertToUTF8( func->mText, buf );

		sqlite3_result_text( ctx, &buf[ 0 ], buf.size() - 1, SQLITE_TRANSIENT );
	}
	else
	{
		RETURN_ERROR( "Function doesn't exist or has been removed!" );
	}
}


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static void CreateTable( sqlite3* db )
{
	static const wstring query = L"CREATE TABLE IF NOT EXISTS sqlite_sp_functions( name TEXT, text TEXT );";
	SqliteExecute( db, query );
}

/**********************************************************************************************/
static void InitCore( sqlite3* db )
{
	REGISTER( sp_change				, 2 );	
	REGISTER( sp_compile			, 1 );
	REGISTER( sp_compile			, 2 );
	REGISTER( sp_create				, 2 );	
	REGISTER( sp_drop				, 1 );	
	REGISTER( sp_explain			, 1 );	
	REGISTER( sp_list				, 0 );	
	REGISTER( sp_load				, 2 );	
	REGISTER( sp_rename				, 2 );	
	REGISTER( sp_save				, 2 );	
	REGISTER( sp_show				, 1 );	
}

/**********************************************************************************************/
static void RegisterFunction(
	sqlite3*		db,
	const wstring&	name,
	const wstring&	text )
{
	vector<char> buf;
	ConvertToUTF8( name, buf );

	// Try to find existing function by name
	// TODO: argc

	Function* func = Function::FindFunction( db, name );
	if( func )
	{
		func->mAttached = true;
		func->mCompiled = false;
		func->mText		= text;
		
		return;
	}
		
	// Create new function
	
	func = new Function( db, name, text );
	
	sqlite3_create_function_v2(
		db,
		&buf[ 0 ],
		-1,
		SQLITE_UTF8,
		func,
		sp_execute,
		NULL,
		NULL,
		DestroyFunction );
}

/**********************************************************************************************/
void sp_init( sqlite3* db )
{
	// Constants
	AddGlobalConstant( L"false"	, 0 );
	AddGlobalConstant( L"true"	, 1 );

	// Register additional functions
	InitCore( db );
	RegisterFunctions( db );

	// Enable to change system tables to have ability to use sqlite_functions name
	SqliteExecute( db, L"PRAGMA writable_schema = 1" );

	// Create tables
	CreateTable( db );
	
	// Read descriptions of functions from special table
	Cursor* cursor = SqliteQuery( db, L"SELECT * FROM sqlite_sp_functions" );
	if( !cursor )
		return;
	
	// References to fields
	Field* fname = cursor->get_Field( L"name" );
	Field* ftext = cursor->get_Field( L"text" );
	
	if( !fname || !ftext )
	{
		delete cursor;
		return;
	}
	
	// Read descriptions and create functions
	bool step = cursor->NextRecord();
	while( step )
	{
		if( fname->mStr.size() )
			RegisterFunction( db, fname->mStr, ftext->mStr );

		step = cursor->NextRecord();
	}
	
	delete cursor;
}


/**********************************************************************************************/
SP_NAMESPACE_END
