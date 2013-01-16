/**********************************************************************************************/
/* GlobalVariables.h																		  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Functions

					/** Adds new global constant. */
Variable*			AddGlobalConstant(
						const wstring&	name,
						const Value&	value );

					/** Adds new global variable. */
Variable*			AddGlobalVariable(
						const wstring&	name,
						const Value&	value );

					/** Returns global variable or constant by name,
					 *  @name - lower-cased name of variable. */
Variable*			FindGlobalVariable( const wstring& name );


/**********************************************************************************************/
SP_NAMESPACE_END
