/**********************************************************************************************/		
/* Cursor.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"
#include "Field.h"
#include "Type.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Class Cursor uses to store queries result
//
class Cursor
{
	public://///////////////////////////////////////////////////////////////////////////////////

									Cursor(
										Parser*			parser,
										const wstring&	query );

									Cursor( sqlite3_stmt* stmt );

									~Cursor( void );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		
		
	// -----------------------
	// Properties		
		
		// <Field> [r/o]
									/** Returns pointer to the field by field's name or index. */
		Field*						get_Field( const wstring& name ) const;

		// <RowType> [r/o]
									/** Returns type to create variables with rowtype from cursor. */
		const Type&					get_RowType( void ) const;


	public://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Methods

									/** Assign variables to update attributes. */
		void						AssignVariables(
										int		found,
										int		isOpen,
										int		notFound );

									/** Returns copy of itself. */
		Cursor*						Clone( VMachine* vm );

									/** Closes cursor and update attributes. */
		bool						Close( void );

									/** Reset cursor's position to first record. */
		bool						FirstRecord( void );
		
									/** Move cursor's position to the next record. */
		bool						NextRecord( void );
		
									/** Opens cursor and update attributes. */
		bool						Open( void );

		
	public://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods

									/** Fill @mFields with wrappers for child fields.  */
		void						FillFields( void );
		
									/** Returns pointers to attributes. */
		Variable*					GetVarFound( void ) const;
		Variable*					GetVarIsOpen( void ) const;
		Variable*					GetVarNotFound( void ) const;

									/** Opens cursor. */
		bool						OpenInternal( void );


	private://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Private constructor to clone
		
									Cursor(
										const Cursor&	cr,
										VMachine*		vm );

									
	public://///////////////////////////////////////////////////////////////////////////////////

// References

		sqlite3_stmt*				mpStmt;
		VMachine*					mpVM;


// Source data

		wstring						mQuery;


// Attributes

		int							mFound;
		int							mIsOpen;
		int							mNotFound;


// Cached data

		vector<Field*>				mFields;
		hmap<wstring,Field*>		mFieldsMap;
mutable	Type						mRowType;
		Statement*					mpStatQ;
};


/**********************************************************************************************/
typedef hmap<wstring,Cursor*> CursorMap;


/**********************************************************************************************/
SP_NAMESPACE_END
