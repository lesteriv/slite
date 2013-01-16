/**********************************************************************************************/
/* Server_API_Server.cpp		  	                                               			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SQLITE
#include "../third/sqlite/sqlite3.h"

// SLITE
#include "../slite.h"


//////////////////////////////////////////////////////////////////////////
// API server
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
POST_ADMIN( server/shutdown )( cr_connection& conn )
{
	g_dbs_mutex.lock();
	g_dbs.clear();
	g_dbs_mutex.unlock();

	conn.respond( CR_HTTP_ACCEPTED );
	cr_stop();
}

/**********************************************************************************************/
GET( server/stats )( cr_connection& conn )
{
	cr_result r( conn );
	r.add_property( "queries_cache_hit_count"	, to_string( g_cache_used ) );
	r.add_property( "requests"					, to_string( cr_request_count()	) );
	r.add_property( "uptime"					, format_time( ( time( NULL ) - g_time_start ) * 1000 ) );
	
	conn.respond( CR_HTTP_OK, r.data() );
}

/**********************************************************************************************/
GET( server/version )( cr_connection& conn )
{
	cr_result r( conn );
	r.add_property( "Slite"		, SLITE_VERSION_STRING );
	r.add_property( "SQLite"	, SQLITE_VERSION );
	
	conn.respond( CR_HTTP_OK, r.data() );
}
