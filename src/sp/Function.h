/**********************************************************************************************/		
/* Function.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// class Function hold data to execute stored procedure
//
class Function
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
								Function(
									sqlite3*		db,
									const wstring&	name,
									const wstring&	text );
								
								~Function( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		
		
	// -----------------------
	// Methods

		void					Compile( sqlite3_context* ctx );
static	void					Destroy( void* ptr );

		void					Execute(
									sqlite3_context*	ctx,
									int					argc,
									sqlite3_value**		argv );
		
		void					Explain( sqlite3_context* ctx );

static	Function*				FindFunction(		
									sqlite3*		db,
									const wstring&	name );

		
	public://///////////////////////////////////////////////////////////////////////////////////

// Other
		
		vector<VMachine*>		mInstances;
		VMachine				mVM;
		
		
// Properties
		
		wstring					mName;
		wstring					mText;
		

// Flags
		
		bool					mAttached	: 1;
		bool					mCompiled	: 1;
};


/**********************************************************************************************/
SP_NAMESPACE_END
