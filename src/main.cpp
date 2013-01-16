/**********************************************************************************************/
/* main.cpp					                                                 				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// POSIX
#include <sys/stat.h>

// SLITE
#include "slite.h"

// SQLITE
#include "third/sqlite/sqlite3.h"


//////////////////////////////////////////////////////////////////////////
// default SSL certificate
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static const char* g_pem =
	"-----BEGIN CERTIFICATE-----\n"
	"MIICATCCAWoCCQC332DIUSZ0oTANBgkqhkiG9w0BAQUFADBFMQswCQYDVQQGEwJV\n"
	"QTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"
	"cyBQdHkgTHRkMB4XDTEyMDkyMjE3NDk1N1oXDTEzMDkyMjE3NDk1N1owRTELMAkG\n"
	"A1UEBhMCVUExEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"
	"IFdpZGdpdHMgUHR5IEx0ZDCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA09cJ\n"
	"5C+k5XO9enZInwtq2JdHPMG+8srI4DJyhg5t+XmkTXlKbz0Qqs1nyh1Pvx9bBIM0\n"
	"vN/FIluYQn+r2qOnONbp2+4SBPTbYLFzGSz7LzIuughqwmpGAd+snc0GSUpbwY0X\n"
	"VMebY4uuQndOqZOfmZf1KZomJdHgj4aMhXC9ggMCAwEAATANBgkqhkiG9w0BAQUF\n"
	"AAOBgQA5pHMjIvKjt5+/Msf2MP1ZoqvjZr7CVzNt/thD0sGUQqQZSWpiBKurGvNp\n"
	"baevlpC8Cq8XkY/jIkc6uH1mnabxuOse81jAHpRd/TPQ5NzeR0Kjg1/9y6g6xODD\n"
	"/thuYxqk0b2BVmmws/IagIlOq7iVxjEozmukc6oglc8BMb0fZQ==\n"
	"-----END CERTIFICATE-----\n"
	"-----BEGIN RSA PRIVATE KEY-----\n"
	"MIICXgIBAAKBgQDT1wnkL6Tlc716dkifC2rYl0c8wb7yysjgMnKGDm35eaRNeUpv\n"
	"PRCqzWfKHU+/H1sEgzS838UiW5hCf6vao6c41unb7hIE9NtgsXMZLPsvMi66CGrC\n"
	"akYB36ydzQZJSlvBjRdUx5tji65Cd06pk5+Zl/UpmiYl0eCPhoyFcL2CAwIDAQAB\n"
	"AoGAHg7ODX2hJRmGEiFEbqS9FVxptDo9cM/LLtyZmWHkwtg6su9520zL16BrkKcW\n"
	"EKVPUdWYHxCVNSnP8eeRp+U+rbTes4F4/pfyNgkfR6GtFajx9u8oFz1coLcj+Jns\n"
	"iKGZCSBlCArnPZExLFnfXGKmBqymvBBg36Jb5+paBC/fgSECQQDuENdKLAHS63Zv\n"
	"wINQd/X5yuU/D+PINzjW+inwFBJWnuLjHEtZqm+IORWpIjBPBQ0QqQ0FZUONm/W5\n"
	"cGIGHuixAkEA48xtOKfuPTJ0h6RqEAdbhYn2ngzBr8x9xhVf2g1C3nBYKzqXWPu0\n"
	"UzgEK4kMqZlwWMnEA7NPsP74Yvv3+jlC8wJBAIEQbEdv+EUrFnibfBWCE76quwIn\n"
	"USxYk94ondfkadTAKu1Jh6hC+Fh36kBZoqsUZ/LiU4bbUXZSWTOBtALxeuECQQC9\n"
	"wjAFrxYtUldyGsrjeU2LCyct95Bs4lqvZGTJXpyicVFxCUjELkDDxfjrTyoGKbRF\n"
	"siWfs4Ih/fNpeV3TyllzAkEAt1JirRfSTxcDDqY/67pMrhSYzusv4BMj1YTCKqp/\n"
	"V/9GrSHCC3yWqPS8aRLkYEYBGymvM1uXUXI9rwN5QCGBIw==\n"
	"-----END RSA PRIVATE KEY-----";
	

//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
uint64_t	g_cache_used;
databases_t	g_dbs;
mutex		g_dbs_mutex;
string		g_path_backup;
string		g_path_log;
string		g_path_db;
time_t		g_time_start;


//////////////////////////////////////////////////////////////////////////
// entry point
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
int main( void )
{
	string base = app_path();
	
	sqlite3_initialize();
	g_time_start = time( 0 );
	
	// Creates 'backups' directory
	g_path_backup = base + "backups/";
	mkdir( g_path_backup.c_str(), S_IRWXU | S_IRWXG );

	// Creates 'databases' directory
	g_path_db = base + "databases/";
	mkdir( g_path_db.c_str(), S_IRWXU | S_IRWXG );

	// Creates 'logs' directory
	g_path_log = base + "logs/";
	mkdir( g_path_log.c_str(), S_IRWXU | S_IRWXG );
	
	// Open databases
	auto dbs = get_files( g_path_db );
	for( string& name : dbs )
	{
		// Only with .db extension
		size_t len = name.length();
		if( len > 3 && name.substr( len - 3 ) == ".db" )
		{
			database_ptr db = database::open( g_path_db + name );
			if( db )
			{
				name.resize( len - 3 );
				g_dbs[ name ] = db;
			}
		}
	}

	// Prepare SSL certificate
	string pem_file	= base + "key.pem";
	if( !cr_file_exists( pem_file ) )
		ofstream( pem_file ) << g_pem;
	
	// Init config
	the_config.set_file( base + "slite.cfg" );
	
	// Options for libcrest
	cr_options opts;
	opts.auth_file		= base + "slite.passwd";
	opts.auth_kind		= the_config.get_auth_kind();
	opts.log_enabled	= the_config.get_log_enabled();
	opts.log_file		= g_path_log + "slite.log";
	opts.pem_file		= pem_file;
	opts.ports			= the_config.get_ports();
	opts.result_format	= the_config.get_default_format();
	opts.thread_count	= the_config.get_thread_count();
		
	// Start server
	if( !cr_start( opts ) )
		printf( "%s\n", cr_error_string() );	
}
