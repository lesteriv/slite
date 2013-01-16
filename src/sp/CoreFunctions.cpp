/**********************************************************************************************/
/* CoreFunctions.cpp																		  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SP
#include "CoreFunctions.h"
#include "Unicode.h"
#include "Utils.h"

// STD
#include <cmath>
#include <errno.h>
#include <float.h>
#include <limits>

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// macroses
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
#define RETURN_IF_NULL( x )														\
	if( IS_NULL( x ) ) RESULT_NULL;

/**********************************************************************************************/
#define RETURN_NULL																\
	{ RESULT_NULL; return; }

/**********************************************************************************************/
#define WRAPPER_ARG1( name )													\
FUNCTION_NC( name )																\
{																				\
	RETURN_IF_NULL( 0 );														\
	errno = 0;																	\
	double vl = name( DOUBLE( 0 ) );											\
	if( errno ) sqlite3_result_error( ctx, strerror( errno ), errno );			\
	else RESULT_DOUBLE( vl );													\
}

/**********************************************************************************************/
#define WRAPPER_ARG2( name )													\
FUNCTION_NC( name )																\
{																				\
	RETURN_IF_NULL( 0 );														\
	RETURN_IF_NULL( 1 );														\
	errno = 0;																	\
	double vl = name( DOUBLE( 0 ), DOUBLE( 1 ) );								\
	if( errno ) sqlite3_result_error( ctx, strerror( errno ), errno );			\
	else RESULT_DOUBLE( vl );													\
}


//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static double ln( double x )
{
	return log( x );
}

/**********************************************************************************************/
static double power( double m, double n )
{
	return pow( m, n );
}


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
WRAPPER_ARG1( acos	);
WRAPPER_ARG1( asin	);
WRAPPER_ARG1( atan	);
WRAPPER_ARG2( atan2 );
WRAPPER_ARG1( ceil	);
WRAPPER_ARG1( cos	);
WRAPPER_ARG1( cosh	);
WRAPPER_ARG1( exp	);
WRAPPER_ARG1( floor );
WRAPPER_ARG1( ln	);
WRAPPER_ARG2( power );
WRAPPER_ARG1( sin	);
WRAPPER_ARG1( sinh	);
WRAPPER_ARG1( sqrt	);
WRAPPER_ARG1( tan	);
WRAPPER_ARG1( tanh	);

/**********************************************************************************************/
FUNCTION_NC( ascii )
{
	STRING_FROM_PARAM( s, 0 );
	RESULT_INT( s ? (uchar) *s : 0 );
}

/**********************************************************************************************/
FUNCTION_NC( asciistr )
{
	RETURN_IF_NULL( 0 );
	WTEXT( w, 0 );
	
	string cstr;
	size_t len = w.length();
	for( size_t i = 0 ; i < len ; ++i )
	{
		wchar_t ch = w[ i ];
		
		if( ch < 128 )
		{
			cstr += (char) ch;
		}
		else
		{
			char buf[] = "0000\0";
			cstr += '\\';
			
			for( size_t j = 0 ; j < 4 ; ++j )
			{
				buf[ 3 - j ] = "0123456789ABCDEF"[ ch % 16 ];
				ch /= 16;
			}
			
			cstr += buf;
		}
	}
	
	RESULT_TEXT( &cstr[ 0 ] );
}

/**********************************************************************************************/
FUNCTION( bin_to_num )
{
	int res = 0;

	for( int i = 0 ; i < c ; ++i )
	{
		res *= 2;
		if( INT( i ) )
			++res;
	}

	RESULT_INT( res );
}

/**********************************************************************************************/
FUNCTION_NC( bitand )
{
	int m = INT( 0 );
	int n = INT( 1 );

	RESULT_INT( m & n );
}

/**********************************************************************************************/
FUNCTION_NC( chr )
{
	char res[] = "0";
	res[ 0 ] = (char) INT( 0 );
	RESULT_TEXT( res );
}

/**********************************************************************************************/
FUNCTION_NC( compose )
{
	RETURN_IF_NULL( 0 );
	WTEXT( w, 0 );

	size_t count = w.size();
	if( !count )
	{
		RESULT_TEXT( "" );
		return;
	}

	bool upper = true;
	for( size_t i = 0 ; i < count ; ++i )
	{
		wchar_t ch = w[ i ];

		if( upper )
		{
			w[ i ] = towupper( ch );
			upper = false;
		}
		else
		{
			if( strchr( " ,.;\t\n[]()*?!-+={}'\"", (char) ch ) )
				upper = true;
			else
				w[ i ] = towlower( ch );
		}
	}

	RESULT_WTEXT( w );
}

/**********************************************************************************************/
FUNCTION_NC( concat )
{
	STRING_FROM_PARAM( s1, 0 );
	STRING_FROM_PARAM( s2, 1 );

	if( !s1 && !s2 )
	{
		RESULT_NULL;
	}
	else if( !s1 && s2 )
	{
		RESULT_TEXT( s2 );
	}
	else if( s1 && !s2 )
	{
		RESULT_TEXT( s1 );
	}
	else
	{
		string str = s1;
		str += s2;
		
		RESULT_TEXT( str.length() ? &str[ 0 ] : s1 );
	}
}

/**********************************************************************************************/
FUNCTION( decode )
{
	if( c < 3 )
		RETURN_NULL
	
	string str = TEXT( 0 );
	
	int i = 1;
	while( i < c - 1 )
	{
		if( str == TEXT( i ) )
		{
			RESULT_VALUE( i + 1 );
			return;
		}
		
		i += 2;
	}
	
	i < c ? RESULT_VALUE( i ) : RESULT_NULL;
}

/**********************************************************************************************/
FUNCTION( greatest )
{
	if( !c )
		RETURN_NULL;
	
	RETURN_IF_NULL( 0 );
	
	switch( TYPE( 0 ) )
	{
		case kDouble	:
		{
			double vl = DOUBLE( 0 );
			for( int i = 1 ; i < c ; ++i )
				vl = max( vl, DOUBLE( i ) );
			
			RESULT_DOUBLE( vl );
		}
		break;
			
		case kInt	:
		{
			int vl = INT( 0 );
			for( int i = 1 ; i < c ; ++i )
				vl = max( vl, INT( i ) );
			
			RESULT_INT( vl );
		}
		break;
		
		default:
		{
			WTEXT( vl, 0 );

			for( int i = 1 ; i < c ; ++i )
			{
				WTEXT( s, i );
				vl = max( vl, s );
			}

			RESULT_WTEXT( vl );
		}
		break;
	}
}

/**********************************************************************************************/
#define XTOI( x ) ( ( x>='0' && x<='9' ) ? x-'0' : ( x>='a' && x<='f' ) ? x-'a'+10 : x-'A'+10 )

/**********************************************************************************************/
FUNCTION_NC( hextoraw )
{
	RETURN_IF_NULL( 0 );
	STRING_FROM_PARAM( s, 0 )
	
	size_t slen = strlen( s );
	size_t len = ( slen + 1 ) / 2;
	if( !len )
		RETURN_NULL

	char* data = (char*) malloc( len );
	
	if( slen % 2 )
	{
		*data++ = XTOI( s[ 0 ] );
		++s; --len;
	}
	
	for( size_t i = 0 ; i < len ; ++i )
		data[ i ] = XTOI( s[ i * 2 ] ) * 16 + XTOI( s[ i * 2 + 1 ] );
	
	if( slen % 2 )
	{
		data--; len++;
	}
	
	sqlite3_result_blob( ctx, data, len, SQLITE_TRANSIENT );
}

/**********************************************************************************************/
FUNCTION_NC( initcap )
{
	RETURN_IF_NULL( 0 );
	WTEXT( s, 0 );

	size_t len = s.size();
	if( !len )
		RESULT_TEXT( "" );

	bool upper = true;
	for( size_t i = 0 ; i < len ; ++i )
	{
		wchar_t ch = s[ i ];

		if( upper )
		{
			s[ i ] = towupper( ch );
			upper = false;
		}
		else
		{
			if( strchr( " ,.;\t\n[]()*?!-+={}'\"", (char) ch ) )
				upper = true;
			else
				s[ i ] = towlower( ch );
		}
	}

	RESULT_WTEXT( s );
}

/**********************************************************************************************/
FUNCTION( least )
{
	if( !c || IS_NULL( 0 ) )
		RETURN_NULL;
	
	switch( TYPE( 0 ) )
	{
		case kDouble:
		{
			double vl = DOUBLE( 0 );
			for( int i = 1 ; i < c ; ++i )
			{
				RETURN_IF_NULL( i );
				vl = min( vl, DOUBLE( i ) );
			}
			
			RESULT_DOUBLE( vl );
		}
		break;
			
		case kInt	:
		{
			int vl = INT( 0 );
			for( int i = 1 ; i < c ; ++i )
			{
				RETURN_IF_NULL( i );
				vl = min( vl, INT( i ) );
			}
			
			RESULT_INT( vl );
		}
		break;
		
		default:
		{
			WTEXT( vl, 0 );
			
			for( int i = 1 ; i < c ; ++i )
			{
				WTEXT( s, i );
				if( !st )
					RETURN_NULL;
				
				vl = min( vl, s );
			}
			
			RESULT_WTEXT( vl );
		}
		break;
	}
}

/**********************************************************************************************/
FUNCTION_NC( lnnvl )
{
	RESULT_INT( IS_NULL( 0 ) ? 1 : INT( 0 ) ? 0 : 1 );
}

/**********************************************************************************************/
FUNCTION( lpad )
{
	RETURN_IF_NULL( 0 );
	WTEXT( s, 0 );

	int plen = max( 0, INT( 1 ) );

	wstring ps = L" ";
	if( c > 2 )
	{
		STRING_FROM_PARAM( sp, 2 );
		if( sp )
		{
			TO_WTEXT( wp, sp );
			if( wp.size() )
				ps.swap( wp );
		}
	}

	// TODO
	while( s.size() < size_t( plen ) )
		s.insert( 0, ps );

	s.resize( plen );
	RESULT_WTEXT( s );
}

/**********************************************************************************************/
FUNCTION_NC( rawtohex )
{
	size_t c = BYTES( 0 );
	const uchar* data = BLOB( 0 );

	vector<char> out( c * 2 + 1 );
	out[ c * 2 ] = 0;
	
	for( size_t i = 0 ; i < c ; ++i )
	{
		out[ i * 2	   ] = "01234567890ABCDEF"[ ( data[ i ] >> 4 ) & 0xF ];
		out[ i * 2 + 1 ] = "01234567890ABCDEF"[ data[ i ] & 0xF ];
	}
	
	sqlite3_result_text( ctx, &out[ 0 ], c * 2, SQLITE_TRANSIENT );
}

/**********************************************************************************************/
FUNCTION_NC( remainder )
{
	double m = DOUBLE( 0 );
	double n = DOUBLE( 1 );
	
	 m ? RESULT_DOUBLE( m - ( n * floor( m / n + .5 ) ) ) : RESULT_NULL;
}

/**********************************************************************************************/
FUNCTION( rpad )
{
	RETURN_IF_NULL( 0 );
	WTEXT( s, 0 );

	int plen = max( 0, INT( 1 ) );

	wstring ps = L" ";
	if( c > 2 )
	{
		STRING_FROM_PARAM( sp, 2 );
		if( sp )
		{
			TO_WTEXT( wp, sp );
			if( wp.size() )
				ps.swap( wp );
		}
	}

	while( s.size() < size_t( plen ) )
		s += ps;

	s.resize( plen );
	RESULT_WTEXT( s );
}

/**********************************************************************************************/
FUNCTION_NC( mod )
{
	if( TYPE( 0 ) == kInt && TYPE( 1 ) == kInt )
	{
		int m = INT( 0 );
		int n = INT( 1 );

		RESULT_INT( n ? m % n : m );
	}
	else
	{
		double m = DOUBLE( 0 );
		double n = DOUBLE( 1 );

		RESULT_DOUBLE( n ? m - n * floor( m / n ) : m );
	}
}

/**********************************************************************************************/
FUNCTION_NC( nanvl )
{
	double d = DOUBLE( 0 );
	RESULT_VALUE( ( IS_NULL( 0 ) || d != d ) ? 1 : 0 );
}

/**********************************************************************************************/
FUNCTION_NC( numtodsinterval )
{
	RETURN_IF_NULL( 1 );
	WTEXT( s, 1 );
	ToLower( s );
	
	double d = DOUBLE( 0 );
	if( s == L"day" )
		d *= 60 * 60 * 24;
	else if( s == L"hour" )
		d *= 60 * 60;
	else if( s == L"minute" )
		d *= 60;
	else if( s != L"second")
		RETURN_NULL;

	int days = int( d / 60 / 60 / 24 );
	s = IntToStr( days );
	while( s.length() < 9 ) s.insert( 0, L"0" );
	s.insert( 0, d > 0 ? L"+" : L"-" );
	d -= days * 60 * 60 * 24;
	
	s += ' ';
	
	int h = int( d / 60 / 60 );
	if( h < 10 ) s += '0';
	s += IntToStr( h );
	d -= h * 60 * 60;
	
	s += ':';
	
	int m = int( d / 60 );
	if( m < 10 ) s += '0';
	s += IntToStr( m );
	d -= m * 60;
	
	s += ':';
	if( d < 10 ) s += '0';
	
	wchar_t buf[ 32 ];
	swprintf( buf, 32, L"%.9f", d );
	s += buf;
	
	RESULT_WTEXT( s );
}

/**********************************************************************************************/
FUNCTION_NC( numtoyminterval )
{
	RETURN_IF_NULL( 1 );
	WTEXT( s, 1 );
	ToLower( s );
	
	double d = DOUBLE( 0 );
	if( s == L"year" )
		d *= 12;
	else if( s != L"month")
		RETURN_NULL;

	int y = int( d / 12 );
	s = IntToStr( y );
	while( s.length() < 9 ) s.insert( 0, L"0" );
	s.insert( 0, d > 0 ? L"+" : L"-" );
	d -= y * 12;
	
	s += '-';
	
	int m = (int) d;
	if( m < 10 ) s += '0';
	s += IntToStr( m );

	RESULT_WTEXT( s );
}

/**********************************************************************************************/
FUNCTION_NC( nvl )
{
	RESULT_VALUE( IS_NULL( 0 ) ? 1 : 0 );
}

/**********************************************************************************************/
FUNCTION_NC( nvl2 )
{
	RESULT_VALUE( IS_NULL( 0 ) ? 2 : 1 );
}

/**********************************************************************************************/
FUNCTION_NC( sign )
{
	RETURN_IF_NULL( 0 );
	double d = DOUBLE( 0 );
	RESULT_INT( d ? d < 0 ? -1 : 1 : 0 );
}

/**********************************************************************************************/
FUNCTION_NC( translate )
{
	RETURN_IF_NULL( 0 )
	RETURN_IF_NULL( 1 )
	RETURN_IF_NULL( 2 )
	
	WTEXT( w1, 0 );
	WTEXT( w2, 1 );
	WTEXT( w3, 2 );

	size_t count = min( w2.size(), w3.size() );
	size_t len = w1.size();

	for( size_t i = 0 ; i < count ; ++i )
	{
		wchar_t from = w2[ i ];
		wchar_t to   = w3[ i ];

		for( size_t j = 0 ; j < len ; ++j )
		{
			if( w1[ j ] == from )
				w1[ j ] = to;
		}
	}

	RESULT_WTEXT( w1 );
}

/**********************************************************************************************/
FUNCTION_NC( to_char )
{
	STRING_FROM_PARAM( s, 0 );
	if( !s )
		RETURN_NULL;

	// TODO
	RESULT_TEXT( s );
}

/**********************************************************************************************/
FUNCTION_NC( to_number )
{
	// TODO
	RESULT_INT( INT( 0 ) );
}

/**********************************************************************************************/
FUNCTION( trunc )
{
	RETURN_IF_NULL( 0 );
	
	int count = ( c == 2 ) ? INT( 1 ) : 0;
	double vl = DOUBLE( 0 );
	
	double dvd = 1;
	if( count > 0 && count < 20 )
	{
		for( int i = 0 ; i < count ; ++i )
			dvd *= 10;
	}
	
	vl *= dvd;
	vl = int( vl );
	vl /= dvd;
	
	RESULT_DOUBLE( vl );
}

/**********************************************************************************************/
FUNCTION_NC( vsize )
{
	int type = TYPE( 0 );
	if( type == kInt || type == kDouble )
	{
		RESULT_INT( 8 );
	}
	else
	{
		int size = BYTES( 0 );
		size ? RESULT_INT( size ) : RESULT_NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void RegisterFunctions( sqlite3* db )
{
	REGISTER( acos				, 1 );
	REGISTER( ascii				, 1 );
	REGISTER( asciistr			, 1 );
	REGISTER( asin				, 1 );
	REGISTER( atan				, 1 );
	REGISTER( atan2				, 2 );
	REGISTER( bin_to_num		,-1 );
	REGISTER( bitand			, 2 );
	REGISTER( ceil				, 1 );
	REGISTER( chr				, 1 );
	REGISTER( compose			, 1 );
	REGISTER( concat			, 2 );
	REGISTER( cos				, 1 );
	REGISTER( cosh				, 1 );
	REGISTER( decode			,-1 );
	REGISTER( exp				, 1 );
	REGISTER( floor				, 1 );
	REGISTER( greatest			,-1 );
	REGISTER( hextoraw			, 1 );
	REGISTER( initcap			, 1 );
	REGISTER( least				,-1 );
	REGISTER( ln				, 1 );
	REGISTER( lnnvl				, 1 );
	REGISTER( lpad				, 2 );
	REGISTER( lpad				, 3 );
	REGISTER( mod				, 2 );
	REGISTER( nanvl				, 2 );
	REGISTER( numtodsinterval	, 2 );
	REGISTER( numtoyminterval	, 2 );
	REGISTER( power				, 2 );
	REGISTER( rawtohex			, 1 );
	REGISTER( remainder			, 2 );
	REGISTER( rpad				, 2 );
	REGISTER( rpad				, 3 );
	REGISTER( nvl				, 2 );
	REGISTER( nvl2				, 3 );
	REGISTER( sign				, 1 );
	REGISTER( sin				, 1 );
	REGISTER( sinh				, 1 );
	REGISTER( sqrt				, 1 );
	REGISTER( tan				, 1 );
	REGISTER( tanh				, 1 );
	REGISTER( to_char			,-1 );
	REGISTER( to_number			,-1 );
	REGISTER( translate			, 3 );
	REGISTER( trunc				, 1 );
	REGISTER( trunc				, 2 );
	REGISTER( vsize				, 1 );
}


/**********************************************************************************************/
SP_NAMESPACE_END
