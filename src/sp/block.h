/**********************************************************************************************/
/* Block.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// SP
#include "Variable.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Struct Block used to store block's related data
//
struct Block
{
	CursorMap				mCursors;	// Cursors
	wstring					mLabel;		// Labels for block
	hmap<wstring,size_t>	mLabels;	// Labels inside block
	TypeMap					mRecords;	// Variables with compound type
	size_t					mStart;		// Index of first command inside block
	TypeMap					mTypes;		// SubTypes
	VariableMap				mVarsMap;	// Map to find variable by name
};


/**********************************************************************************************/
SP_NAMESPACE_END
