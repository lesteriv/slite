/**********************************************************************************************/
/* database.h			  		                                                   			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// CREST
#include "../third/libcrest/include/crest.h"

// SQLITE
#include "../third/sqlite/sqlite3.h"

/**********************************************************************************************/
class database;

/**********************************************************************************************/
typedef shared_ptr<database> database_ptr;


/**********************************************************************************************/
// Wrapper for sqlite3
//
class database
{
	public://////////////////////////////////////////////////////////////////////////
		
								database( 
									sqlite3*		db,
									const string&	path );
								
								~database( void );

	public://////////////////////////////////////////////////////////////////////////

// This class API:

	// ---------------------
	// Methods
		
								/** Makes backup of database to file at @path. */
		void					backup( 
									cr_connection&	conn,
									const string&	path,
									const string&	db_name );

								/** Checks for database integrity. */
		void					check( cr_connection& conn );
		
								/** Creates new database, returns NULL if fail. */
static	database_ptr			create( const string& path );
		
								/** Returns first value from first column in result of query. */
		bool					evaluate( 
									const string&	query,
									string&			result );

								/** Flushes all changes to disk. */
		void					flush( cr_connection& conn );
		
								/** Tries to adds new record and returns HTTP responce with result. */
		void					insert(
									cr_connection&	conn,
									const string&	table );

								/** Causes any pending database operation to abort. */
		void					interrupt( cr_connection& conn );
		
								/** Opens existed database, returns NULL if fail. */
static	database_ptr			open( const string& path );
		
								/** Returns HTTP responce with data from query. */
		void					query(
									cr_connection&	conn,
									const string&	query,
									bool			add_id = true );
		
								/** Returns statistics for the database. */
		void					stats( cr_connection& conn );
		
								/** Tries to update existing record and returns HTTP responce with result. */
		void					update(
									cr_connection&	conn,
									const string&	table,
									const string&	id,
									const string&	filter );
		

	protected://////////////////////////////////////////////////////////////////////////
		
	// ---------------------
	// Cache methods
		
		void					cache_add(
									const string&		query,
									cr_result_format	format,
									bool				compressed,
									shared_ptr<string>	data );
		
		shared_ptr<string>		cache_find(
									const string&		query,
									cr_result_format	format,
									bool				compressed );

		
	protected://////////////////////////////////////////////////////////////////////////
		
	// ---------------------
	// Util methods

		cr_http_status			execute(
									string&			out,
									cr_connection&	conn,
									sqlite3_stmt*	stmt,
									bool			add_id );
		
		string					query_insert(
									const string&			table,
									const cr_string_map&	params );
		
		string					query_update(
									const string&			table,
									const string&			id,
									const string&			filter,
									const cr_string_map&	params );
		
		
	protected://////////////////////////////////////////////////////////////////////////
		
		struct cache_entry
		{
			bool				compressed;		// is content compressed
			shared_ptr<string>	data;			// stored http response
			cr_result_format	format;			// format of responce
			size_t				query_hash;		// hash to optimize search in cache
			string				query;			// original query for responce
		};

		
	protected://////////////////////////////////////////////////////////////////////////
		
// References
		
		sqlite3*				db_;
		
		
// Cache
		
		list<cache_entry>		cache_;
		mutex					cache_mutex_;
		
		
// Other
		
		string					path_;
		mutex					query_mutex_;
		
		
// Statistics
		
		size_t					cache_hits_	= 0;
		size_t					queries_	= 0;
};
