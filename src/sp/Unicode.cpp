/**********************************************************************************************/		
/* Unicode.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Unicode.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
#define u32 long


//////////////////////////////////////////////////////////////////////////
// static data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static const uchar first[] =
	{ 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/**********************************************************************************************/
static const u32 offs[] =
	{ 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/**********************************************************************************************/
static const uchar tr[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void ConvertUTF8ToUTF16(
	const char*	rs,
	size_t		count,
	wstring&	buf )
{
	buf.clear();
	
	const uchar* s = (const uchar*) rs;
	if( !s )
		return;

	buf.reserve( count + 1 );

    uchar cl;
    u32	c;

	const uchar* end = s + count;
	while( s < end )
	{
		cl = tr[ *s ];
		if( s + cl >= end )
			break;

		c = 0;
		switch( cl )
		{
			case 5:	c += *s++; c <<= 6;
			case 4:	c += *s++; c <<= 6;
			case 3:	c += *s++; c <<= 6;
			case 2:	c += *s++; c <<= 6;
			case 1:	c += *s++; c <<= 6;
			case 0:	c += *s++;

            default:;
		}

		c -= offs[ cl ];

		if( c <= u32( 0x0000FFFF ) )
		{
			buf.push_back( wchar_t( c ) );
		}
		else if( c > u32( 0x0010FFFF ) )
		{
			buf.push_back( u32( 0x0000FFFD ) );
		}
		else
		{
			c -= u32( 0x0010000UL );
			buf.push_back( wchar_t( c >> 10 ) + u32( 0xD800 ) );
			buf.push_back( wchar_t( c & u32( 0x3FFUL ) ) + u32( 0xDC00 ) );
		}
	}
}

/**********************************************************************************************/
void ConvertUTF8ToUTF32(
	const char*	rs,
	size_t		count,
	wstring&	buf )
{
	buf.clear();
	
	const uchar* s = (const uchar*) rs;
	if( !s )
		return;

	buf.reserve( count + 1 );

    uchar cl;
    u32	c;

	const uchar* end = s + count;
	while( s < end )
	{
		cl = tr[ *s ];
		if( s + cl >= end )
			break;

		c = 0;
		switch( cl )
		{
			case 5:	c += *s++; c <<= 6;
			case 4:	c += *s++; c <<= 6;
			case 3:	c += *s++; c <<= 6;
			case 2:	c += *s++; c <<= 6;
			case 1:	c += *s++; c <<= 6;
			case 0:	c += *s++;

            default:;
		}

		c -= offs[ cl ];

		if( c <= u32( 0x7FFFFFFF ) )
			buf.push_back( ( wchar_t )c );
		else
			buf.push_back( u32( 0x0000FFFD ) );
	}
}

#ifdef _WIN32
/**********************************************************************************************/
static void ConvertUTF16TtoUTF8(
	const wchar_t*	s,
	size_t			count,
	vector<char>&	buf )
{
	buf.clear();
	if( !s )
		return;

	buf.reserve( count + 1 );

    u32			c;
    uchar		cl;
    const u32	bmask = 0xBF;
    const u32	bmark = 0x80;

	const wchar_t* end = s + count;
	while( s < end )
	{
		c  = *s++;
		cl = 0;

		if( c >= u32( 0xD800 ) && c <= u32( 0xDBFF ) && s < end - 1 )
		{
			u32 ch2 = *s;
			if( ch2 >= u32( 0xDC00 ) && ch2 <= u32( 0xDFFF ) )
			{
				c = ( ( c - ( u32( 0xD800 ) << 10 ) ) + ( ch2 - u32( 0xDC00 ) ) + u32( 0x0010000UL ) );
				++s;
			}
		}

		if( c < u32( 0x80 ) )
			cl = 1;
		else if( c < u32( 0x800 ) )
			cl = 2;
		else if( c < u32( 0x10000 ) )
			cl = 3;
		else if( c < u32( 0x200000 ) )
			cl = 4;
		else
		{
			cl = 2;
			c = u32( 0x0000FFFD );
		}

		buf.resize( buf.size() + cl );
		vector<char>::iterator it = buf.end();

		switch( cl )
		{
			case 4:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 3:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 2:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 1:	*--it = char( c | first[ cl ] );

            default:;
		}
	}
}
#elif 1 // _WIN32
/**********************************************************************************************/
static void ConvertUTF32ToUTF8(
	const wchar_t*	s,
	size_t			count,
	vector<char>&	buf )
{
    buf.clear();
	buf.reserve( count + 1 );

    u32		c;
    unsigned short	cl;
    const u32	bmask = 0xBF;
    const u32	bmark = 0x80;

	for( size_t i = 0 ; i < count ; ++i )
	{
		c = *s++;

		if( c < u32( 0x80 ) )
			cl = 1;
		else if( c < u32( 0x800 ) )
			cl = 2;
		else if( c < u32( 0x10000 ) )
			cl = 3;
		else if( c < u32( 0x200000 ) )
			cl = 4;
		else
		{
			cl = 2;
			c = u32( 0x0000FFFD );
		}

		buf.resize( buf.size() + cl );
		vector<char>::iterator it = buf.end();

		switch( cl )
		{
			case 4:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 3:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 2:	*--it = char( ( c | bmark ) & bmask ); c >>= 6;
			case 1:	*--it = char( c | first[ cl ] );

            default:;
		}
	}
}
#endif // _WIN32

/**********************************************************************************************/
void ConvertToUTF8(
	const wstring&	s,
	vector<char>&	out )
{
#ifdef _WIN32
	ConvertUTF16TtoUTF8( &s[ 0 ], s.size(), out );
#elif 1 // _WIN32
	ConvertUTF32ToUTF8( &s[ 0 ], s.size(), out );
#endif // _WIN32
	
	out.push_back( 0 );
}

/**********************************************************************************************/
void ConvertFromUTF8(
	const char*	s,
	size_t		count,
	wstring&	buf )
{
#ifdef _WIN32
	ConvertUTF8ToUTF16( s, count, buf );
#elif 1 // _WIN32
	ConvertUTF8ToUTF32( s, count, buf );
#endif // _WIN32
}


/**********************************************************************************************/
SP_NAMESPACE_END
