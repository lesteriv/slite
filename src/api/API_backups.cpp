/**********************************************************************************************/
/* API_backups.cpp																			  */
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
static cr_result_fields g_backup_fields =
{
	{ "name", CR_TEXT },
	{ "size", CR_INTEGER }
};

/**********************************************************************************************/
static string g_backup_name = "backup";


//////////////////////////////////////////////////////////////////////////
// API backups
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
DELETE_ADMIN( backups )( cr_connection& conn )
{
	auto files = get_files( g_path_backup );
	for( string& file : files )
	{
		string path = g_path_backup + file;
		if( remove( path.c_str() ) )
		{
			conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to delete file: " + file );
			return;
		}
	}
	
	conn.respond( CR_HTTP_OK, to_string( files.size() ) + " files were deleted" );
}

/**********************************************************************************************/
GET_ADMIN( backups )( cr_connection& conn )
{
	cr_result r( conn );
	r.set_record_fields	( g_backup_fields );
	r.set_record_name	( g_backup_name );
	
	auto files = get_files( g_path_backup );
	for( auto& file : files )
	{
		size_t len = file.length();
		if( len > 3 && !strcmp( &file[ len - 3 ], ".db" ) )
		{
			int64_t size = cr_file_size( g_path_backup + file );
			file.resize( len - 3 );
			
			r.add_text( file );
			r.add_int( size );
		}
	}
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
POST_ADMIN( backups )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;
	
	// Name for backup file
	string backup_name = conn.query_parameter( "name" );
	if( backup_name.empty() )
	{
		// Generate suffix for backup file
		time_t timer = time( NULL );
		tm* t = localtime( &timer );
		char buf[ 32 ];
		strftime( buf, 32, " %y-%m-%d %H:%M:%S.db", t );

		backup_name = string( conn.path_parameter( 0 ) ) + buf;
	}
	else
	{
		if( backup_name[ 0 ] == '.' || backup_name[ 0 ] == '_' )
		{
			conn.respond( CR_HTTP_BAD_REQUEST, "Invalid name: " + backup_name );
			return;
		}
	
		backup_name += ".db";
	}
	
	// Check for existing file
	string db_path = g_path_backup + backup_name;
	if( cr_file_exists( db_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "File already exists: " + backup_name );
		return;
	}
	
	// Do backup
	db->backup( conn, db_path, backup_name );	
}

/**********************************************************************************************/
DELETE_ADMIN( backups/<backup> )( cr_connection& conn )
{
	string backup_name = conn.path_parameter( 0 );
	if( backup_name.empty() || backup_name[ 0 ] == '.' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Expected valid name of backup file" );
		return;		
	}
	
	string backup_path = g_path_backup + backup_name + ".db";
	if( !cr_file_exists( backup_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Backup file with name '" + backup_name + "' doesn't exists" );
		return;			
	}
	
	remove( backup_path.c_str() ) ?
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to delete file: " + backup_name + ".db" ) :
		conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
POST_ADMIN( backups/<backup>/restore )( cr_connection& conn )
{
	database_ptr db;
	string old_path;
	
	// Collect and check parameters
	
	string backup_name = conn.path_parameter( 0 );
	if( backup_name.empty() || backup_name[ 0 ] == '.' )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Expected valid name of backup file" );
		return;		
	}
	
	string backup_path = g_path_backup + backup_name + ".db";
	if( !cr_file_exists( backup_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Backup file with name '" + backup_name + "' doesn't exists" );
		return;			
	}

	string db_name = conn.query_parameter( "db" );
	if( db_name.empty() )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "None database specified" );
		return;
	}
	
	string db_path = g_path_db + db_name + ".db";
	if( !cr_file_exists( db_path ) )
	{
		conn.respond( CR_HTTP_BAD_REQUEST, "Database file with name '" + backup_name + "' doesn't exists" );
		return;		
	}
	
	// Make copy of file
	
	time_counter tc;
	string restore_path = db_path + ".restore";
	
    ifstream ifs( backup_path, std::ios::binary );
    ofstream ofs( restore_path, std::ios::binary );

	bool copied = ifs && ofs && ( ofs << ifs.rdbuf() );
	if( !copied )
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to restore backup file" );
		return;
	}
	
	// Close database if it already exists
	
	g_dbs_mutex.lock(); // --------------------------

	auto it = g_dbs.find( db_name );
	if( it != g_dbs.end() )
	{
		auto db = it->second;
		g_dbs.erase( it );
		
		g_dbs_mutex.unlock(); // --------------------------
		
		// Wait for other threads that use this database now
		size_t count = 0;
		while( !db.unique() && count < 100 )
			cr_sleep( 100 );
		
		if( !db.unique() )
		{
			conn.respond( CR_HTTP_INTERNAL_ERROR, "Database is in use, please try again later" );
			goto err;
		}
	}
	else
	{
		g_dbs_mutex.unlock(); // --------------------------
	}

	// Move database
	
	old_path = db_path + ".old";
	
	if( rename( db_path.c_str(), old_path.c_str() ) )
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to move original file" );
		goto err;
	}
	
	if( rename( restore_path.c_str(), db_path.c_str() ) )
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to move backup file" );
		rename( old_path.c_str(), db_path.c_str() );
		goto err;
	}
	
	remove( old_path.c_str() );	
	
	// Open database
	
	db = database::open( db_path );
	if( db )
	{
		g_dbs_mutex.lock();
		g_dbs[ db_name ] = db;
		g_dbs_mutex.unlock();
	
		cr_result r( conn );
		r.add_property( "time", tc.value() );
		conn.respond( CR_HTTP_OK, r );
	}
	else
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to open database after restore" );
	}
	
	return;
	
err:
	remove( restore_path.c_str() );
	
	db = database::open( db_path );
	if( db )
	{
		g_dbs_mutex.lock();
		g_dbs[ db_name ] = db;
		g_dbs_mutex.unlock();
	}
}
