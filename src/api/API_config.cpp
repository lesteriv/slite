/**********************************************************************************************/
/* API_config.cpp																			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
inline void add_auth( cr_result& r )
{
	r.add_property( "auth", cr_get_auth_name( the_config.get_auth_kind() ) );
}

/**********************************************************************************************/
inline void add_default_format( cr_result& r )
{
	r.add_property( "default_format", cr_get_format_name( the_config.get_default_format() ) );
}

/**********************************************************************************************/
inline void add_log( cr_result& r )
{
	r.add_property( "log", the_config.get_log_enabled() ? "1" : "0" );
}

/**********************************************************************************************/
inline void add_ports( cr_result& r )
{
	r.add_property( "ports", the_config.get_ports() );
}

/**********************************************************************************************/
inline void add_threads( cr_result& r )
{
	r.add_property( "threads", to_string( the_config.get_thread_count() ) );
}


//////////////////////////////////////////////////////////////////////////
// API config
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
DELETE_ADMIN( config )( cr_connection& conn )
{
	the_config.set_auth_kind		( "none" );
	the_config.set_default_format	( "json" );
	the_config.set_log_enabled		( "1" );
	the_config.set_ports			( "15555" );
	the_config.set_thread_count		( "8" );

	the_config.flush();
	conn.respond( CR_HTTP_OK, "You need restart server to apply changes." );
}

/**********************************************************************************************/
GET_ADMIN( config )( cr_connection& conn )
{
	cr_result r( conn );
	add_auth( r );
	add_default_format( r );
	add_log( r );
	add_ports( r );
	add_threads( r );
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
GET_ADMIN( config/<key> )( cr_connection& conn )
{
	cr_result r( conn );
	
	string key = conn.path_parameter( 0 );
	if( key == "auth" )
		add_auth( r );
	if( key == "default_format" )
		add_default_format( r );
	else if( key == "log" )
		add_log( r );
	else if( key == "ports" )
		add_ports( r );
	else if( key == "threads" )
		add_threads( r );
	else
		return conn.respond( CR_HTTP_BAD_REQUEST, "Invalid config's entry name: " + key );
		
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
PUT_ADMIN( config/<key> )( cr_connection& conn )
{
	bool need_restart = false;
	
	string msg;
	string key	 = conn.path_parameter( 0 );
	string value = conn.query_parameter( "value" );
	
	// auth
	if( key == "auth" )
	{
		msg = the_config.set_auth_kind( value.c_str() );
	}
	// default_format
	else if( key == "default_format" )
	{
		msg = the_config.set_default_format( value.c_str() );
	}
	// log
	else if( key == "log" )
	{
		msg = the_config.set_log_enabled( value.c_str() );
	}
	// ports
	else if( key == "ports" )
	{
		msg = the_config.set_ports( value );
		need_restart = true;
	}
	// threads
	else if( key == "threads" )
	{
		msg = the_config.set_thread_count( value.c_str() );
		need_restart = true;
	}
	
	if( msg.empty() )
		the_config.flush();
	
	else if( msg.length() )
		conn.respond( CR_HTTP_BAD_REQUEST, "Invalid value - '" + value + "', " + msg );
	if( need_restart )
		conn.respond( CR_HTTP_OK, "You need restart server to apply changes" );
	else
		conn.respond( CR_HTTP_OK );
}
