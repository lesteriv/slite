/**********************************************************************************************/
/* Server_API_Table.cpp		  	                                               				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static string g_empty_string;


//////////////////////////////////////////////////////////////////////////
// API table
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
DELETE( records/<db>/<table> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string query = string( "DELETE FROM \"" ) + conn.path_parameter( 1 ) + '"';

	string filter = conn.query_parameter( "filter" );
	if( filter.length() )
		query += " WHERE " + filter;

	db->query( conn, query, false );
}

/**********************************************************************************************/
DELETE( records/<db>/<table>/<rec_id> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string query =
		string( "DELETE FROM \"" ) + conn.path_parameter( 1 ) +
		"\" WHERE oid=" + conn.path_parameter( 2 );
	
	db->query( conn, query, false );
}

/**********************************************************************************************/
GET( records/<db>/<table> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;
	
	string query = string( "SELECT * FROM \"" ) + conn.path_parameter( 1 ) + "\"";

	string filter = conn.query_parameter( "filter" );
	if( filter.length() )
		query += " WHERE " + filter;

	string limit = conn.query_parameter( "limit" );
	if( limit.length() )
		query += " LIMIT " + limit;

	db->query( conn, query, false );
}

/**********************************************************************************************/
GET( records/<db>/<table>/<id> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;
	
	string query =
		string( "SELECT * FROM \"" ) + conn.path_parameter( 1 ) +
		"\" WHERE oid=" + conn.path_parameter( 2 );
	
	db->query( conn, query, false );
}

/**********************************************************************************************/
POST( records/<db>/<table> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	db->insert( conn, conn.path_parameter( 1 ) );
}

/**********************************************************************************************/
PUT( records/<db>/<table> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	db->update(
		conn, conn.path_parameter( 1 ), g_empty_string,
		conn.query_parameter( "filter" ) );
}

/**********************************************************************************************/
PUT( records/<db>/<table>/<id> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	db->update(
		conn, conn.path_parameter( 1 ), conn.path_parameter( 2 ),
		g_empty_string );
}
