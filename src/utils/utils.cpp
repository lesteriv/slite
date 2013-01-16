/**********************************************************************************************/
/* utils.cpp				                                                   				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// POSIX
#if defined(__linux__) || defined(__MACH__)
#include <dirent.h>
#include <unistd.h>
#endif // __linux__ || _MACH__

// OS-SPECIFIC
#if defined(_WIN32)
#include <windows.h>
#elif defined(__MACH__)
#include <mach-o/dyld.h>
#endif // __MACH__

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
string app_path( void )
{
#ifdef _WIN32
	
	// TODO: not tested

	char buf[ 4096 ];
	string res;
	
	if( GetModuleFileNameA( NULL, buf, 1023 ) )
	{
		res = buf;

		size_t p = res.rfind( '\\' );
		if( p != string::npos )
			res.resize( p + 1 );
	}
	
	return res;

#elif defined(LINUX) || defined(__linux__)

	char buf[ 4096 ];
	string res;

	int len = readlink( "/proc/self/exe", buf, 4095 );
	if( len > 0 )
	{
		buf[ len ] = 0;
		res = buf;

		size_t p = res.rfind( '/' );
		if( p != string::npos )
			res.resize( p + 1 );
	}
	
	return res;
		
#elif defined(__MACH__)

	// TODO: not tested
	
	char path[ 4096 ];
	string res;
	
	uint32_t size = sizeof( path );
	if( _NSGetExecutablePath( path, &size ) == 0 )
	{
		res = buf;

		size_t p = res.rfind( '/' );
		if( p != string::npos )
			res.resize( p + 1 );
	}
	
#else // __MACH__
	
	// TODO: not tested
	
	char buf[ 4096 ];
	getcwd( buf, 4095 );
	
	size_t len = strlen( buf );
	buf[ len ] = '/';
	buf[ len + 1 ] = 0;

	return buf;
	
#endif // __MACH__		
}

/**********************************************************************************************/
void escape_sql( string& str )
{
	size_t len = str.length();
	for( size_t i = 0 ; i < len ; ++i )
	{
		if( str[ i ] == '\'' )
		{
			str.insert( i, 1, '\'' );
			++i; ++len;
		}
	}
}

/**********************************************************************************************/
string format_time( int ms )
{
	char buf[ 64 ];

	ms < 1000 ?
		snprintf( buf, 64, "%d ms", ms ) :
		ms < 60000 ?
			snprintf( buf, 64, "%.1f second(s)", ms / 1000. ) :
			ms < 3600000 ?
				snprintf( buf, 64, "%.1f minute(s)", ms / 60000. ) :
				ms < 86400000 ?
					snprintf( buf, 64, "%.1f hour(s)", ms / 3600000. ) :
					snprintf( buf, 64, "%.1f day(s)", ms / 86400000. );
				
	buf[ 63 ] = 0;
	return buf;
}

/**********************************************************************************************/
database_ptr get_database( cr_connection& conn )
{
	string db_name = conn.path_parameter( 0 );
	
	g_dbs_mutex.lock(); // -----------------------

	// Try to find database by name
	auto it = g_dbs.find( db_name );
	if( it == g_dbs.end() )
	{
		g_dbs_mutex.unlock(); // -----------------------

		conn.respond( CR_HTTP_BAD_REQUEST, "Unable to find database: " + db_name );
		return NULL;
	}

	// Found - returns it
	auto db = it->second;

	g_dbs_mutex.unlock(); // -----------------------
	
	return db;	
}

/**********************************************************************************************/
vector<string> get_files( const string& path )
{
	vector<string> res;

#ifdef _WIN32
	
	_finddatai64_t data;
	
	long handler = _findfirsti64( path.c_str(), &data );
	if( handler >= 0 )
	{
		do
		{
			res.push_back( data.name );
		}
		while( _findnexti64( handler, &data ) == 0 );

		_findclose( handler );
	}
	
#else // _WIN32

    DIR* pdir = opendir( path.c_str() );
    if( pdir )
	{
		dirent* pent;
		while( ( pent = readdir( pdir ) ) )
		{
			if( *pent->d_name != '.' )
				res.push_back( pent->d_name );
		}

		closedir( pdir );
	}
	
#endif // _WIN32
	
	return res;
}
