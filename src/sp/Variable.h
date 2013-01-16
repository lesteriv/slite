/**********************************************************************************************/		
/* Variable.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "Value.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Class Variable uses to store value of one variable from stored procedure
//
class Variable
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
								Variable( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Methods

								/** Assign value without any checks. */
		void					AssignValue( const Value& vl );

								/** Set NotNULL flag. */
		void					SetNotNULL( bool value );
		
								/** Set type of variable. */
		void					SetType( EValueType type ) { mValue.mType = type; }

		
	public://///////////////////////////////////////////////////////////////////////////////////

// Properties

		Value					mValue;				// Current value of variable

// Flags

		bool					mIsNULL		: 1;	// TRUE if value is NULL
		bool					mNotNULL	: 1;	// TRUE if we cannot assign NULL as value
		bool					mReadOnly	: 1;	// TRUE if we must not change value
};


/**********************************************************************************************/
typedef hmap<wstring,Variable*> VariableMap;


/**********************************************************************************************/
SP_NAMESPACE_END
