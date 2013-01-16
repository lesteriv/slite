/**********************************************************************************************/
/* Server_API_Misc.cpp		  	                                               				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// STD
#include <sys/stat.h>

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// static data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static cr_result_fields g_db_fields =
{
	{ "name", CR_TEXT },
	{ "state", CR_TEXT }
};

/**********************************************************************************************/
static string g_db_name = "database";


//////////////////////////////////////////////////////////////////////////
// API databases
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
GET( databases )( cr_connection& conn )
{
	cr_result r( conn );
	r.set_record_fields( g_db_fields );
	r.set_record_name( g_db_name );
	
	auto files = get_files( g_path_db );

	g_dbs_mutex.lock(); // ----------------------
	
	for( string& file : files )
	{
		if( file.length() < 4 || file.substr( file.length() - 3, 3 ) != ".db" )
			continue;
		
		file.erase( file.length() - 3 );
		r.add_text( file );
		r.add_text( ( g_dbs.find( file ) != g_dbs.end() ) ? "opened" : "closed" );
	}
	
	g_dbs_mutex.unlock(); // ----------------------
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
POST_ADMIN( databases )( cr_connection& conn )
{
	string db_name = conn.query_parameter( "name" );
	if( db_name.empty() || db_name[ 0 ] == '.' || db_name[ 0 ] == '_' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Invalid database name" );
		return;
	}
	
	string db_path = g_path_db + db_name + ".db";
	if( cr_file_exists( db_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Database with such name already exists: " + db_name );
		return;
	}
	
	database_ptr db = database::create( db_path );
	if( db )
	{
		g_dbs_mutex.lock();
		g_dbs[ db_name ] = db;
		g_dbs_mutex.unlock();
		
		conn.respond( CR_HTTP_CREATED, "ok" );
	}
	else
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to create database" );
	}
}

/**********************************************************************************************/
DELETE( databases/<db> )( cr_connection& conn )
{
	string db_name = conn.path_parameter( 0 );
	if( db_name.empty() || db_name[ 0 ] == '.' || db_name[ 0 ] == '_' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Invalid database name" );
		return;
	}	
	
	// Check confirmation
	const char* confirmation = conn.query_parameter( "confirmation" );
	if( !confirmation || strcmp( confirmation, "yes" ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "This action must be confirmed" );
		return;
	}

	// Check if file exists
	string db_path = g_path_db + db_name + ".db";
	if( !cr_file_exists( db_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Database not exists" );
		return;
	}
	
	g_dbs_mutex.lock(); // ----------------------------

	// Close database if need
	auto it = g_dbs.find( db_name );
	if( it != g_dbs.end() )
	{
		auto db = it->second;
		g_dbs.erase( it );
		
		g_dbs_mutex.unlock(); // ----------------------------
		
		// Wait for other threads
		size_t count = 0;
		while( !db.unique() && count < 100 )
			cr_sleep( 100 );
		
		if( !db.unique() )
		{
			conn.respond( CR_HTTP_INTERNAL_ERROR, "Database is in use" );
			
			g_dbs_mutex.lock();
			g_dbs[ db_name ] = db;
			g_dbs_mutex.unlock();
			
			return;
		}
	}
	else
	{
		g_dbs_mutex.unlock(); // ----------------------------
	}

	// Remove from disk
	remove( db_path.c_str() ) ?
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to remove file from disk" ) :
		conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
POST( databases/<db>/check )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( db )
		db->check( conn );
}

/**********************************************************************************************/
POST( databases/<db>/close )( cr_connection& conn )
{
	string dbname = conn.path_parameter( 0 );
	g_dbs_mutex.lock();
	
	auto it = g_dbs.find( dbname );
	if( it == g_dbs.end() )
	{
		g_dbs_mutex.unlock();
		
		cr_file_exists( g_path_db + dbname + ".db" ) ?
			conn.respond( CR_HTTP_BAD_REQUEST, "Database not opened" + dbname ) :
			conn.respond( CR_HTTP_BAD_REQUEST, "Database not found: " + dbname );
		
		return;
	}

	g_dbs.erase( it );
	g_dbs_mutex.unlock();
	
	conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
POST( databases/<db>/flush )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( db )
		db->flush( conn );
}

/**********************************************************************************************/
POST( databases/<db>/interrupt )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( db )
		db->interrupt( conn );
}

/**********************************************************************************************/
POST( databases/<db>/open )( cr_connection& conn )
{
	string db_name = conn.path_parameter( 0 );
		
	// Check for valid name
	if( db_name.empty() || db_name[ 0 ] == '.' || db_name[ 0 ] == '_' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Invalid database name: " + db_name );
		return;
	}
	
	g_dbs_mutex.lock();
	bool opened = g_dbs.find( db_name ) != g_dbs.end();
	g_dbs_mutex.unlock();	
	
	// Check is already opened
	if( opened )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Database already opened" );
		return;
	}
	
	// Check is file exists
	string db_path = g_path_db + db_name + ".db";
	if( !cr_file_exists( db_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Database not exists" );
		return;
	}
	
	// Open database
	database_ptr db = database::open( db_path );
	if( db )
	{
		g_dbs_mutex.lock();
		g_dbs[ db_name ] = db;
		g_dbs_mutex.unlock();
		
		conn.respond( CR_HTTP_OK, "ok" );
	}
	else
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to open database" );
	}
}

/**********************************************************************************************/
POST( databases/<db>/query )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;
		
	const char* sql = conn.query_parameter( "sql", 3 );
	sql ?
		db->query( conn, sql ) :
		conn.respond( CR_HTTP_BAD_REQUEST, "None query specified" );
}

/**********************************************************************************************/
GET( databases/<db>/stats )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( db )
		db->stats( conn );
}
