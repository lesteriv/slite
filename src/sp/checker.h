/**********************************************************************************************/		
/* Checker.h																				  */
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
// Checker class uses to show warnings about code
//
class Checker
{
	public://////////////////////////////////////////////////////////////////////////

// This class API:

	// -----------------------
	// Methods

								/** Check for warnings. */
		wstring					Check( VMachine& vm );

        
	protected://////////////////////////////////////////////////////////////////////////
		
	// -----------------------
	// Internal methods

								/** Search for variables/cursors/etc. that have the same name
								 *  in nested blocks. */
		wstring					FindDuplicates( void );

								/** Search for variables/types/etc. that were defined but not
								 *  used in any expression or command. */
		wstring					FindUnused( void );
		
		
	protected://////////////////////////////////////////////////////////////////////////
		
// References
		
		VMachine*				mpVM;
};


/**********************************************************************************************/
SP_NAMESPACE_END
