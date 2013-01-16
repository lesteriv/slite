/**********************************************************************************************/		
/* ParserBlocks.cpp																			  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Parser.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// blocks methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::ReadBegin( void )
{
	// Pop declare section - to disable add any declarations
	if( GetSection() == kSectionDeclare )
	{
		mSections.pop_back();
	}
	else
	{
		mBlocks.push_back( Block() );
		mBlocks.back().mLabel.swap( mLabel );
		mBlocks.back().mStart = mVM.mCommands.size();
	}

	// Add special variables to first block
	if( mBlocks.size() == 1 )
	{
		mVM.mVarFound		= AddVariable( L"sql%found"		, kInt );
		mVM.mVarNotFound	= AddVariable( L"sql%notfound"	, kInt );
		mVM.mVarRowCount	= AddVariable( L"sql%rowcount"	, kInt );
	}
	
	// Each BEGIN/END block can has own exception handler
	++mException;
	
	mSections.push_back( kSectionBegin );
}

/**********************************************************************************************/
void Parser::ProcessEnd( bool label )
{
	ESection block = GetSection();
	mSections.pop_back();
	
	if( block == kSectionException && mSections.size() )
		mSections.pop_back(); // Pop also begin
		
	if( mSections.empty() )
		mEndReached = true;

	if( mBlocks.size() )
	{
		Block& blk = mBlocks.back();
		
		if( label )
		{
			ToLower( mToken );
			if( mToken != blk.mLabel )
				BREAK( L"Missmatch label for block" );
		}
		
		// Close cursors
		CursorMap& list = blk.mCursors;
		CursorMap::iterator it = list.begin();
		for( ; it != list.end() ; ++it )
			COMMAND( kCmdClose )->mObject = mVM.Index( it->second );

		// Apply labels
		vector<Command*>& cmds = mVM.mCommands;
		size_t size = cmds.size();
		for( size_t i = blk.mStart ; i < size ; ++i )
		{
			Command* cmd = cmds[ i ];

			if( cmd->mCmdID == kCmdGo && cmd->mParam.mType == kText )
			{
				const wstring& label = cmds[ i ]->mParam.ToString();

				auto it = blk.mLabels.find( label );
				if( it != blk.mLabels.end() )
					cmd->mParam = (int) it->second;
			}
		}
		
		mBlocks.pop_back();
	}
	
	if( block == kSectionException )
		PopNextCase();
}

/**********************************************************************************************/
void Parser::ReadException( void )
{
	// TODO: mVM.mExceptionHandlers

	ESection block = GetSection();
	if( block != kSectionBegin )
		BREAK( L"Unexpected EXCEPTION token" );	
		
	mSections.push_back( kSectionException );
	
	// Skip exception handler
	PushNextCase( COMMAND( kCmdGo ) );
}

/**********************************************************************************************/
void Parser::StartDeclare( void )
{
	mBlocks.push_back( Block() );
	mSections.push_back( kSectionDeclare );

	mBlocks.back().mLabel.swap( mLabel );
	mBlocks.back().mStart = mVM.mCommands.size();
}


/**********************************************************************************************/
SP_NAMESPACE_END
