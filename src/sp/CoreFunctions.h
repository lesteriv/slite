/**********************************************************************************************/
/* CoreFunctions.h																			  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Functions

				/** Registers core functions for the database. */
void			RegisterFunctions( sqlite3* db );


/**********************************************************************************************/
SP_NAMESPACE_END
