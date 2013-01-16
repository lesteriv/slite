/**********************************************************************************************/
/* API_users.cpp																			  */
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
static cr_result_fields g_user_fields =
{
	{ "name", CR_TEXT },
	{ "admin", CR_INTEGER }
};

/**********************************************************************************************/
static string g_user_name = "user";


//////////////////////////////////////////////////////////////////////////
// API users
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
GET_ADMIN( users )( cr_connection& conn )
{
	cr_result r( conn );
	r.set_record_fields( g_user_fields );
	r.set_record_name( g_user_name );
	
	auto users = the_cr_user_manager.get_users();
	for( string& name : users )
	{
		r.add_text( name );
		r.add_int( the_cr_user_manager.get_user_is_admin( name ) );
	}
	
	conn.respond( CR_HTTP_OK, r );
}

/**********************************************************************************************/
POST_ADMIN( users )( cr_connection& conn )
{
	auto name  = conn.query_parameter( "name", 4 );
	auto pass  = conn.query_parameter( "password", 8 );
	auto admin = conn.query_parameter( "admin", 5 );
	
	auto err = the_cr_user_manager.add_user( name, pass, *admin == '1' );
	err ?
		conn.respond( CR_HTTP_BAD_REQUEST, err ) :
		conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
DELETE_ADMIN( users/<user> )( cr_connection& conn )
{
	auto name = conn.path_parameter( 0 );

	auto err = the_cr_user_manager.delete_user( name );
	err ?
		conn.respond( CR_HTTP_BAD_REQUEST, err ) :
		conn.respond( CR_HTTP_OK, "ok" );
}

/**********************************************************************************************/
PUT_ADMIN( users/<user> )( cr_connection& conn )
{
	// TODO
	
	auto name  = conn.path_parameter( 0 );
	auto admin = conn.query_parameter( "admin", 5 );

	auto err = the_cr_user_manager.update_user_is_admin( name, *admin == '1' );
	err ?
		conn.respond( CR_HTTP_BAD_REQUEST, err ) :
		conn.respond( CR_HTTP_OK, "ok" );	
}
