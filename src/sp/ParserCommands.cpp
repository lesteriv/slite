/**********************************************************************************************/		
/* ParserCommands.cpp																		  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Parser.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// commands methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::ReadCreate( void )
{
	SaveAndGo();
	ToLower( mToken );
	
	if( mToken == L"or" )
	{
		SkipToken( L"replace" );
		NextToken();
		ToLower( mToken );
	}
	
	if( mToken == L"procedure" )
	{
		ReadProcedure();
	}
	else if( mToken == L"function" )
	{
		ReadFunction();
	}
	else
	{
		Restore();
		ReadQuery();
	}
}

/**********************************************************************************************/
void Parser::ReadClose( void )
{
	CHECK_BLOCK

	NextToken();
	wstring name = ReadIdent( false );
	
	Cursor* cursor = FindCursor( name );
	if( !cursor )
		BREAK( L"Unable to find cursor '" + name + L"'" );

	COMMAND( kCmdClose )->mObject = mVM.Index( cursor );
	SkipToken( L";" );
}

/**********************************************************************************************/
void Parser::ReadCursor( void )
{
	NextToken();
	wstring name = ReadIdent( false );

	CHECK_DUPLICATE( FindVariable( name, true ) )
	CHECK_RESERVED( name );
		
	SkipToken( L"is" );
	wstring expr = ReadStatement();

	Cursor* cr = FindCursor( name, true );
	CHECK_DUPLICATE( cr );
		
	AddCursor( name, expr );
}

/**********************************************************************************************/
void Parser::ReadExit( void )
{
	CHECK_BLOCK

	if( mLoops.empty() )
		BREAK( L"Unexpected EXIT" );

	NextToken();
	Command* cmd = NULL;
	
	// LABEL
	ToLower( mToken );
	if( mToken != L"when" && mToken != L";" )
	{
		wstring label = ReadIdent();
		
		int count = mLoops.size();
		for( int i = count - 1 ; i >= 0 ; --i )
		{
			if( mLoops[ i ].mLabel == label )
			{
				cmd = COMMAND( kCmdGo );
				mLoops[ i ].mEnds.push_back( cmd );
				break;
			}
		}
		
		if( !cmd )
			BREAK( L"Unable to find loop with label '" + label + L"'" );
		
		NextToken();
		ToLower( mToken );
	}
	
	// EXIT FROM 'NEAREST" LOOP
	if( !cmd )
	{
		cmd = COMMAND( kCmdGo );
		mLoops.back().mEnds.push_back( cmd );
	}
	
	// EXIT WHEN
	if( mToken == L"when" )
	{
		cmd->mCmdID = kCmdGoIf;
		cmd->mpExpr = EXPRESSION( ReadUntil( L";" ) );
	}
}

/**********************************************************************************************/
void Parser::ReadFetch( void )
{
	CHECK_BLOCK

	NextToken();
	wstring crname = ReadIdent( true );
		
	Cursor* cr = FindCursor( crname );
	if( !cr )
		BREAK( L"Unable to find cursor '" + crname + L"'" );

	// INTO
	SkipToken( L"into" );

	// COLUMNS
	vector<int> vars;
	
	SaveAndGo();
	wstring base = ReadIdent( true );
	
	const Type* rec = FindRecord( base );
	if( rec )
	{
		size_t count = rec->mMembers.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			int idx = mVM.Index( FindVariable( base + L'.' + rec->mMembers[ i ].mName ) );
			if( idx < 0 )
				BREAK( L"Unable to access record's member" );

			vars.push_back( idx );
		}
		
		NextToken();
	}
	else
	{
		Restore();
		
		vector<wstring> list = ReadList();
		size_t count = list.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			int idx = mVM.Index( FindVariable( list[ i ] ) );
			if( idx < 0 )
				BREAK( L"Unknown variable '" + list[ i ] + L"'" );

			vars.push_back( idx );
		}
		
		NextToken();
	}

	if( mToken != L";" )
		BREAK( L"';' expected" );

	Command* cmd = COMMAND( kCmdFetch );
	cmd->mObject = mVM.Index( cr );
	cmd->mFields.swap( vars );
}

/**********************************************************************************************/
void Parser::ReadFor( void )
{
	CHECK_BLOCK
	
	wstring label = mLabel;
	ReadBegin();	

	// READ PARAMETERS

	// Variable name
	NextToken();
	wstring vname = ReadIdent( false );

	// IN
	SkipToken( L"in" );

	bool reverse = false;

	SaveAndGo();

	EToken tk = GET_TOKEN;
	if( tk == kTokenReverse )
		reverse = true;
	else
		Restore();

	wstring stmt = ReadUntil( kTokenLoop );
	
	// RANGE LOOP
	int dpos = stmt.find( L".." );
	if( dpos > 0 )
	{
		// LOOP VARIABLE
		int var = AddVariable( vname, kInt );
		mVM.mVariables[ var ]->mReadOnly = true;
	
		// FROM
		wstring from = stmt.substr( 0, dpos );
		wstring to	 = stmt.substr( dpos + 2 );

		// START VALUE
		Command* cmd = COMMAND( kCmdAssign );
		cmd->mpExpr = EXPRESSION( reverse ? to + L"+1" : from + L"-1" );
		cmd->mObject = var;

		// START OF LOOP
		COMMAND( kCmdNope );
		mLoops.push_back( Loop( kLoopFor, mVM.LastCommandIndex(), label ) );

		// INCREMENT VALUE
		cmd = COMMAND( kCmdAssign );
		cmd->mpExpr = EXPRESSION( vname + ( reverse ? L"-1" : L"+1" ) );
		cmd->mObject = var;

		// CHECK FOR END OF LOOP
		cmd = COMMAND( kCmdGoIf );
		cmd->mpExpr = EXPRESSION( vname + ( reverse ? L"<" + from : L">" + to ) );
		mLoops.back().mEnds.push_back( cmd );
	}
	// CURSOR LOOP
	else
	{
		TrimAll( stmt );
		size_t slen = stmt.length();
		
		wstring ident = stmt;
		if( slen > 2 )
		{
			wchar_t cb = stmt[ 0 ];
			wchar_t ce = stmt[ slen - 1 ];
			
			if( ( cb == '"' && ce == '"' ) ||
				( cb == '`' && ce == '`' ) ||
				( cb == '[' && ce == ']' ) )
				ident = stmt.substr( 1, slen - 2 );
		}
		
		int cidx;
		Cursor* cr = FindCursor( ident );
		if( !cr )
		{
			if( slen > 2 && stmt[ 0 ] == '(' && stmt[ slen - 1 ] == ')' )
				stmt = stmt.substr( 1, slen - 2 );
			
			static int tindex = 0;
			ident = L"_sp_tmp_cursor_" + IntToStr( tindex++ );
			cidx = AddCursor( ident, stmt );
			cr = mVM.mCursors[ cidx ];
		}
		else
		{
			cidx = mVM.Index( cr );
		}
		
		// OPEN CURSOR
		COMMAND( kCmdOpen )->mObject = cidx;

		// START OF LOOP
		COMMAND( kCmdNope );
		mLoops.push_back( Loop( kLoopFor, mVM.LastCommandIndex(), label ) );

		// FETCH RECORD
		Command* cmd = COMMAND( kCmdFetch );
		cmd->mObject = cidx;
		cmd->mFields = AddRecord( vname, cr->get_RowType() );

		// CHECK FOR END OF LOOP
		cmd = COMMAND( kCmdGoIf );
		cmd->mpExpr = EXPRESSION( ident + L"%notfound" );
		mLoops.back().mEnds.push_back( cmd );		
	}
}

/**********************************************************************************************/
void Parser::ReadFunction( void )
{
	NextToken();
	ReadIdent( false );
	
	StartDeclare();
	
	NextToken();
	EToken tk = GET_TOKEN;
	if( tk == kTokenBraceLeft )
	{
		ReadParameters();	
		SkipToken( L"return" );
	}
	else if( tk != kTokenReturn )
	{
		INVALID_TOKEN;
	}

	NextToken();
	mVM.mType = ReadType().mType;

	NextToken();
	ToLower( mToken );
	if( mToken != L"as" && mToken != L"is" )
		INVALID_TOKEN;
}

/**********************************************************************************************/
void Parser::ReadLoop( void )
{
	CHECK_BLOCK

	COMMAND( kCmdNope );
	mLoops.push_back( Loop( kLoop, mVM.LastCommandIndex(), mLabel ) );
}

/**********************************************************************************************/
void Parser::ReadOpen( void )
{
	CHECK_BLOCK

	NextToken();
	wstring crname = ReadIdent( true );
	
	Cursor* cr = FindCursor( crname );
	if( !cr )
		BREAK( L"Unable to find cursor '" + crname + L"'" );

	COMMAND( kCmdOpen )->
		mObject = mVM.Index( cr );

	SkipToken( L";" );
}

/**********************************************************************************************/
void Parser::ReadProcedure( void )
{
	NextToken();
	ReadIdent( false );
	
	StartDeclare();
	
	NextToken();
	EToken tk = GET_TOKEN;
	if( tk == kTokenBraceLeft )
	{
		ReadParameters();
		NextToken();
	}

	ToLower( mToken );
	if( mToken != L"as" && mToken != L"is" )
		INVALID_TOKEN;	
}

/**********************************************************************************************/
void Parser::ReadWhile( void )
{
	CHECK_BLOCK

	// START LOOP
	COMMAND( kCmdNope );
	mLoops.push_back( Loop( kLoopWhile, mVM.LastCommandIndex(), mLabel ) );

	// ADD 'EXIT'
	Command* cmd = COMMAND( kCmdGoIfNot );
	cmd->mpExpr = EXPRESSION( ReadUntil( kTokenLoop ) );
	mLoops.back().mEnds.push_back( cmd );
}


/**********************************************************************************************/
SP_NAMESPACE_END
