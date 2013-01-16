/**********************************************************************************************/		
/* Expression.h																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"
#include "Value.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// class Variable uses to store expression
//
class Expression
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
								Expression(
									Parser&			parser,
									const wstring&	text,
									EValueType		type = kUnknown );
									
								~Expression( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Properties		

		// <Value> [r/o]
								/** Returns value of the expression. */
		const Value&			get_Value( void );

		// <Values> [r/o]
								/** Returns values from the expression. */
		vector<Value>			get_Values( void );
		
	
	public://///////////////////////////////////////////////////////////////////////////////////
	
	// -----------------------
	// Methods

								/** Returns copy of itself. */
		Expression*				Clone( VMachine* vm );
			
				
	protected://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods
	
								/** Use prepared statement to evaluate expression. */
		void					ApplyBinds( void );
		
								/** If expression if just variable - returns pointer to it. */
		Variable*				GetVariable( void );
		
								/** Parse expression's text to define kind of expression and
								 *  fill @mBinds member. */
		void					Parse( Parser& parser );
		
								/** Check is expression is number. */
		bool					ParseAsNumber( void );
		
								/** Check is expression is string. */
		bool					ParseAsString( void );
		
								/** Check is expression is variable name. */
		bool					ParseAsVariable( Parser& parser );
		
								/** Update mStmt member. */
		void					Prepare( void );
	

	private://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Private constructor

								Expression( void );


	public://///////////////////////////////////////////////////////////////////////////////////

// References

		sqlite3*				mpDB;			// Used database
		sqlite3_stmt*			mpStmt;			// Cached statement
		VMachine*				mpVM;			// Parent Virtual Machine


// Cached data

		vector<int>				mBinds;			// Used variables
		EExpressionKind			mKind;			// Kind of expression
		vector<char>			mQuery;			// Generated query to evaluate expression's value
		Value					mValue;			// Result value
		int						mVariableIndex;	// If expression is just variable name - store index to it


// Properties

		wstring*				mpText;			// Expression's text
		
		
// Flags
		
		bool					mPure;			// TRUE if expression doesn't use database
};


/**********************************************************************************************/
SP_NAMESPACE_END
