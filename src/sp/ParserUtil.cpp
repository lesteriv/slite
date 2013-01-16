/**********************************************************************************************/		
/* ParserUtil.cpp																			  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "GlobalVariables.h"
#include "Parser.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Parser::NextToken( const char* delims )
{
	mToken.clear();
	
	while( mPos < mEnd )
	{
		wchar_t ch = mText[ mPos ];

		// Multiline comment
		if( ch == '/' && mPos < mEnd - 1 && mText[ mPos + 1 ] == '*' )
		{
			if( mToken.length() )
				return;
			
			SkipComment();
			continue;
		}

		// Singleline comment
		if( ch == '-' && mPos < mEnd - 1 && mText[ mPos + 1 ] == '-' )
		{
			if( mToken.length() )
				return;
			
			SkipLine();
			continue;
		}

		// New line
		if( ch == '\n' || ch == '\r' )
		{
			if( mToken.length() )
			{
				break;
			}
			else
			{
				if( ch == '\n' )
					++mLine;

				++mPos;
				continue;
			}
		}

		// Space
		if( ch == ' ' || ch == '\t' )
		{
			if( mToken.length() )
			{
				break;
			}
			else
			{
				++mPos;
				continue;
			}
		}

		// One character tokens
		if( strchr( "{}=:,<>\".()[]'", ch ) || ( delims && strchr( delims, ch ) ) )
		{
			if( mToken.length() )
			{
				break;
			}
			else
			{
				++mPos;
				mToken += ch;
				break;
			}
		}
		
		// End of token
		if( ch == ';' )
		{
			if( mToken.length() )
			{
				break;
			}
			else
			{
				++mPos;
				mToken += ch;
				break;
			}
		}

		++mPos;
		mToken += ch;
	}
}


//////////////////////////////////////////////////////////////////////////
// util methods
//////////////////////////////////////////////////////////////////////////


/**************************************************************************************/
int Parser::AddCursor(
	const wstring&	name,
	const wstring&	expr )
{
	Cursor* cr = new Cursor( this, expr );
	
	mBlocks.back().mCursors[ name ] = cr;
	mVM.mCursors.push_back( cr );
	mVM.mCursorsMap[ name ] = cr;
	
	cr->AssignVariables(
		AddVariable( name + L"%found"	, kInt ),
		AddVariable( name + L"%isopen"	, kInt ),
		AddVariable( name + L"%notfound", kInt ) );
	
	return (int) mVM.mCursors.size() - 1;
}

/**************************************************************************************/
vector<int> Parser::AddRecord( 
	const wstring&	name,
	const Type&		type,
	bool			ro )
{
	vector<int> res;
	
	// Record
	
	Block& bl = mBlocks.back();
	
	if( bl.mRecords.find( name ) != bl.mRecords.end() )
		BREAK( L"Variable '" + name + L"' already declared" );

	bl.mRecords[ name ] = type;
	
	// Fields

	vector<Variable*>& vars = mVM.mVariables;
	
	wstring base = name + L'.';
	size_t count = type.mMembers.size();
	for( size_t i = 0 ; i < count ; ++i )
	{
		const Member& fld = type.mMembers[ i ];
		res.push_back( vars.size() );

		Variable* var = new Variable;
		var->SetNotNULL( fld.mNotNULL );
		var->mReadOnly = ro;
		var->SetType( fld.mType );
		vars.push_back( var );

		wstring name = base + fld.mName;
		bl.mVarsMap[ name ] = var;
		mVM.mVariablesNames[ var ] = name;

		if( bl.mLabel.length() )
			bl.mVarsMap[ bl.mLabel + L"." + name ] = var;
	}	
	
	return res;
}

/**************************************************************************************/
void Parser::AddSubType( 
	const wstring&	name,
	const Type&		type )
{
	Block& bl = mBlocks.back();
	
	if( bl.mTypes.find( name ) != bl.mTypes.end() )
		BREAK( L"SubType '" + name + L"' already declared" );

	bl.mTypes[ name ] = type;
}

/**************************************************************************************/
int Parser::AddVariable(
	const wstring&	name,
	EValueType		type )
{
	CHECK_DUPLICATE( FindVariable( name, true ) );

	Variable* var = new Variable;
	var->SetType( type );

	Block& bl = mBlocks.back();
	bl.mVarsMap[ name ] = var;

	if( bl.mLabel.length() )
		bl.mVarsMap[ bl.mLabel + L"." + name ] = var;
	
	mVM.mVariables.push_back( var );
	mVM.mVariablesNames[ var ] = name;

	return (int) mVM.mVariables.size() - 1;
}

/**************************************************************************************/
Cursor* Parser::FindCursor(
	const wstring&	name,
	bool			curr )
{
	int count = mBlocks.size();
	int end = curr ? count - 1 : 0;
	for( int i = count - 1 ; i >= end ; --i )
	{
		Block& bl = mBlocks[ i ];
		CursorMap::iterator it = bl.mCursors.find( name );
		if( it != bl.mCursors.end() )
			return it->second;	
	}
	
	return NULL;
}

/**************************************************************************************/
const Type* Parser::FindRecord(
	const wstring&	name,
	bool			curr )
{
	int count = mBlocks.size();
	int end = curr ? count - 1 : 0;
	for( int i = count - 1 ; i >= end ; --i )
	{
		Block& bl = mBlocks[ i ];
		TypeMap::iterator it = bl.mRecords.find( name );
		if( it != bl.mRecords.end() )
			return &it->second;	
	}
	
	return NULL;
}

/**************************************************************************************/
const Type* Parser::FindSubType(
	const wstring&	name,
	bool			curr )
{
	int count = mBlocks.size();
	int end = curr ? count - 1 : 0;
	for( int i = count - 1 ; i >= end ; --i )
	{
		Block& bl = mBlocks[ i ];
		TypeMap::iterator it = bl.mTypes.find( name );
		if( it != bl.mTypes.end() )
			return &it->second;	
	}
	
	return NULL;
}

/**************************************************************************************/
Variable* Parser::FindVariable( 
	const wstring&	name,
	bool			curr )
{
	int count = mBlocks.size();
	int end = curr ? count - 1 : 0;
	for( int i = count - 1 ; i >= end ; --i )
	{
		Block& bl = mBlocks[ i ];
		VariableMap::iterator it = bl.mVarsMap.find( name );
		if( it != bl.mVarsMap.end() )
			return it->second;	
	}
	
	return NULL;
}

/**********************************************************************************************/
void Parser::ReadParameters( void )
{
	bool def = false;
	
	do
	{
		Command* cmd = COMMAND( kCmdParam );
		
		wstring vname = ReadNextIdent();

		SkipToken( L"in" );
		NextToken();

		EValueType type = ReadType().mType;
		cmd->mObject = AddVariable( vname, type );
		cmd->mParam = mVM.mArgc++;

		NextToken();
		ToLower( mToken );
		
		if( mToken == L":" || mToken == L"default" )
		{
			if( !def )
			{
				mVM.mArgcMin = mVM.mArgc - 1;
				def = true;
			}
			
			if( mToken == L":" )
				SkipChar( '=' );
			
			cmd->mpExpr = new Expression( *this, ReadExpression(), type );
			NextToken();
		}
		else if( def )
		{
			BREAK( L"Default argument missing" );
		}
	}
	while( mToken == L"," );

	if( !def )
		mVM.mArgcMin = mVM.mArgc;
	
	if( mToken != L")" )
		INVALID_TOKEN;	
}

/**********************************************************************************************/
Type Parser::ReadRecord( void )
{
	Type res;
	res.mNotNULL = false;
	
	do
	{
		Member fld;
		fld.mNotNULL = false;
		fld.mName = ReadNextIdent();

		NextToken();
		fld.mType = ReadType().mType;
		res.mMembers.push_back( fld );
		
		NextToken();
	}
	while( mToken == L"," );

	if( mToken != L")" )
		INVALID_TOKEN;
	
	return res;
}

/**************************************************************************************/
wstring Parser::ReadExpression( const char* sp )
{
	wstring res;

	int	 br	= 0;
	bool qt	= false;

	while( mPos < mEnd )
	{
		wchar_t ch = mText[ mPos ];
		
		// SKIP SPACE AT START
		
		if( res.empty() && isspace( ch ) )
		{
			++mPos;
			continue;
		}
		
		// STOP ON SEPARATOR

		if( !qt && !br && strchr( sp, ch ) )
			break;
		
		// BRACES
		
		if( ch == ')' && !qt )
		{
			// TODO:
			
			// Stop on close brace
			if( br == 0 )
			{
				break;
			}
			// Close brace
			else
			{
				--br;
				if( br < 0 )
					BREAK( L"Unexpected close brace" ) 
			}
		}
		
		if( ch == '(' && !qt )
			++br;
			
		// QUOTES
		
		if( ch == '\'' )
		{
			if( !qt )
			{
				qt = true;
			}
			else
			{
				if( mPos < mEnd - 1 && mText[ mPos + 1 ] == '\'' )
				{
					res += '\'';
					++mPos;
				}
				else
				{
					qt = false;
				}
			}
		}	
			
		res += ch;
		++mPos;
	}
	
	if( res.empty() )
		BREAK( L"Expression expected" );

	if( mPos >= mEnd )
		BREAK( L"Unexpected end of function" );
		
	if( qt )
		BREAK( L"Unclosed string constant" )
		
	TrimRight( res );	
		
	return res;
}

/**********************************************************************************************/
wstring Parser::ReadUntil( 
	EToken	token,
	bool	allowEmpty )
{
	EToken tk;
	size_t start = mPos;
	
	do
	{
		NextToken();
		tk = GET_TOKEN;
	}
	while( tk != token && mToken.length() );
	
	size_t len = mToken.length();
	if( !len )
		BREAK( L"Unexpected end of function" );

	len = mPos - start - len;
	if( !len && !allowEmpty )
		BREAK( L"Expression expected" );

	wstring res = mText.substr( start, len );
	TrimAll( res );
	return res;
}

/**********************************************************************************************/
wstring Parser::ReadUntil(
	const wstring&	token,
	bool			allowEmpty )
{
	size_t start = mPos;

	do
	{
		NextToken();
	}
	while( mToken != token && mToken.length() );

	size_t len = mToken.length();
	if( !len )
		BREAK( L"Unexpected end of function" );

	len = mPos - start - len;
	if( !len && !allowEmpty )
		BREAK( L"Expression expected" );

	wstring res = mText.substr( start, len );
	TrimAll( res );
	return res;
}

/**********************************************************************************************/
wstring Parser::ReadIdent( bool complex )
{
	wstring ident;
	
start:
		
	if( mToken == L"`"  )
		ident += ReadUntil( L"`" );
	else if( mToken == L"\"" )
		ident += ReadUntil( L"\"" );
	else if( mToken == L"["	)
		ident += ReadUntil( L"]" );
	else
	{
		CHECK_IDENT
		ident += mToken;
	}

	wchar_t ch = complex && mPos < mEnd ? mText[ mPos ] : 0;
	if( ch == '.' )
	{
		ident += '.';
		mPos++;
		NextToken( "%" );
		
		goto start;
	}

	return Lower( ident );
}

/**********************************************************************************************/
wstring Parser::ReadNextIdent( bool complex )
{
	NextToken();
	return ReadIdent( complex );
}

/**********************************************************************************************/
vector<wstring> Parser::ReadList( void )
{
	vector<wstring> res;

	for(;;)
	{
		res.push_back( ReadNextIdent( true ) );

		SaveAndGo();
		if( mToken.empty() )
			break;

		if( mToken != L"," )
		{
			Restore();
			break;
		}
	}

	return res;
}

/**********************************************************************************************/
void Parser::Restore( void )
{
	mLine = mLineR;
	mPos = mPosR;
	mToken.swap( mTokenR );
}

/**********************************************************************************************/
void Parser::SaveAndGo( const char* delims )
{
	mLineR = mLine;
	mPosR = mPos;
	mToken.swap( mTokenR );

	NextToken( delims );
}

/**********************************************************************************************/
void Parser::SkipChar( char ch )
{
	if( mPos >= mEnd || mText[ mPos++ ] != ch )
		BREAK( L"Syntax error" ); // TODO
}

/**********************************************************************************************/
void Parser::SkipComment( void )
{
	wchar_t ch;
	while( mPos < mEnd )
	{
		ch = mText[ mPos++ ];
		
		if( ch == '\n' )
			++mLine;
		
		if( ch == '/' && mText[ mPos - 2 ] == '*' )
			break;
	}
	
	if( mPos >= mEnd )
		BREAK( L"PLS-00111: end-of-file in comment" );
}

/**********************************************************************************************/
void Parser::SkipLine( void )
{
	while( mPos < mEnd )
	{
		wchar_t ch = mText[ mPos++ ];
		if( ch == '\r' || ch == '\n' )
			break;
	}
	
	++mLine;
}

/**********************************************************************************************/
void Parser::SkipToken( const wstring& token )
{
	EToken tk = ToToken( token );

	NextToken();
	if( tk != GET_TOKEN )
		BREAK( L"PLS-00103: found '" + mToken + L"' but expected one of the following: '" + token + L"'" );
}


/**********************************************************************************************/
SP_NAMESPACE_END
