/**********************************************************************************************/
/* API_procedures.cpp																		  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// API procedures
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
GET( procedures/<db> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( db )
		db->query( conn, "SELECT name FROM sqlite_sp_functions ORDER BY name" );
}

/**********************************************************************************************/
POST( procedures/<db> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string name = conn.query_parameter( "name" );

	const char* clen = conn.header( "Content-Length" );
	int len = clen ? atoi( clen ) : 0;

	string text;
	text.resize( len );
	text.resize( conn.read( &text[ 0 ], len ) );	
	
	escape_sql( name );
	escape_sql( text );
	
	string r;
	if( db->evaluate( "SELECT sp_create('" + name + "','" + text + "')", r ) )
		conn.respond( CR_HTTP_OK, "ok" );
	else
		conn.respond( CR_HTTP_BAD_REQUEST, r );
}

/**********************************************************************************************/
DELETE( procedures/<db>/<proc> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string name = conn.path_parameter( 1 );
	escape_sql( name );
	
	string r;
	if( db->evaluate( "SELECT sp_drop('" + name + "')", r ) )
		conn.respond( CR_HTTP_OK, "ok" );
	else
		conn.respond( CR_HTTP_BAD_REQUEST, r );
}

/**********************************************************************************************/
GET( procedures/<db>/<proc> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string name = conn.path_parameter( 1 );
	escape_sql( name );
	
	string text;
	if( db->evaluate( "SELECT sp_show('" + name + "')", text ) )
	{
		cr_result r( conn );
		r.add_property( "text", text );
		conn.respond( CR_HTTP_OK, r.data() );
	}
	else
	{
		conn.respond( CR_HTTP_BAD_REQUEST, text );
	}
}

/**********************************************************************************************/
PUT( procedures/<db>/<proc> )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;
	
	string name = conn.path_parameter( 1 );
	escape_sql( name );
	
	const char* clen = conn.header( "Content-Length" );
	int len = clen ? atoi( clen ) : 0;

	string text;
	text.resize( len );
	text.resize( conn.read( &text[ 0 ], len ) );	
	escape_sql( text );
	
	string r;
	if( db->evaluate( "SELECT sp_change('" + name + "','" + text + "')", r ) )
		conn.respond( CR_HTTP_OK, "ok" );
	else
		conn.respond( CR_HTTP_BAD_REQUEST, r );
}

/**********************************************************************************************/
POST( procedures/<db>/<proc>/call )( cr_connection& conn )
{
	auto db = get_database( conn );
	if( !db )
		return;

	string name = conn.path_parameter( 1 );
	
	string value;
	if( db->evaluate( "SELECT \"" + name + "\"()", value ) )
	{
		cr_result r( conn );
		r.add_property( "result", value );
		conn.respond( CR_HTTP_OK, r.data() );
	}
	else
	{
		conn.respond( CR_HTTP_BAD_REQUEST, value );
	}
}
