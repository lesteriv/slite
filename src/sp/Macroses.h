/**********************************************************************************************/		
/* Macroses.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once


/**********************************************************************************************/
#define PLINE L", at line " + IntToStr( mLine )

/**********************************************************************************************/
#define BREAK( str ) \
	{ mErr = str; mErr += PLINE; longjmp( mJBuf, 1 ); }

/**********************************************************************************************/
#define CHECK_BLOCK											\
	ESection sc = GetSection();								\
	if( sc != kSectionBegin && sc != kSectionException )	\
		BREAK( L"Unexpected " + mToken + L" token" + PLINE )

/**********************************************************************************************/
#define CHECK_DUPLICATE( x ) \
	if( x ) BREAK( L"Duplicate declaration: " + mToken + PLINE );

/**********************************************************************************************/
#define CHECK_EMPTY \
	{ if( mToken.empty() ) { mErr = L"Unexpected end"; longjmp( mJBuf, 1 ); } }

/**********************************************************************************************/
#define CHECK_IDENT															\
	{																		\
		CHECK_EMPTY															\
		if( !IsValidName( mToken ) )										\
		{																	\
			mErr = L"Expected indentifier, but '" + mToken + L"' found!";	\
			mErr += PLINE;													\
			longjmp( mJBuf, 1 );											\
		}																	\
	}

/**********************************************************************************************/
#define CHECK_RESERVED( x )													\
	if( ToToken( x ) != kTokenUnknown )										\
		BREAK( L"Unable to use reserved word as identifier" );

/**********************************************************************************************/
#define CHECK_TYPE															\
	{																		\
		CHECK_EMPTY															\
		if( !IsValidType( mToken ) )										\
		{																	\
			mErr = L"Expected type, but '" + mToken + L" found!";			\
			mErr += PLINE;													\
			longjmp( mJBuf, 1 );											\
		}																	\
	}
	
/**********************************************************************************************/
#define CLEAR( x ) \
	{ size_t c = x.size(); for( size_t i = 0 ; i < c ; ++i ) delete x[ i ]; x.clear(); }

/**********************************************************************************************/
#define FINALIZE_STATEMENT( x ) \
	{ sqlite3_finalize( x ); x = NULL; }

/**********************************************************************************************/
#define FUNCTION( name ) \
	static void sp_ ## name( sqlite3_context* ctx, int c, sqlite3_value** v )

/**********************************************************************************************/
#define FUNCTION_NC( name ) \
	static void sp_ ## name( sqlite3_context* ctx, int, sqlite3_value** v )

/**********************************************************************************************/
#define FROM_UTF8( x, y ) \
	{ if( x ) ConvertFromUTF8( x, strlen( x ), y ); }

/**********************************************************************************************/
#define INVALID_TOKEN \
	{ mErr = mToken + L" - unknown token" + PLINE; longjmp( mJBuf, 1 ); }

/**********************************************************************************************/
#define NPOS \
	wstring::npos

/**********************************************************************************************/
#define REGISTER( name, args ) \
	sqlite3_create_function( db, #name, args, SQLITE_UTF8, NULL, sp_ ## name, NULL, NULL );

/**********************************************************************************************/
#define RETURN_ERROR( x ) \
	{ sqlite3_result_error( ctx, x, -1 ); return; }

/**********************************************************************************************/
#define SP_NAMESPACE_START	namespace SP {
#define SP_NAMESPACE_END	}

/**********************************************************************************************/
#define STRING_FROM_PARAM( s, x ) \
	const char* s = TEXT( x );

/**********************************************************************************************/
#define WTEXT( s, x ) \
	const char* s ## t = TEXT( x ); TO_WTEXT( s, s ## t );

/**********************************************************************************************/
#define RESULT_DOUBLE( x )		sqlite3_result_double( ctx, x )
#define RESULT_INT( x )			sqlite3_result_int( ctx, x )
#define RESULT_NULL				sqlite3_result_null( ctx )
#define RESULT_TEXT( x )		sqlite3_result_text( ctx, x, -1, SQLITE_TRANSIENT )
#define RESULT_VALUE( x )		sqlite3_result_value( ctx, v[ x ] )
#define BLOB( x )				((const uchar*) sqlite3_value_blob( v[ x ] ))
#define BYTES( x )				sqlite3_value_bytes( v[ x ] )
#define DOUBLE( x )				sqlite3_value_double( v[ x ] )
#define INT( x )				sqlite3_value_int( v[ x ] )
#define TEXT( x )				((const char*) sqlite3_value_text( v[ x ] ))
#define TYPE( x )				sqlite3_value_type( v[ x ] )

/**********************************************************************************************/
#define COMMAND( x )			mVM.AddCommand( x, mException )
#define EXPRESSION( x )			new Expression( *this, x )
#define GET_TOKEN				ToToken( mToken )
#define IS_NULL( x )			( TYPE( x ) == SQLITE_NULL )

/**********************************************************************************************/
#define RESULT_WTEXT( x ) \
	{ vector<char> buf; ConvertToUTF8( x, buf ); RESULT_TEXT( &buf[ 0 ] ); }

/**********************************************************************************************/
#define TO_WTEXT( w, s ) \
	wstring w; FROM_UTF8( s, w );
