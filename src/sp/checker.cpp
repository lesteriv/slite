/**********************************************************************************************/		
/* Checker.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SP
#include "checker.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
wstring Checker::Check( VMachine& vm )
{
	wstring res;
	mpVM = &vm;
	
	res += FindDuplicates();
	res += FindUnused();
	
	return res;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
wstring Checker::FindDuplicates( void )
{
	wstring res;
	return res;
}

/**********************************************************************************************/
wstring Checker::FindUnused( void )
{
	wstring res;
	return res;
}


/**********************************************************************************************/
SP_NAMESPACE_END
