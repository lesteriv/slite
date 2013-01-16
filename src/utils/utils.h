/**********************************************************************************************/
/* utils.h			  		                                                   				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once


/**********************************************************************************************/
// Functions

						/** Returns path to slite binary parent directory. */
string					app_path( void );

						/** Escape string to use as string literal in SQL query. */
void					escape_sql( string& str );

						/** Returns formated time interval name. */
string					format_time( int ms );

						/** Returns destination database for connection.  */
database_ptr			get_database( cr_connection& conn );

						/** Returns list of files in directory. */
vector<string>			get_files( const string& path );
