/**********************************************************************************************/
/* slite.h			  		                                                   				  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// STD
#include <cstring>
#include <fstream>
#include <list>
#include <unordered_map>

/**********************************************************************************************/
using namespace std;

// CREST
#include "third/libcrest/include/crest.h"

// SLITE
#include "version.h"
#include "db/database.h"
#include "utils/config.h"
#include "utils/time_counter.h"
#include "utils/utils.h"


/**********************************************************************************************/
typedef unordered_map<string,database_ptr> databases_t;

/**********************************************************************************************/
extern uint64_t		g_cache_used;
extern databases_t	g_dbs;
extern mutex		g_dbs_mutex;
extern string		g_path_backup;
extern string		g_path_db;
extern string		g_path_log;
extern time_t		g_time_start;
