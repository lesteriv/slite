/**********************************************************************************************/
/* config.cpp																				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// LIBCREST
#include "../third/libcrest/src/cr_utils_private.h"

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static config g_config;


//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static void trim( char*& s )
{
	while( isspace( *s ) ) ++s;
	if( !*s )
		return;
	
	char* e = s + strlen( s ) - 1;
	while( isspace( *e ) )
		*e-- = 0;
}


//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
cr_http_auth config::get_auth_kind( void ) const
{
	return auth_;
}

/**********************************************************************************************/
const char* config::set_auth_kind( const char* value )
{
	if( !strcmp( value, "basic" ) )
		auth_ = CR_AUTH_BASIC;
	else if( !strcmp( value, "digest" ) )
		auth_ = CR_AUTH_DIGEST;
	else if( !strcmp( value, "none" ) )
		auth_ = CR_AUTH_NONE;
	else
		return "expected 'none', 'basic' or 'digest'";
	
	cr_set_auth_kind( auth_ );
	return "";
}

/**********************************************************************************************/
cr_result_format config::get_default_format( void ) const
{
	return default_format_;
}

/**********************************************************************************************/
const char* config::set_default_format( const char* value )
{
	if( !strcmp( value, "xml" ) )
		default_format_ = CR_FORMAT_XML;
	else if( !strcmp( value, "json" ) )
		default_format_ = CR_FORMAT_JSON;
	else
		return "expected 'json' or 'xml'";;
	
	cr_set_default_result_format( default_format_ );
	return "";
}

/**********************************************************************************************/
bool config::get_log_enabled( void ) const
{
	return log_enabled_;
}

/**********************************************************************************************/
const char* config::set_log_enabled( const char* value )
{
	if( *value == '1' )
		log_enabled_ = true;
	else if( *value == '0' )
		log_enabled_ = false;
	else
		return  "expected '0' or '1'";
		
	cr_set_log_enabled( log_enabled_ );
	return "";
}

/**********************************************************************************************/
string config::get_ports( void ) const
{
	mutex_.lock();
	string res = ports_;
	mutex_.unlock();
	
	return res;
}

/**********************************************************************************************/
const char* config::set_ports( const string& ports )
{
	if( parse_ports( ports ).empty() )
		return "expected comma separated list of [ip_address:]port[s] values";
	
	mutex_.lock();
	ports_ = ports;
	mutex_.unlock();
	
	return "";
}

/**********************************************************************************************/
size_t config::get_thread_count( void ) const
{
	return thread_count_;
}

/**********************************************************************************************/
const char* config::set_thread_count( const char* value )
{
	int n = atoi( value );
	if( n > 0 && n <= CR_MAX_THREADS )
	{
		thread_count_ = n;
		return "";
	}
	
	return "expected integer value from 1 to " CR_MAX_THREADS_STRING;
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
config& config::instance( void )
{
	return g_config;
}

/**********************************************************************************************/
void config::set_file( const string& file )
{
	config_file_ = file;

	if( cr_file_exists( file ) )	
		load();
	else
		flush();
}


//////////////////////////////////////////////////////////////////////////
// save/load
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void config::flush( void )
{
	ofstream f( config_file_ );
	
	mutex_.lock(); // -----------------------------
	
	f << "# Kind of authorization for requests (none,basic,digest)\n";
	f << "auth=" << cr_get_auth_name( auth_ ) << "\n\n";
	
	f << "# Default format for responces (json,xml)\n";
	f << "default_format=" << cr_get_format_name( default_format_ ) << "\n\n";
	
	f << "# Enable logs (1,0)\n";
	f << "log=" << ( log_enabled_ ? "1" : "0" ) << "\n\n";

	f << "# Ports listen by server (comma separated list of [ip_address:]port[s] values)\n";
	f << "ports=" << ports_ << "\n\n";

	f << "# Thread count (1.." CR_MAX_THREADS_STRING ")\n";
	f << "threads=" << thread_count_ << "\n\n";

	mutex_.unlock(); // -----------------------------
}

/**********************************************************************************************/
void config::load( void )
{
	ifstream f( config_file_ );
	int line_index = 1;
	
    while( f.good() )
    {
		string line;
		getline( f, line );
		
		if( line.empty() || line[ 0 ] == '#' )
			continue;
		
		size_t sp = line.find( '=' );
		
		// Warn about incorrect line in config
		if( sp == string::npos )
		{
			char* s = &line[ 0 ];
			trim( s );
			
			if( *s )
				printf( "slite.cfg, line %d: '=' not found in \"%s\"\n", line_index, s );
			
			continue;
		}
		
		char* key   = &line[ 0 ];
		char* value = &line[ sp + 1 ];
		line[ sp ] = 0;
		
		trim( key );
		trim( value );
		
		const char* err = 0;
		
		if( !strcmp( key, "auth" ) )
			err = set_auth_kind( value );
		else if( !strcmp( key, "default_format" ) )
			err = set_default_format( value );
		else if( !strcmp( key, "log" ) )
			err = set_log_enabled( value );
		else if( !strcmp( key, "ports" ) )
			err = set_ports( value );
		else if( !strcmp( key, "threads" ) )
			err = set_thread_count( value );
		else
			printf( "slite.cfg, line %d: unknown key \"%s\"\n", line_index, key );
		
		if( err && *err )
			printf( "slite.cfg, line %d: wrong value for \"%s\" - %s, but '%s' found\n", line_index, key, err, value );
		
		++line_index;
	}
}
