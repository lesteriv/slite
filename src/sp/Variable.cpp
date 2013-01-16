/**********************************************************************************************/		
/* Variable.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Variable.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Variable::Variable( void )
:
// Flags
	mIsNULL		( true ),
	mNotNULL	( false ),
	mReadOnly	( false )
{
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Variable::AssignValue( const Value& vl )
{
	// Assign new value but keep original type
	EValueType type = mValue.mType;
	mValue = vl;

	if( type != kUnknown && type != mValue.mType )
		mValue.Convert( type );

	// NULL value
	if( vl.mType == kUnknown )
	{
		if( mNotNULL )
			mValue.mType = kText;
		else
			mIsNULL = true;
	}
	// Not NULL
	else
	{
		mIsNULL = false;
	}
}

/**********************************************************************************************/
void Variable::SetNotNULL( bool value )
{
	mNotNULL = value;

	if( value )
	{
		mIsNULL = false;
		
		if( mValue.mType == kUnknown )
			mValue.mType = kText;
	}
}


/**********************************************************************************************/
SP_NAMESPACE_END
