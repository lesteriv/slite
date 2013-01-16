/**********************************************************************************************/
/* Loop.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "Enums.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Struct Block used to store block's related data
//
struct Loop
{
	Loop(
		ELoop	kind,
		int		start,
		wstring	label )
	:
		mKind( kind ),
		mLabel( label ),
		mStart( start )
	{
	}
	
	vector<Command*>	mEnds;
	ELoop				mKind;
	wstring				mLabel;
	int					mStart;
};


/**********************************************************************************************/
SP_NAMESPACE_END
