/**********************************************************************************************/		
/* Cursor.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Field.h"
#include "Unicode.h"
#include "Utils.h"
#include "Variable.h"
#include "VMachine.h"
#include "Parser.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Cursor::Cursor(
	Parser*			parser,
	const wstring&	query )
:
// References
	mpStmt		( NULL ),
	mpVM		( &(parser->mVM) ),

// Source data
	mQuery		( query ),

// Attribures
	mFound		( -1 ),
	mIsOpen		( -1 ),
	mNotFound	( -1 )
{
	mRowType.mNotNULL = true;
	mpStatQ = new Statement( *parser, query );
}

/**********************************************************************************************/
Cursor::Cursor( sqlite3_stmt* stmt )
:
// References
	mpStmt		( stmt ),
	mpVM		( NULL ),
	
// Cached data
	mpStatQ		( NULL )	
{
	FillFields();
}

/**********************************************************************************************/
Cursor::~Cursor( void )
{
	CLEAR( mFields );
	
	if( mpStatQ )
		delete mpStatQ;
	else
		FINALIZE_STATEMENT( mpStmt );
}


//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Field* Cursor::get_Field( const wstring& name ) const
{
	auto it = mFieldsMap.find( name );
	return it != mFieldsMap.end() ?
		it->second :
		NULL;
}

/**********************************************************************************************/
const Type& Cursor::get_RowType( void ) const
{
	if( mRowType.mNotNULL )
		ReadTableStructure( mRowType, mpVM->mpDB, mQuery );
	
	return mRowType;
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Cursor::AssignVariables(
	int		found,
	int		isOpen,
	int		notFound )
{
	mFound		= found;
	mIsOpen		= isOpen;
	mNotFound	= notFound;
}

/**********************************************************************************************/
Cursor* Cursor::Clone( VMachine* vm )
{
	Cursor* res = new Cursor( *this, vm );

	res->mFound		= mFound;
	res->mIsOpen	= mIsOpen;
	res->mNotFound	= mNotFound;

	res->GetVarFound()->AssignValue( 0 );
	res->GetVarIsOpen()->AssignValue( 0 );
	res->GetVarNotFound()->AssignValue( 1 );

	return res;
}

/**********************************************************************************************/
bool Cursor::Close( void )
{
	FINALIZE_STATEMENT( mpStmt );
	if( mpStatQ )
		mpStatQ->mpStmt = NULL;

	GetVarFound()->AssignValue( false );
	GetVarIsOpen()->AssignValue( false );
	GetVarNotFound()->AssignValue( true );

	return true;
}

/**********************************************************************************************/
bool Cursor::FirstRecord( void )
{
	return
		sqlite3_reset( mpStmt ) == SQLITE_OK &&
		NextRecord();
}

/**********************************************************************************************/
bool Cursor::NextRecord( void )
{
	bool res = sqlite3_step( mpStmt ) == SQLITE_ROW;
	
	if( res )
	{
		size_t count = mFields.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			const char* str = (const char*) sqlite3_column_text( mpStmt, (int) i );
			if( str )
			{
				mFields[ i ]->mIsNULL = false;
				FROM_UTF8( str, mFields[ i ]->mStr );
			}
			else
			{
				mFields[ i ]->mIsNULL = true;
			}
		}
	}
	
	return res;
}

/**********************************************************************************************/
bool Cursor::Open( void )
{
	bool res = OpenInternal();

	GetVarFound()->AssignValue( false );
	GetVarIsOpen()->AssignValue( res );
	GetVarNotFound()->AssignValue( true );

	return res;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Cursor::FillFields( void )
{
	const char*	s;
	wstring		name;

	int count = sqlite3_column_count( mpStmt );
	for( int i = 0 ; i < count ; ++i )
	{
		s = sqlite3_column_name( mpStmt, i );
		FROM_UTF8( s, name );

		Field* fld = new Field;
		mFields.push_back( fld );
		mFieldsMap[ name ] = fld;
	}	
}

/**********************************************************************************************/
Variable* Cursor::GetVarFound( void ) const
{
	return mpVM->mVariables[ mFound ];
}

/**********************************************************************************************/
Variable* Cursor::GetVarIsOpen( void ) const
{
	return mpVM->mVariables[ mIsOpen ];
}

/**********************************************************************************************/
Variable* Cursor::GetVarNotFound( void ) const
{
	return mpVM->mVariables[ mNotFound ];
}

/**********************************************************************************************/
bool Cursor::OpenInternal( void )
{
	if( mpStmt )
		return FirstRecord();

	if( !mpStatQ )
		return false;
		
	if( !mpStatQ->mpStmt )
		mpStatQ->Prepare();
	
	mpStmt = mpStatQ->mpStmt;
	return mpStmt != NULL;
}


//////////////////////////////////////////////////////////////////////////
// private constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Cursor::Cursor(
	const Cursor&	cr,
	VMachine*		vm )
:
// References
	mpStmt	( NULL ),
	mpVM	( vm )
{
	if( cr.mpStatQ )
		mpStatQ = cr.mpStatQ->Clone( vm );
	else
		mpStatQ = NULL;
}


/**********************************************************************************************/
SP_NAMESPACE_END
