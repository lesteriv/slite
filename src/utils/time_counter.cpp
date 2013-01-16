/**********************************************************************************************/
/* time_counter.cpp				                                                   			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SLITE
#include "../slite.h"

// OS SPECIFIC
#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include <sys/timeb.h>
#endif // _WIN32


//////////////////////////////////////////////////////////////////////////
// construction
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
time_counter::time_counter( void )
{
	reset();
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
uint64_t time_counter::milliseconds( void ) const
{
#if _WIN32

	LARGE_INTEGER freq, t;
	QueryPerformanceCounter( &t );
	QueryPerformanceFrequency( &freq );
	return (unit64_t) ( 1000.0 * ( t.QuadPart - mStart ) / freq.QuadPart );

#else // _WIN32

	timeb tp;
	ftime( &tp );
	uint64_t now = 1000L * tp.time + tp.millitm;
	return now - mStart;

#endif // _WIN32
}

/**********************************************************************************************/
void time_counter::reset( void )
{
#ifdef _WIN32

	LARGE_INTEGER t;
	QueryPerformanceCounter( &t );
	mStart = t.QuadPart;

#else // _WIN32

	timeb tp;
	ftime( &tp );
	mStart = 1000L * tp.time + tp.millitm;

#endif // _WIN32
}

/**********************************************************************************************/
string time_counter::value( void ) const
{
	return format_time( milliseconds() );
}
