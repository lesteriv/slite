/**********************************************************************************************/		
/* Command.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// SP
#include "Expression.h"
#include "Statement.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Class Command used to store and execute one command from procedure
//
class Command
{
	public://///////////////////////////////////////////////////////////////////////////////////

								Command(
									VMachine*	vm,
									ECommand	id,
									int			catchIndex );

								~Command( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Methods

								/** Returns copy of itself. */
		Command*				Clone( VMachine* vm );
		
								/** Executes command - returns FALSE on fail. */
		bool					Execute( void );
		
								/** Returns description of command. */
		wstring					Explain( void );


	protected://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal execute methods
	
		bool					DoAssign( void );			// <var> := <EXPR>
		bool					DoCloseCursor( void );		// CLOSE <cursor>
		bool					DoExec( void );				// EXEC <func>
		bool					DoFetch( void );			// FETCH <cursor> INTO ...
		bool					DoGoIf( void );				// IF <expr> GOTO <label>
		bool					DoGoIfNot( void );			// IF NOT <expr> GOTO <label>
		bool					DoOpenCursor( void );		// OPEN <cursor>
		bool					DoPrint( void );			// PRINT <expr>
		bool					DoReadParam( void );		// READ PARAMETER TO VARIABLE
		bool					DoReturn( void );			// RETURN <expr>
		bool					DoSelect( void );			// SELECT ...
	

	protected://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods

		void					Throw( void ); /* TODO: args */
	
		
	public://///////////////////////////////////////////////////////////////////////////////////

// References

mutable	Expression*				mpExpr;				// Used expression, for example, for PRINT command
mutable	Statement*				mpStmt;				// SQL statement to execute
		VMachine*				mpVM;				// Parent Virtual Machine


// Execution data

		int						mCatchIndex;		// Index of catch block on exception thrown
		
		
// Properties
		
		ECommand				mCmdID;				// Command's ID - what we will do
		vector<int>				mFields;			// Record's fields
		int						mObject;			// Used variable's or cursor's index
		Value					mParam;				// Additional parameter, for example, position for GO TO command
};


/**********************************************************************************************/
SP_NAMESPACE_END
