/**********************************************************************************************/
/* API_logs.cpp																				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// static data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static cr_result_fields g_log_fields =
{
	{ "name", CR_TEXT },
	{ "size", CR_INTEGER }
};

/**********************************************************************************************/
static string g_log_name = "log";


//////////////////////////////////////////////////////////////////////////
// API database
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
DELETE_ADMIN( logs )( cr_connection& conn )
{
	auto files = get_files( g_path_log );
	for( string& file : files )
	{
		if( file != "slite.log" )
		{
			string log_path = g_path_log + file;
			if( remove( log_path.c_str() ) )
			{
				conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to delete file: " + file );
				return;
			}
		}
	}
	
/*	if( gLogFile )
	{
		fclose( gLogFile );
		gLogFile = fopen( gPathLogFile.c_str(), "wt" );
		gLogSize = cr_file_size( gPathLogFile );
	}*/
	
	conn.respond( CR_HTTP_OK, to_string( files.size() ) + " files were deleted" );
}

/**********************************************************************************************/
GET_ADMIN( logs )( cr_connection& conn )
{
	cr_result r( conn );
	r.set_record_fields( g_log_fields );
	r.set_record_name( g_log_name );
	
	auto files = get_files( g_path_log );
	for( auto& file : files )
	{
		size_t len = file.length();
		if( len > 3 && !strcmp( &file[ len - 4 ], ".log" ) )
		{
			int64_t size = cr_file_size( g_path_log + file );
			file.resize( len - 4 );
			
			r.add_text( file );
			r.add_int( size );
		}
	}
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
DELETE_ADMIN( logs/<log> )( cr_connection& conn )
{
	string log_name = conn.path_parameter( 0 );
	if( log_name.empty() || log_name[ 0 ] == '.' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST );
		return;		
	}
	
	string log_path = g_path_log + log_name + ".log";
	if( !cr_file_exists( log_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Log file with name '" + log_path + "' doen't exists" );
		return;			
	}
	
	if( log_name == "slite" )
	{
/*		if( gLogFile )
		{
			fclose( gLogFile );
			gLogFile = fopen( gPathLogFile.c_str(), "wt" );
			gLogSize = cr_file_size( gPathLogFile );
		}*/

		conn.respond( CR_HTTP_OK, "ok" );
	}
	else
	{
		remove( log_path.c_str() ) ?
			conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to delete file: " + log_path + ".db" ) :
			conn.respond( CR_HTTP_OK, "ok" );
	}
}

/**********************************************************************************************/
GET_ADMIN( logs/<log> )( cr_connection& conn )
{
	string log_name = conn.path_parameter( 0 );
	if( log_name.empty() || log_name[ 0 ] == '.' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST );
		return;		
	}
	
	conn.send_file( g_path_log + log_name + ".log" );
}
