/**********************************************************************************************/
/* database.cpp				                                                  				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SLITE
#include "../sp/core.h"
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// construction
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
database::database( 
	sqlite3*		db,
	const string&	path )
:
// References
	db_( db ),
	
// Other
	path_( path )
{
}

/**********************************************************************************************/
database::~database( void )
{
	sqlite3_close_v2( db_ );
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void database::backup(
	cr_connection&	conn,
	const string&	path,
	const string&	db_name )
{
	time_counter tc;
	
	sqlite3* file;
	int rc = sqlite3_open( path.c_str(), &file );
	if( rc != SQLITE_OK )
	{
		conn.respond( CR_HTTP_INTERNAL_ERROR, "Unable to create backup file: " + path );
		return;
	}
	
	sqlite3_exec( file, "PRAGMA journal_mode = OFF", 0, 0, 0 );
	sqlite3_exec( file, "PRAGMA synchronous = 0", 0, 0, 0 );
		
	sqlite3_backup* backup = sqlite3_backup_init( file, "main", db_, "main" );
	if( backup )
	{
		do
		{
			rc = sqlite3_backup_step( backup, 100 );
			sqlite3_sleep( 10 );
		}
		while( rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED );

		sqlite3_backup_finish( backup );
	}

	string err = sqlite3_errmsg( file );
	rc = sqlite3_errcode( file );
	sqlite3_close( file );
	
	cr_result r( conn );
	if( rc ) r.add_property( "message", err );		
	r.add_property( "file", db_name );
	r.add_property( "time", tc.value() );
	
	conn.respond( rc ? CR_HTTP_INTERNAL_ERROR : CR_HTTP_CREATED, r );
}

/**********************************************************************************************/
void database::check( cr_connection& conn )
{
	string status;
	evaluate( "PRAGMA integrity_check", status );
	
	conn.respond( CR_HTTP_OK, status );
}

/**********************************************************************************************/
database_ptr database::create( const string& path )
{
	sqlite3* db = 0;
	
	if( !cr_file_exists( path ) )
	{
		sqlite3_open( path.c_str(), &db );
		sqlite3_exec( db, "PRAGMA journal_mode = WAL", 0, 0, 0 );
		sqlite3_exec( db, "PRAGMA synchronous = 0", 0, 0, 0 );
		
		sp_init( db );
	}
	
	return database_ptr( db ? new database( db, path ) : 0 );
}

/**********************************************************************************************/
bool database::evaluate( 
	const string&	query,
	string&			result )
{
	bool res = true;
	
	query_mutex_.lock(); // ----------------------------
	
	sqlite3_stmt* stmt = 0;
	sqlite3_prepare_v2( db_, query.c_str(), query.length(), &stmt, 0 );
	if( stmt && sqlite3_column_count( stmt ) &&  sqlite3_step( stmt ) == SQLITE_ROW )
	{
		const char* vl = (const char*) sqlite3_column_text( stmt, 0 );
		if( vl )
			result.assign( vl );
	}
	else
	{
		result.assign( sqlite3_errmsg( db_ ) );
		res = false;
	}
	
	query_mutex_.unlock(); // ----------------------------
	
	sqlite3_finalize( stmt );
	return res;
}

/**********************************************************************************************/
void database::flush( cr_connection& conn )
{
	// We need mutex to retreive correct error's message
	query_mutex_.lock(); // ----------------------------
	
	int rc = sqlite3_wal_checkpoint_v2( db_, 0, SQLITE_CHECKPOINT_PASSIVE, 0, 0 );
	( rc == SQLITE_OK ) ?
		conn.respond( CR_HTTP_OK, "ok" ) :
		conn.respond( CR_HTTP_INTERNAL_ERROR, sqlite3_errmsg( db_ ) );
	
	query_mutex_.unlock(); // ----------------------------
}

/**********************************************************************************************/
void database::insert(
	cr_connection&	conn,
	const string&	table )
{
	auto			params	= conn.post_parameters();
	string			query	= query_insert( table, params );
	sqlite3_stmt*	stmt	= 0;
	
	// We need mutex to retreive correct error's message
	query_mutex_.lock(); // --------------------------

	// Prepare query
	sqlite3_prepare_v2( db_, query.c_str(), query.length(), &stmt, 0 );
	
	// Returns error on fail
	if( !stmt )
	{
		string err( sqlite3_errmsg( db_ ) );
		int rc = sqlite3_errcode( db_ );
		
		query_mutex_.unlock(); // --------------------------
		
		conn.respond( rc ? CR_HTTP_INTERNAL_ERROR : CR_HTTP_OK, err );
		return;
	}
		
	query_mutex_.unlock(); // --------------------------

	// Bind values
	size_t count = params.size();
	for( size_t i = 0 ; i < count ; ++i )
		sqlite3_bind_text( stmt, i + 1, params.value( i ), params.value_len( i ), SQLITE_STATIC );

	string res;
	execute( res, conn, stmt, true );
	sqlite3_finalize( stmt );
	
	conn.write( res );
}

/**********************************************************************************************/
void database::interrupt( cr_connection& conn )
{
	sqlite3_interrupt( db_ );
	conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
database_ptr database::open( const string& path )
{
	sqlite3* db = 0;

	if( cr_file_exists( path ) )
	{
		sqlite3_open( path.c_str(), &db );
		sqlite3_exec( db, "PRAGMA journal_mode = WAL", 0, 0, 0 );
		sqlite3_exec( db, "PRAGMA synchronous = 0", 0, 0, 0 );
		
		sp_init( db );
	}
	
	return database_ptr( db ? new database( db, path ) : 0 );
}

/**********************************************************************************************/
void database::query( 
	cr_connection&	conn,
	const string&	query,
	bool			add_id )
{
	++queries_;
	
	bool compress = strstr( conn.header( "Accept-Encoding" ), "deflate" );
	
	// Try to find cached result
	auto cached = cache_find( query, cr_get_result_format( conn ), compress );
	if( cached )
	{
		conn.write( *cached );
		
		++cache_hits_;
		++g_cache_used;
		
		return;
	}
	
	shared_ptr<string> res( new string );
	sqlite3_stmt* stmt = 0;
	
	query_mutex_.lock(); // ------------------------------
	
	// Prepare query
	sqlite3_prepare_v2( db_, query.c_str(), query.length(), &stmt, 0 );
	int rc = sqlite3_errcode( db_ );
	
	// OK - we can execute query
	if( stmt )
	{
		query_mutex_.unlock(); // ------------------------------
		
		// Select
		int fcount = sqlite3_column_count( stmt );
		if( fcount )
		{
			cr_result_fields fields;

			// No need to use mutex - we don't want to get correct value for the query later,
			// anyway we can reset cache on any changes in database
			int ccount = sqlite3_total_changes( db_ );
			
			// Collect names and types of fields
			int step = sqlite3_step( stmt );
			for( int i = 0 ; i < fcount ; ++i )
			{
				int type = sqlite3_column_type( stmt, i );
				fields.push_back( { sqlite3_column_name( stmt, i ), (cr_value_type) type } );
			}

			cr_result r( conn );
			r.set_record_fields( fields );
			
			if( step != SQLITE_ROW )
				fcount = 0;
			
			// Process records
			do
			{
				for( int i = 0 ; i < fcount ; ++i )
				{
					switch( sqlite3_column_type( stmt, i ) )
					{
						case SQLITE_FLOAT	: r.add_double( sqlite3_column_double( stmt, i ) ); break;
						case SQLITE_INTEGER	: r.add_int( sqlite3_column_int64( stmt, i ) ); break;
						case SQLITE_NULL	: r.add_null(); break;
						
						default:
						{
							const char* text = (const char*) sqlite3_column_text( stmt, i );
							r.add_text( text ? text : "" );
						}
						break;
					}
				}
			}
			while( sqlite3_step( stmt ) == SQLITE_ROW );

			// Here were changes in database - reset cache
			if( sqlite3_total_changes( db_ ) != ccount )
			{
				cache_mutex_.lock();
				cache_.clear();
				cache_mutex_.unlock();
			}

			auto data = r.data();
				
			if( compress )
			{
				size_t out_len	= cr_compress_bound( data.length() );
				char*  out		= (char*) malloc( out_len );
				
				size_t dlen = cr_deflate( data.c_str(), data.length(), out, out_len );
				
				*res = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Encoding: deflate\r\n"
					"Content-Length: " + to_string( dlen ) + "\r\n"
					"\r\n" + string( out, dlen );
				
				free( out );
			}
			else
			{
				*res = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: " + to_string( data.length() ) + "\r\n"
					"\r\n" + data;				
			}
			
			cache_add( query, cr_get_result_format( conn ), compress, res );
		}
		// Other queries
		else
		{
			execute( *res, conn, stmt, add_id );
		}

		sqlite3_finalize( stmt );
	}
	// Error in query text
	else if( rc )
	{
		string err( sqlite3_errmsg( db_ ) );
		
		query_mutex_.unlock(); // ------------------------------
		
		cr_result r( conn );
		r.add_property( "message", err );
		*res = cr_create_responce( CR_HTTP_INTERNAL_ERROR, r );
	}
	// Empty query
	else
	{
		query_mutex_.unlock(); // ------------------------------
		
		cr_result r( conn );
		r.add_property( "message", "empty query" );
		*res = cr_create_responce( CR_HTTP_OK, r );
	}
	
	conn.write( *res );
}

/**********************************************************************************************/
void database::stats( cr_connection& conn )
{
	int value, dummy;
	
	cr_result r( conn );

	r.add_property( "executed_queries"			, to_string( queries_ ) );
	r.add_property( "queries_cache_hit_count"	, to_string( cache_hits_ ) );
	r.add_property( "size_bytes"				, to_string( cr_file_size( path_ ) ) );

	sqlite3_db_status( db_, SQLITE_DBSTATUS_CACHE_USED, &value, &dummy, 0 );
	r.add_property( "sqlite_cache_bytes"		, to_string( value ) );

	sqlite3_db_status( db_, SQLITE_DBSTATUS_CACHE_HIT, &value, &dummy, 0 );
	r.add_property( "sqlite_cache_hit_count"	, to_string( value ) );

	sqlite3_db_status( db_, SQLITE_DBSTATUS_LOOKASIDE_HIT, &dummy, &value, 0 );
	r.add_property( "sqlite_lookaside_hit_count", to_string( value ) );

	r.add_property( "sqlite_total_changes"		, to_string( sqlite3_total_changes( db_ ) ) );
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
void database::update(
	cr_connection&	conn,
	const string&	table,
	const string&	id,
	const string&	filter )
{
	const cr_string_map& params = conn.post_parameters();
	if( !params.size() )
	{
		conn.respond( CR_HTTP_BAD_REQUEST );
		return;
	}
	
	++queries_;
	
	sqlite3_stmt* stmt = 0;
	string query = query_update( table, id, filter, params );
	
	// We need mutex to get correct error's message
	query_mutex_.lock(); // -----------------------

	// Prepare query
	sqlite3_prepare_v2( db_, query.c_str(), query.length(), &stmt, 0 );
	
	// Returns error on fail
	if( !stmt )
	{
		int rc = sqlite3_errcode( db_ );
		string err( sqlite3_errmsg( db_ ) );
		
		query_mutex_.unlock(); // -----------------------
		
		conn.respond( rc ? CR_HTTP_INTERNAL_ERROR : CR_HTTP_OK, err );
		return;
	}
		
	query_mutex_.unlock(); // -----------------------

	// Bind values
	size_t count = params.size();
	for( size_t i = 0 ; i < count ; ++i )
		sqlite3_bind_text( stmt, i + 1, params.value( i ), params.value_len( i ), SQLITE_STATIC );

	string res;
	execute( res, conn, stmt, false );
	conn.write( res );
	
	sqlite3_finalize( stmt );
}

		
//////////////////////////////////////////////////////////////////////////
// cache methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void database::cache_add(
	const string&		query,
	cr_result_format	format,
	bool				compressed,
	shared_ptr<string>	data )
{
	size_t query_hash = std::hash<string>()( query );
	
	cache_mutex_.lock(); // -----------------------

	// Remove old copy if any
	for( auto it = cache_.begin() ; it != cache_.end() ; ++it )
	{
		if( it->format		== format &&
			it->compressed	== compressed &&
			it->query_hash	== query_hash &&
			it->query		== query )
		{
			cache_.erase( it );
			break;
		}
	}

	// Add new item to top
	cache_.push_front( cache_entry() );

	auto& item = cache_.front();
	item.compressed = compressed;
	item.format		= format;
	item.query		= query;
	item.query_hash = query_hash;

	item.data.swap( data );
	
	// Remove most unused items
	if( cache_.size() > 20 )
		cache_.resize( 20 );
	
	cache_mutex_.unlock(); // -----------------------
}

/**********************************************************************************************/
shared_ptr<string> database::cache_find(
	const string&		query,
	cr_result_format	format,
	bool				compressed )
{
	shared_ptr<string> res;
	size_t query_hash = std::hash<string>()( query );
	
	cache_mutex_.lock(); // ------------------------------

	for( auto it = cache_.begin() ; it != cache_.end() ; ++it )
	{
		if( it->format		== format &&
			it->compressed	== compressed &&
			it->query_hash	== query_hash &&
			it->query		== query )
		{
			// Move founded item to the top
			if( it != cache_.begin() )
			{
				// Copy item to new at top
				cache_.push_front( cache_entry() );
				
				auto& item = cache_.front();
				item.compressed = compressed;
				item.format		= format;
				item.query_hash	= query_hash;

				item.data.swap( it->data );
				item.query.swap( it->query );
				
				// Remove old item
				cache_.erase( it );
				
				res = item.data;
			}
			else
			{
				res = it->data;
			}
		}
	}

	cache_mutex_.unlock(); // ------------------------------
	
	return res;
}


//////////////////////////////////////////////////////////////////////////
// util methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
cr_http_status database::execute(
	string&			out,
	cr_connection&	conn,
	sqlite3_stmt*	stmt,
	bool			add_id )
{
	cr_http_status rc = CR_HTTP_OK;
	
	query_mutex_.lock(); // --------------------------
	
	int ccount = sqlite3_total_changes( db_ );

	// Execute query
	int qres = sqlite3_step( stmt );

	// Here were changes in database - reset cache
	bool resetCache = sqlite3_total_changes( db_ ) != ccount;

	// Query has been executed
	if( qres == SQLITE_OK || qres == SQLITE_DONE )
	{
		int changes = sqlite3_changes( db_ );
		int64_t id = sqlite3_last_insert_rowid( db_ );

		query_mutex_.unlock(); // --------------------------
		
		cr_result r( conn );
		r.add_property( "changed", to_string( changes ) );
		
		if( add_id )
			r.add_property( "last_insert_id", to_string( id ) );
		
		out = cr_create_responce( CR_HTTP_OK, r );
	}
	// Query returns error
	else
	{
		string err( sqlite3_errmsg( db_ ) );
		
		query_mutex_.unlock(); // --------------------------
		
		cr_result r( conn );
		r.add_property( "message", err );
		out = cr_create_responce( CR_HTTP_INTERNAL_ERROR, r );
		
		rc = CR_HTTP_INTERNAL_ERROR;
	}
	
	if( resetCache )
	{
		cache_mutex_.lock();
		cache_.clear();
		cache_mutex_.unlock();
	}
	
	return rc;
}

/**********************************************************************************************/
string database::query_insert(
	const string&			table,
	const cr_string_map&	params )
{
	string query = "INSERT INTO \"" + table + "\"";
	string values;

	size_t count = params.size();
	if( count )
	{
		query += '(';
		
		for( size_t i = 0 ; i < count ; ++i )
		{
			if( values.length() )
			{
				query.push_back( ',' );
				values.push_back( ',' );
			}

			query += '"';
			query += params.value( i );
			query += '"';

			values.push_back( '?' );
		}

		query += ") VALUES (";
		query += values;
		query += ')';
	}
	else
	{
		query += " DEFAULT VALUES";
	}
	
	return query;
}

/**********************************************************************************************/
string database::query_update(
	const string&			table,
	const string&			id,
	const string&			filter,
	const cr_string_map&	params )
{
	string query = "UPDATE \"" + table + "\" SET ";

	size_t count = params.size();
	for( size_t i = 0 ; i < count ; ++i )
	{
		if( i )
			query.push_back( ',' );
		
		query += '"';
		query += params.name( i );
		query += "\"=?";
	}
	
	if( id.length()		) query += " WHERE oid=" + id;
	if( filter.length() ) query += " WHERE " + filter;
	
	return query;
}
