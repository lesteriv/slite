/**********************************************************************************************/		
/* Statement.h																				  */
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
// class Statement used to store SQLite's SQL statement
//
class Statement
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
								Statement(
									Parser&			parser,
									const wstring&	text );
									
								~Statement( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Methods

								/** Returns copy of itself. */
		Statement*				Clone( VMachine* vm );
			
								/** Use prepared statement. */
		bool					Execute( void );

		
	public://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods
	
								/** Apply binds to current statement. */
		void					ApplyBinds( void );
		
								/** Parse expression's text to define kind of expression and
								 *  fill @mBinds member. */
		void					Parse( Parser& parser );
		
								/** Update mStmt member. */
		void					Prepare( void );
	

	private://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Private constructor

								Statement( void );


	public://///////////////////////////////////////////////////////////////////////////////////

// References

		sqlite3*				mpDB;			// Used database
		sqlite3_stmt*			mpStmt;			// Cached statement
		VMachine*				mpVM;			// Parent Virtual Machine


// Cached data

		vector<int>				mBinds;			// Used variables
		vector<char>			mQuery;			// Generated query to evaluate expression's value


// Properties

		wstring*				mpText;			// Expression's text
};


/**********************************************************************************************/
SP_NAMESPACE_END
