/**********************************************************************************************/		
/* VMachine.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Function.h"
#include "Unicode.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructors
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
VMachine::VMachine( sqlite3* db )
{
	// Cached data
	mType			= kUnknown;
	mVarFound		= 0;
	mVarNotFound	= 0;
	mVarRowCount	= 0;
	
	// References
	mpDB			= db;
	
	// Internal data
	mBusy			= false;
}

/**********************************************************************************************/
VMachine::VMachine( const VMachine& vm )
{
	// Cached data
	mArgc			= vm.mArgc;
	mArgcMin		= vm.mArgcMin;
	mType			= kUnknown;
	mVarFound		= vm.mVarFound;
	mVarNotFound	= vm.mVarNotFound;
	mVarRowCount	= vm.mVarRowCount;
	
	// References
	mpDB			= vm.mpDB;

	// Exceptions
	mCatches		= vm.mCatches;

	// Variables
	size_t count = vm.mVariables.size();
	for( size_t i = 0 ; i < count ; ++i )
		mVariables.push_back( new Variable( *vm.mVariables[ i ] ) );
	
	// Commands
	count = vm.mCommands.size();
	for( size_t i = 0 ; i < count ; ++i )
		mCommands.push_back( vm.mCommands[ i ]->Clone( this ) );

	// Cursors
	count = vm.mCursors.size();
	for( size_t i = 0 ; i < count ; ++i )
		mCursors.push_back( vm.mCursors[ i ]->Clone( this ) );
	
	// Internal data
	mBusy = false;
}

/**********************************************************************************************/
VMachine::~VMachine( void )
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Command* VMachine::AddCommand(
	ECommand	id,
	int			catchIndex )
{
	Command* cmd = new Command( this, id, catchIndex );
	mCommands.push_back( cmd );
	
	return cmd;
}

/**********************************************************************************************/
void VMachine::Catch( int index )
{
	// TODO:

	mPos = mCatches[ index ];
}

/**********************************************************************************************/
const wstring& VMachine::GetCursorName( Cursor* cursor ) const
{
	auto it = mCursorsMap.begin();
	while( it != mCursorsMap.end() )
	{
		if( it->second == cursor )
			return it->first;

		++it;
	}

	static wstring res;
	return res;
}

/**********************************************************************************************/
const wstring& VMachine::GetVariableName( Variable* var ) const
{
	auto it = mVariablesNames.find( var );
	if( it != mVariablesNames.end() )
		return it->second;

	static wstring res;
	return res;
}

/**********************************************************************************************/
int VMachine::Index( Cursor* vl ) const
{
	vector<Cursor*>::const_iterator it = find( mCursors.begin(), mCursors.end(), vl );
	return it == mCursors.end() ?
		-1 :
		it - mCursors.begin();
}

/**********************************************************************************************/
int VMachine::Index( Variable* vl ) const
{
	vector<Variable*>::const_iterator it = find( mVariables.begin(), mVariables.end(), vl );
	return it == mVariables.end() ?
		-1 :
		it - mVariables.begin();
}

/**********************************************************************************************/
void VMachine::Print( const char* str )
{
	if( mOutput.size() )
		mOutput.push_back( '\n' );
	
	size_t len = strlen( str );
	size_t pos = mOutput.size();
	mOutput.resize( pos + len );
	memcpy( &mOutput[ pos ], str, len );
}

/**********************************************************************************************/
void VMachine::PrintError( const char* msg )
{
	if( !msg )
		msg = sqlite3_errmsg( mpDB );
	
	sqlite3_result_error( mpContext, msg, strlen( msg ) );
	mOutput.clear();
}

/**********************************************************************************************/
void VMachine::PrintW( const wstring& line )
{
	if( mOutput.size() )
		mOutput.push_back( '\n' );

	vector<char> buf;
	ConvertToUTF8( line, buf );
	
	size_t len = buf.size() - 1;
	size_t pos = mOutput.size();
	mOutput.resize( pos + len );
	memcpy( &mOutput[ pos ], &buf[ 0 ], len );
}
		
/**********************************************************************************************/
void VMachine::Run(
	sqlite3_context*	ctx,
	int					argc,
	sqlite3_value**		argv )
{
	mArgcRun	= argc;
	mpArgv		= argv;
	mBusy		= true;
	mpContext	= ctx;
	mPos		= 0;
	mSteps		= 0;

	if( argc < mArgcMin || argc > mArgc )
	{
		PrintError( "Invalid count of arguments" );
		return;
	}
	
	while( Step() );
	
	mBusy = false;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void VMachine::Clear( void )
{
	CLEAR( mCommands );
	CLEAR( mCursors );
	CLEAR( mVariables );
	
	mCursorsMap.clear();
	mCatches.clear();
	mVariablesNames.clear();
}

/**********************************************************************************************/
bool VMachine::Step( void )
{
	if( mPos >= (int) mCommands.size() || ++mSteps > 99999 )
		return false;

	int	 prev	= mPos;
	bool res	= mCommands[ mPos ]->Execute();
	
	// Command doesn't touch position - so increment it
	if( mPos == prev )
		++mPos;
		
	return res;
}


/**********************************************************************************************/
SP_NAMESPACE_END
