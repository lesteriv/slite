/**********************************************************************************************/		
/* Function.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Field.h"
#include "Function.h"
#include "Parser.h"
#include "Unicode.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// types
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
typedef pair<sqlite3*,wstring> FuncKey;


//////////////////////////////////////////////////////////////////////////
// global data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static map<FuncKey,Function*> gFunctions;		


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Function::Function(
	sqlite3*		db,
	const wstring&	name,
	const wstring&	text )
:
// Other
	mVM			( db ),

// Source
	mName		( name ),
	mText		( text ),
	
// Flags
	mAttached	( true ),
	mCompiled	( false )
{
	gFunctions[ FuncKey( db, Lower( name ) ) ] = this;
}

/**********************************************************************************************/
Function::~Function( void )
{
	auto it = gFunctions.begin();
	for( ; it != gFunctions.end() ; ++it )
	{
		if( it->second == this )
		{
			gFunctions.erase( it );
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Function::Compile( sqlite3_context* ctx )
{
	Parser ps( mVM, mText );	

	if( ps.mErr.size() )
	{
		mCompiled = false;
		
		vector<char> buf;
		ConvertToUTF8( ps.mErr, buf );
		sqlite3_result_error( ctx, &buf[ 0 ], buf.size() - 1 );
	}
	else
	{
		mCompiled = true;
	}
}

/**********************************************************************************************/
void Function::Destroy( void* ptr )
{
	auto it = gFunctions.begin();
	for( ; it != gFunctions.end() ; ++it )
	{
		if( it->second == ptr )
		{
			delete (Function*) ptr;
			break;
		}
	}	
}

/**********************************************************************************************/
void Function::Execute(
	sqlite3_context*	ctx,
	int					argc,
	sqlite3_value**		argv )
{
	// Prepare Virtual Machine
	if( !mCompiled )
	{
		Compile( ctx );
		if( !mCompiled )
			return;
	}

	// Execute function	
	if( mVM.mBusy )
	{
		VMachine vm( mVM );
		vm.Run( ctx, argc, argv );

		// Returns output
		if( vm.mOutput.size() )
			sqlite3_result_text( ctx, &vm.mOutput[ 0 ], vm.mOutput.size(), SQLITE_TRANSIENT );
	}
	else
	{
		mVM.Run( ctx, argc, argv );

		// Returns output
		if( mVM.mOutput.size() )
		{
			sqlite3_result_text( ctx, &mVM.mOutput[ 0 ], mVM.mOutput.size(), SQLITE_TRANSIENT );
			mVM.mOutput.clear();
		}
	}
}

/**********************************************************************************************/
void Function::Explain( sqlite3_context* ctx )
{
	// Prepare Virtual Machine
	if( !mCompiled )
		Compile( ctx );
		
	if( !mCompiled )
	{
		RESULT_TEXT( "Function contain errors" );
		return;
	}
	
	wstring res;

	size_t count = mVM.mCommands.size();
	for( size_t i = 0 ; i < count ; ++i )
	{
		if( res.size() )
			res += '\n';
		
		wstring label = IntToStr( i );
		while( label.size() < 4 ) label = L" " + label;
		label += L": ";
					
		res += label;
		res += mVM.mCommands[ i ]->Explain();
	}
	
	RESULT_WTEXT( res );
}

/**********************************************************************************************/
Function* Function::FindFunction(		
	sqlite3*		db,
	const wstring&	name )
{
	auto it = gFunctions.find( FuncKey( db, Lower( name ) ) );
	return it != gFunctions.end() ?
		it->second :
		NULL;
}


/**********************************************************************************************/
SP_NAMESPACE_END
