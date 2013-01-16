/**********************************************************************************************/		
/* VMachine.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "Command.h"
#include "Variable.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Class VMachine uses to execute single function
//
class VMachine
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
								VMachine( sqlite3* db );
								VMachine( const VMachine& vm );
								
								~VMachine( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Methods

								/** Push new command into 'program'. */
		Command*				AddCommand(
									ECommand	id,
									int			catchIndex );

								/** Catch exception by index and move current position to
								 *  exception's handler ( if any ). */
		void					Catch( int index );

								/** Used for 'explain' function, not for execution. */
		const wstring&			GetCursorName( Cursor* cursor ) const;

								/** Used for 'explain' function, not for execution. */
		const wstring&			GetVariableName( Variable* var ) const;

								/** Returns index of the cursor, -1 if none. */
		int						Index( Cursor* vl ) const;

								/** Returns index of the variable, -1 if none. */
		int						Index( Variable* vl ) const;

								/** Returns index of last added command. */
		int						LastCommandIndex( void ) const { return (int) mCommands.size() - 1; }
		
								/** Adds string to @mOutput buffer. */
		void					Print( const char* str );

								/** Set error string. */
		void					PrintError( const char* msg = NULL );
		
								/** Adds string to @mOutput buffer. */
		void					PrintW( const wstring& line );
		
								/** Execute function for sqlite context( query ). */
		void					Run(
									sqlite3_context*	ctx,
									int					argc,
									sqlite3_value**		argv );
		
		
	public://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods
	
								/** Deletes all commands, variables etc. */
		void					Clear( void );
		
								/** Execute current command and go to the next. */
		bool					Step( void );
	
		
	public://///////////////////////////////////////////////////////////////////////////////////

// Cached data

		int						mArgc;					// Count of arguments
		int						mArgcMin;				// Minimum count of arguments
		vector<Command*>		mCommands;				// List of commands in function
		vector<size_t>			mCatches;				// Index of start command for exception handler
		vector<Cursor*>			mCursors;				// List of cursors in function
		EValueType				mType;					// Result type
		vector<Variable*>		mVariables;				// List of variables in function
		size_t					mVarFound;				// SQL%FOUND
		size_t					mVarNotFound;			// SQL%NOTFOUND
		size_t					mVarRowCount;			// SQL%ROWCOUNT


// Data to search child by name		
		
		hmap<wstring,Cursor*>	mCursorsMap;			// Map to find cursor by name
		hmap<Variable*,wstring>	mVariablesNames;		// Map to get names of variables


// References		
		
		sqlite3*				mpDB;					// Used database
		sqlite3_context*		mpContext;				// Current context
		
		
// 'Runtime' data

		int						mArgcRun;				// Count of arguments
		sqlite3_value**			mpArgv;					// Arguments
		bool					mBusy;					// TRUE if VM is running
		vector<char>			mOutput;				// Result of prints
		int						mPos;					// Index of current running command
		size_t					mSteps;					// Counter to prevent infinite loops
};


/**********************************************************************************************/
SP_NAMESPACE_END
