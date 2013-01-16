/**********************************************************************************************/		
/* Parser.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Cursor.h"
#include "Function.h"
#include "GlobalVariables.h"
#include "Parser.h"
#include "Statement.h"
#include "Utils.h"
#include "Value.h"
#include "Variable.h"
#include "VMachine.h"
#include "Type.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Parser::Parser(
	VMachine&		out,
	const wstring&	text )
:
// Source data
	mText( text ),
	
// References	
	mVM( out )
{
	out.Clear();
	
	mVM.mArgc = 0;
	mVM.mArgcMin = 0;

	if( !setjmp( mJBuf ) )
		Parse();
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::Parse( void )
{
	mEnd		= mText.length();
	mEndReached	= false;
	mException	= -1;
 	mLine		= 1;
	mPos		= 0;

	NextToken();
	while( mToken.length() )
	{
		ProcessToken();
		NextToken();
	}
	
	if( mSections.size() > 1 ||	( mSections.size() == 1 && mSections[ 0 ] != kSectionDeclare ) )
		BREAK( L"Unexpected end of function" );
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::ProcessGeneralToken( void )
{
	switch( GetSection() )
	{
		case kSectionDeclare:
		{
			switch( GET_TOKEN )
			{
				case kTokenCursor	: ReadCursor(); break;
				case kTokenType		: ReadUserType(); break;
				case kTokenSubType	: ReadSubType(); break;
				
				default:
					ReadVariables();
			}
		}
		break;

		default:
		{
			wstring ident = ReadIdent( true );
			
			// Check for records first
			
			if( mPos >= mEnd || mText[ mPos ] != '.' )
			{
				const Type* ltype = FindRecord( ident );
				if( ltype )
				{
					SkipToken( L":" );
					SkipChar( '=' );

					wstring rrec = ReadNextIdent( true );
					const Type* rtype = FindRecord( rrec );
					if( !rtype )
						BREAK( L"Record expected but '" + rrec + L"' found" );

					SkipToken( L";" );

					size_t count = ltype->mMembers.size();
					for( size_t i = 0 ; i < count ; ++i )
					{
						wstring lname = ident + L'.' + ltype->mMembers[ i ].mName;
						wstring rname = rrec + L'.' + ltype->mMembers[ i ].mName;

						Variable* lvl = FindVariable( lname );
						if( !lvl )
							BREAK( L"Unable to access record's member" );

						Command* cmd = COMMAND( kCmdAssign );
						cmd->mpExpr  = EXPRESSION( rname );
						cmd->mObject = mVM.Index( lvl );
					}

					break;
				}
			}
			
			// Check for variables
			
			Variable* var = FindVariable( ident );
			if( !var )
				var = FindGlobalVariable( ident );
			
			if( !var )
			{
				Function* func = Function::FindFunction( mVM.mpDB, ident );
				if( func )
				{
					mPos -= ident.length();
					ReadExec();
					break;
				}
				else
				{
					INVALID_TOKEN
				}
			}
				
			SkipToken( L":" );
			SkipChar( '=' );
			
			if( var->mReadOnly )
				BREAK( L"Unable assign new value to constant" );
			
			wstring expr = ReadExpression();
			SkipToken( L";" );
				
			Command* cmd = COMMAND( kCmdAssign );
			cmd->mpExpr  = EXPRESSION( expr );
			cmd->mObject = mVM.Index( var );
		}
		break;
	}
}

/**********************************************************************************************/
void Parser::ProcessToken( void )
{
	// 'Top level' tokens

	EToken tk = GET_TOKEN;
	if( mEndReached && tk != kTokenSeparator && tk != kTokenSemicolon )
		BREAK( L"Found '" + mToken + L"' after the last END" );
	
	if( tk >= kTokenAlter && tk <= kTokenVacuum )
	{
		ReadQuery();
		return;
	}
	
	switch( tk )
	{
		case kTokenCase			: ReadCase(); break;
		case kTokenClose		: ReadClose(); break;
		case kTokenCreate		: ReadCreate(); break;
		case kTokenDeclare		: StartDeclare(); break;
		case kTokenElse			: ReadElse(); break;
		case kTokenElsIf		: ReadElsIf(); break;
		case kTokenException	: ReadException(); break;
		case kTokenExec			: ReadExec(); break;
		case kTokenExecute		: ReadExecute(); break;
		case kTokenExit			: ReadExit(); break;
		case kTokenFetch		: ReadFetch(); break;
		case kTokenFor			: ReadFor(); break;
		case kTokenGoto			: ReadGoto(); break;
		case kTokenIf			: ReadIf(); break;
		case kTokenLoop			: ReadLoop(); break;
		case kTokenNull			: ReadNull(); break;
		case kTokenOpen			: ReadOpen(); break;
		case kTokenOutput		: ReadOutput(); break;
		case kTokenPrint		: ReadPrint(); break;
		case kTokenReturn		: ReadReturn(); break;
		case kTokenSelect		: ReadSelect(); break;
		case kTokenSet			: ReadSet(); break;
		case kTokenWhen			: ReadWhen(); break;
		case kTokenWhile		: ReadWhile(); break;

		case kTokenLess			: ReadLabel(); break;
		
		case kTokenSemicolon	:
		case kTokenSeparator	: /* TODO: we cannot be here */ break;
		
		case kTokenBegin		:
		{
			SaveAndGo();
			
			if( Lower( mToken ) == L"transaction" )
			{
				Restore();
				ReadQuery();
			}
			else
			{
				Restore();
				ReadBegin();
			}
		}
		break;
		
		case kTokenEnd			:
		{
			SaveAndGo();
			
			if( Lower( mToken ) == L"transaction" )
			{
				Restore();
				ReadQuery();
			}
			else
			{
				Restore();
				ReadEnd();
			}
		}
		break;
		
		default:
			ProcessGeneralToken();
			break;
	}
	
	mLabel.swap( mLabelNext );
	mLabelNext.clear();
}

/**********************************************************************************************/
void Parser::ReadCase( void )
{
	CHECK_BLOCK
	
	wstring expr = ReadUntil( kTokenWhen, true );
		
	PushSwitch();				// Add forward point to go to after executing block if/elsif
								// It will be defined on END IF

	PushCase( L"0" );			// "Dummy" expression
	PushCaseExpr( expr );		// Remember selector for case block
	
	// Read first WHEN
	
	ReadWhen();
}

/**********************************************************************************************/
void Parser::ReadElse( void )
{
	// Add GO TO to skip 'else' block
	Command* cmd = COMMAND( kCmdGo );

	// IF FALSE at now point here
	PopNextCase();

	// Add GO TO to stack
	PushNextCase( cmd );
}

/**********************************************************************************************/
void Parser::ReadElsIf( void )
{
	CHECK_BLOCK
	
	wstring expr = ReadUntil( kTokenThen );

	// Finish last block - add GO TO to the end of IF...END IF construction
	size_t size = mSwitches.size();
	if( !size )
		BREAK( L"Unexpected ELSIF" );
		
	mSwitches[ size - 1 ].push_back( COMMAND( kCmdGo ) );

	// Here at now points previous GO TO IF NOT
	PopNextCase();
	
	// Add new block to IF...END IF
	PushCase( expr );
}

/**********************************************************************************************/
void Parser::ReadEnd( void )
{
	ESection block = GetSection();
	if( block != kSectionBegin && block != kSectionException )
		BREAK( L"Unexpected END token" );

	NextToken();
	
	// End of block
	if( mToken == L";" )
	{
		ProcessEnd();
		return;
	}

	EToken token = GET_TOKEN;
	switch( token )
	{
		case kTokenCase:
		{
			PopNextCase();
			PopSwitch();
			mCases.pop_back();

			SkipToken( L";" );
		}
		break;

		case kTokenIf:
		{
			PopNextCase();
			PopSwitch();

			SkipToken( L";" );
		}
		break;

		case kTokenLoop:
		{
			size_t count = mLoops.size();
			if( !count )
				BREAK( L"Unexpected END LOOP!" );

			// JUMP TO START
			
			COMMAND( kCmdGo )->mParam = mLoops.back().mStart;
			
			if( mLoops.back().mKind == kLoopFor )
				ProcessEnd();
			
			// JUMPS TO EXIT FROM LOOP
			
			// Add dummy command - to be sure all be ok
			COMMAND( kCmdNope );

			vector<Command*>& list = mLoops.back().mEnds;
			if( list.empty() )
				BREAK( L"Unexpected END" );

			count = list.size();
			for( size_t i = 0 ; i < count ; ++i )
				list[ i ]->mParam = mVM.LastCommandIndex();

			// Add IF, CASE, LOOP to mBlocks to check order of ENDs

			NextToken();
			if( mToken != L";" )
			{
				wstring label = ReadIdent();
				if( label != mLoops.back().mLabel )
					BREAK( L"Mismatch label for loop" );
				
				SkipToken( L";" );
			}

			mLoops.pop_back();
		}
		break;

		default:
		{
			ProcessEnd( true );
			NextToken();
		}
		break;
	}
}

/**********************************************************************************************/
void Parser::ReadExec( void )
{
	CHECK_BLOCK
	
	wstring name = ReadStatement();
		
	size_t size = name.length();
	if( size && name[ size - 1 ] != ')' )
		name += L"()";
	
	COMMAND( kCmdExec )->mpStmt = new Statement( *this, L"SELECT " + name );
}

/**********************************************************************************************/
void Parser::ReadExecute( void )
{
	CHECK_BLOCK

	SkipToken( L"immediate" );
	COMMAND( kCmdExec )->mpExpr = EXPRESSION( ReadUntil( kTokenSemicolon ) );
}

/**********************************************************************************************/
void Parser::ReadGoto( void )
{
	CHECK_BLOCK

	COMMAND( kCmdGo )->mParam = ReadNextIdent();
	SkipToken( L";" );	
}

/**********************************************************************************************/
void Parser::ReadIf( void )
{
	CHECK_BLOCK
	
	wstring expr = ReadUntil( kTokenThen );
		
	PushSwitch();		// Add forward point to go to after executing block if/elsif
						// It will be defined on END IF

	PushCase( expr );	// Implement one 'node' of IF...END_IF construction
}

/**********************************************************************************************/
void Parser::ReadLabel( void )
{
	SkipChar( '<' );
	COMMAND( kCmdNope );

	wstring label = Lower( ReadUntil( L">", false ) );

	if( mBlocks.size() )
	{
		auto& labels = mBlocks.back().mLabels;
			
		auto it = labels.find( label );
		if( it != labels.end() )
			BREAK( L"Duplicate label '" + label + L"'" );
		
		labels[ label ] = mVM.LastCommandIndex();
	}
	
	mLabelNext = label;
	SkipChar( '>' );
}

/**********************************************************************************************/
void Parser::ReadNull( void )
{
	CHECK_BLOCK
	SkipToken( L";" );
}

/**********************************************************************************************/
void Parser::ReadOutput( void )
{
	CHECK_BLOCK
	
	NextToken();
	if( mToken != L"." )
		BREAK( L"Expected '.' after 'dbms_output'" );
		
	NextToken();
	ToLower( mToken );
	
	if( mToken == L"enable" )
	{
		ReadUntil( L";", true );
	
		// COMMAND( kCmdPrintOnOff )->cmd->put_Parameter( true );		
	}
	else if( mToken == L"disable" )
	{
		// COMMAND( kCmdPrintOnOff )->put_Parameter( false );
	}
	else if( mToken == L"new_line" )
	{
		COMMAND( kCmdPrint )->mpExpr = EXPRESSION( L"" );
	}
	else if( mToken == L"put_line" )
	{		
		SkipToken( L"(" );
		wstring expr = ReadExpression();
		SkipToken( L")" );
	
		COMMAND( kCmdPrint )->mpExpr = EXPRESSION( expr );
	}
	else
	{
		BREAK( L"Non supported function - " + mToken );
	}
}

/**********************************************************************************************/
void Parser::ReadPrint( void )
{
	CHECK_BLOCK
	COMMAND( kCmdPrint )->mpExpr = EXPRESSION( ReadExpression() );
}

/**********************************************************************************************/
void Parser::ReadReturn( void )
{
	CHECK_BLOCK
		
	SaveAndGo();
	if( mToken == L";" )
	{
		COMMAND( kCmdReturn );
	}
	else
	{
		Restore();
		COMMAND( kCmdReturn )->mpExpr = EXPRESSION( ReadExpression() );
	}
}

/**********************************************************************************************/
void Parser::ReadQuery( void )
{
	ESection block = GetSection();
	if( block != kSectionBegin && block != kSectionException && block != kSectionUnknown )
		BREAK( L"Unexpected " + mToken + L" token" )	
	
	wstring stmt = mToken;
	stmt.push_back( ' ' );
	stmt += ReadUntil( kTokenSemicolon, true );
		
	COMMAND( kCmdExec )->mpStmt = new Statement( *this, stmt );
}

/**********************************************************************************************/
void Parser::ReadSelect( void )
{
	CHECK_BLOCK

	size_t start = mPos;
	
	do
	{
		NextToken();
		ToLower( mToken );
		
		if( mToken == L"into" )
			break;
		
		// Simple SELECT statement
		if( mToken == L";" )
		{
			COMMAND( kCmdSelect )->mParam = mText.substr( start, mPos - start );
			return;
		}
	}
	while( mToken.length() );
	
	// TODO:
	
	wstring expr = mText.substr( start, mPos - start - 4 );
		
	SaveAndGo();

	wstring ident = ReadIdent( true );
	Command* cmd = COMMAND( kCmdAssign );
	vector<int> idxs;
	
	const Type* rd = FindRecord( ident );
	if( rd )
	{
		wstring base = ident + L'.';
		size_t count = rd->mMembers.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			Variable* var = FindVariable( base + rd->mMembers[ i ].mName );
			
			int index = mVM.Index( var );
			if( index < 0 )
				BREAK( L"Unable to find member of " + ident );
			
			idxs.push_back( index );
		}
	}
	else
	{
		Restore();

		vector<wstring> vars = ReadList();
		size_t count = vars.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			Variable* var = FindVariable( vars[ i ] );
			
			int index = mVM.Index( var );
			if( index < 0 )
				BREAK( L"Unable to find variable " + vars[ i ] );

			idxs.push_back( index );
		}
	}

	cmd->mFields = idxs;
	cmd->mpExpr = EXPRESSION( expr + ReadUntil( kTokenSemicolon ) );
}

/**********************************************************************************************/
void Parser::ReadSet( void )
{
	NextToken();

	EToken token = GET_TOKEN;
	if( token != kTokenServerOutput )
		BREAK( L"At now only 'serveroutput' is supported" );
		
	NextToken();
	token = GET_TOKEN;
	if( token != kTokenOn && token != kTokenOff )
		BREAK( L"'on' or 'off' expected" );
		
	// Skip other arguments	
	SkipLine();	
		
//	COMMAND( kCmdPrintOnOff )->put_Parameter( token == kTokenOn );
}

/**********************************************************************************************/
const Type Parser::ReadType( wstring* def )
{
	Type res;
	res.mNotNULL = false;
	
	wstring tname = ReadIdent();

	SaveAndGo( "%" );

	// Reference type to field
	if( mToken == L"." )
	{
		NextToken( "%" );
		wstring fname = ReadIdent();
	
		res.mType = GetColumnType( mVM.mpDB, tname, fname );
		
		NextToken( "%" );
		if( mToken != L"%" )
			INVALID_TOKEN
			
		NextToken();
		EToken token = GET_TOKEN;
		if( token != kTokenType )
			BREAK( L"'TYPE' expected" );
		
		if( res.mType == kUnknown )
			BREAK( L"Cannot get type of field" );
	
		return res;
	}
	
	// Reference type to varible or rowtype
	if( mToken == L"%" )
	{
		NextToken();
		EToken token = GET_TOKEN;
		
		// RowType
		if( token == kTokenRowType )
		{
			ReadTableStructure( res, mVM.mpDB, L"SELECT * FROM \"" + tname + L"\" LIMIT 1" );
			if( res.mMembers.empty() )
			{
				Cursor*	cursor = FindCursor( tname );
				if( !cursor )
					BREAK( L"Table '" + tname + L"' not found" );
				
				res = cursor->get_RowType();
			}
			
			return res;
		}

		// Type of variable
		if( token != kTokenType )
			BREAK( L"'TYPE' expected" );
	
		Variable* var = FindVariable( tname );
		if( var )
		{
			res.mType = var->mValue.mType;
			res.mNotNULL = var->mNotNULL;
			
			if( def )
			{
				auto it = mDefaults.find( var );
				if( it != mDefaults.end() )
					*def = it->second;
			}
		}
		else
		{
			const Type* rec = FindRecord( tname );
			if( rec )
				res = *rec;
			else
				BREAK( L"Variable '" + tname + L"' not found" );
		}
		
		return res;
	}

	Restore();
	
	// Subtype
	const Type* type = FindSubType( tname );
	if( type )
		return *type;

	// Usual type
	CHECK_TYPE
	res.mType = ConvertTypeNameToEnum( mToken );

	SaveAndGo();
	
	// Skip unsupported( by SQLite ) part of type	
	if( mToken == L"(" )
		ReadUntil( kTokenBraceRight );
	else
		Restore();
	
	return res;
}

/**********************************************************************************************/
void Parser::ReadSubType( void )
{
	wstring name = ReadNextIdent();
	CHECK_DUPLICATE( FindSubType( name, true ) );
	
	SkipToken( L"is" );
	NextToken();
	
	Type type = ReadType();
	SaveAndGo();
	
	EToken tk = GET_TOKEN;
	if( tk == kTokenNot )
	{
		type.mNotNULL = true;
		SkipToken( L"null" );
		NextToken();
	}
	else
	{
		type.mNotNULL = false;
		Restore();
	}
	
	AddSubType( name, type );
}

/**********************************************************************************************/
void Parser::ReadUserType( void )
{
	wstring name = ReadNextIdent();

	CHECK_RESERVED( name );
	CHECK_DUPLICATE( FindSubType( name, true ) );
	SkipToken( L"is" );
	
	// TODO:
	SkipToken( L"record" );
	SkipToken( L"(" );
	Type type = ReadRecord();
	
	if( type.mMembers.empty() )
		BREAK( L"Empty record" );
	
	AddSubType( name, type );
}

/**********************************************************************************************/
void Parser::ReadVariables( void )
{
	ReadVariablesNames();

	Block& bl = mBlocks.back();
	vector<Variable*>& vars = mVM.mVariables;
	size_t cold = vars.size();

	// CONSTANT
	bool ro = ( Lower( mToken ) == L"constant" );
	if( ro )
		NextToken( "%" );
	
	// TYPE
	
	wstring def;
	Type type = ReadType( &def );
	NextToken();
	
	// CREATE VARIABLES
	
	// RowType
	size_t fcount = type.mMembers.size();
	if( fcount )
	{
		size_t count = mVarNames.size();
		for( size_t i = 0 ; i < count ; ++i )
			AddRecord( mVarNames[ i ], type, ro );
	}
	// 'Simple' variables
	else
	{
		size_t count = mVarNames.size();
		for( size_t i = 0 ; i < count ; ++i )
		{
			Variable* var = new Variable;
			var->SetNotNULL( type.mNotNULL );
			var->mReadOnly = ro;
			var->SetType( type.mType );
			vars.push_back( var );
			
			bl.mVarsMap[ mVarNames[ i ] ] = var;
			mVM.mVariablesNames[ var ] = mVarNames[ i ];
			
			if( bl.mLabel.length() )
				bl.mVarsMap[ bl.mLabel + L"." + mVarNames[ i ] ] = var;			
		}
	}

	size_t count = vars.size();
	EToken tk = GET_TOKEN;
	
	// End of declarations
	if( tk == kTokenSemicolon )
		goto end;
	
	if( tk == kTokenNot )
	{
		if( ro )
			BREAK( L"Cannot set NOT NULL flag for constant" );

		type.mNotNULL = true;
		SkipToken( L"NULL" );
		NextToken();
		tk = GET_TOKEN;

		for( size_t i = cold ; i < count ; ++i )
			vars[ i ]->SetNotNULL( true );
	}

	// End of declarations
	if( tk == kTokenSemicolon )
		goto end;

	// Initialize variables with value
	
	if( tk == kTokenDefault )
	{
		/* Skip check */
	}
	else
	{
		if( mToken != L":" )
			INVALID_TOKEN
		
		SkipChar( '=' );
	}

	def = ReadExpression();
	
end:

	if( type.mNotNULL && def.empty() )
	{
		BREAK( L"'NOT NULL' variables must be initialized!" );
	}
	else
	{
		for( size_t i = cold ; i < count ; ++i )
		{
			if( def.length() )
			{
				Command* cmd = COMMAND( kCmdAssign );
				cmd->mpExpr = new Expression( *this, def, type.mType );
				cmd->mObject = i;
				
				mDefaults[ vars[ i ] ] = def;
			}
			else
			{
				Command* cmd = COMMAND( kCmdReset );
				cmd->mObject = i;
			}
		}			
	}			
}

/**********************************************************************************************/
void Parser::ReadVariablesNames( void )
{
	mVarNames.clear();

	bool first = true;
	while( mToken == L"," || first )
	{
		if( first )
			first = false;
		else
			NextToken( "%" );

		wstring ident = ReadIdent();
		
		CHECK_RESERVED( ident );
		CHECK_DUPLICATE( FindVariable( ident, true ) )
		CHECK_DUPLICATE( FindRecord( ident, true ) )
		
		mVarNames.push_back( ident );
		NextToken( "%" );
	}
}

/**********************************************************************************************/
void Parser::ReadWhen( void )
{
	CHECK_BLOCK

	wstring expr = ReadUntil( kTokenThen );

	// Finish last block - add GO TO to the end of IF...END IF construction
	if( mSwitches.empty() )
		BREAK( L"Unexpected WHEN" );
		
	mSwitches.back().push_back( COMMAND( kCmdGo ) );

	// Here at now points previous GO TO IF NOT
	PopNextCase();

	if( mCases.empty() )
		BREAK( L"Unexpected WHEN" );
			
	// Add new block to IF...END IF
	if( mCases.back().size() )
		PushCase( mCases.back() + L"=(" + expr + L")" );
	else
		PushCase( expr );
}


//////////////////////////////////////////////////////////////////////////
// command methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::PopNextCase( void )
{
	// Add dummy command - to be sure all be ok
	COMMAND( kCmdNope );
	
	if( mNextCases.empty() )
		BREAK( L"Unexpected end of block" )
		
	mNextCases.back()->mParam = mVM.LastCommandIndex();
	mNextCases.pop_back();
}

/**********************************************************************************************/
void Parser::PopSwitch( void )
{
	// Add dummy command - to be sure all be ok
	COMMAND( kCmdNope );

	if( mSwitches.empty() )
		BREAK( L"Unexpected END" );
		
	vector<Command*>& list = mSwitches.back();
	
	size_t count = list.size();
	for( size_t i = 0 ; i < count ; ++i )
		list[ i ]->mParam = mVM.LastCommandIndex();
		
	mSwitches.pop_back();
}

/**********************************************************************************************/
void Parser::PushCase( const wstring& expr )
{
	if( mSwitches.empty() )
		BREAK( L"Unexpected ELSIF or WHEN" );	
		
	// Push GO TO to next ELSIF/WHEN/END if expression fail
	Command* cmd = COMMAND( kCmdGoIfNot );	
	cmd->mpExpr = EXPRESSION( expr );
	PushNextCase( cmd );
}


/**********************************************************************************************/
SP_NAMESPACE_END
