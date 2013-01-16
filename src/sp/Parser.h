/**********************************************************************************************/		
/* Parser.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// STD
#include <setjmp.h>

// SP
#include "block.h"
#include "Command.h"
#include "Loop.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Class Parser uses to parse stored procedure text
//
class Parser
{
	public://///////////////////////////////////////////////////////////////////////////////////
		
									Parser(
										VMachine&		out,
										const wstring&	text );

	public://///////////////////////////////////////////////////////////////////////////////////

// This class API:		

	// -----------------------
	// Methods
		
		void						Parse( void );
		
		
	protected://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Internal methods

									/** Reads next token into @mToken, if @delims not NULL -
									 *	use specified characters as delimiters */
		void						NextToken( const char* delims = NULL );
		
		void						ReadSubType( void );
		void						ReadUserType( void );
		void						ReadVariables( void );
		void						ProcessGeneralToken( void );
		void						ProcessToken( void );

		void						ReadCase( void );
		void						ReadClose( void );
		void						ReadCreate( void );
		void						ReadCursor( void );
		void						ReadElse( void );
		void						ReadElsIf( void );
		void						ReadEnd( void );
		void						ReadExec( void );
		void						ReadExecute( void );
		void						ReadExit( void );
		void						ReadFetch( void );
		void						ReadFor( void );
		void						ReadFunction( void );
		void						ReadGoto( void );
		void						ReadIf( void );
		void						ReadLabel( void );
		void						ReadLoop( void );
		void						ReadNull( void );
		void						ReadOpen( void );
		void						ReadOutput( void );
		void						ReadPrint( void );
		void						ReadProcedure( void );
		void						ReadReturn( void );
		void						ReadQuery( void );
		void						ReadSelect( void );
		void						ReadSet( void );
		const Type					ReadType( wstring* def = NULL );
		void						ReadVariablesNames( void );
		void						ReadWhen( void );
		void						ReadWhile( void );
		

	protected://///////////////////////////////////////////////////////////////////////////////////
		
	// -----------------------
	// Sections methods		

		ESection					GetSection( void ) const
										{ size_t count = mSections.size(); return count ? mSections[ count - 1 ] : kSectionUnknown; }
									
		void						ProcessEnd( bool label = false );
		void						ReadBegin( void );
		void						ReadException( void );
		void						StartDeclare( void );
		

	public://///////////////////////////////////////////////////////////////////////////////////
		
	// -----------------------
	// Util methods

									/** Adds new cursor to current scope. */
		int							AddCursor(
										const wstring&	name,
										const wstring&	expr );
		
									/** Adds new record to current scope. */
		vector<int>					AddRecord(
										const wstring&	name,
										const Type&		type,
										bool			ro = false );
		
									/** Adds new subtype to current scope. */
		void						AddSubType(
										const wstring&	name,
										const Type&		type );
		
									/** Adds new variable to current scope. */
		int							AddVariable(
										const wstring&	name,
										EValueType		type );

									/** Returns pointer to accessable cursor,
									 *  @name - lower-cased name of variable;
									 *  @curr - TRUE if search only in current block. */									 
		Cursor*						FindCursor(
										const wstring&	name,
										bool			curr = false );
										
									/** Returns pointer to accessable record,
									 *  @name - lower-cased name of variable;
									 *  @curr - TRUE if search only in current block. */									 
		const Type*					FindRecord(
										const wstring&	name,
										bool			curr = false );
										
									/** Returns pointer to accessable subtype,
									 *  @name - lower-cased name of variable;
									 *  @curr - TRUE if search only in current block. */
		const Type*					FindSubType(
										const wstring&	name,
										bool			curr = false );
										
									/** Returns pointer to accessable variable,
									 *  @name - lower-cased name of variable;
									 *  @curr - TRUE if search only in current block. */
		Variable*					FindVariable( 
										const wstring&	name,
										bool			curr = false );

									/** Read and returns expression. */
		wstring						ReadExpression( const char* sp = ";," );
		
									/** Read parameters for procedure or function. */
		void						ReadParameters( void );

									/** Read user's record type. */
		Type						ReadRecord( void );
		
									/** Read and returns statement. */
		wstring						ReadStatement( void )						{ return ReadExpression( ";" ); }
		
									/** Read and returns expression until token. */
		wstring						ReadUntil( 
										EToken			token,
										bool			allowEmpty = false );

									/** Read and returns expression until token. */
		wstring						ReadUntil(
										const wstring&	token,
										bool			allowEmpty = false );

									/** Read single identifier. */
		wstring						ReadIdent( bool complex = false );

									/** Move to next token and read single identifier. */
		wstring						ReadNextIdent( bool complex = false );
		
									/** Read coma-separated list. */
		vector<wstring>				ReadList( void );
		
									/** Restore current token and position. */
		void						Restore( void );

									/** Remember current token and position and read next token. */
		void						SaveAndGo( const char* delims = NULL );

									/** Skip character. */
		void						SkipChar( char ch );
		
									/** Skip multiline comment. */
		void						SkipComment( void );

									/** Skip rest of current line. */
		void						SkipLine( void );

									/** Read expected token and skip it without processing. */
		void						SkipToken( const wstring& tk );
		

	protected://///////////////////////////////////////////////////////////////////////////////////

	// -----------------------
	// Command methods
	
									/** Add 'nope' command as destination for last GO TO to the next case block. */
		void						PopNextCase( void );

									/** Decrease level of nested IF/CASE blocks,
									 *  correct GO TO to finish switch block( @mCmdsGoSwitchFinish ). */
		void						PopSwitch( void );

									/** Add IF NOT expr GO TO command( see @mCmdsGoNextCase ). */
		void						PushCase( const wstring& expr );

									/** Add expession for next case nested block. */
		void						PushCaseExpr( const wstring& str )			{ mCases.push_back( str ); }

									/** Add GO TO command to mCmdsGoNextCase, see @mCmdsGoNextCase description. */
		void						PushNextCase( Command* cmd )				{ mNextCases.push_back( cmd ); }

									/** Increase level of nested IF/CASE blocks. */
		void						PushSwitch( void )							{ mSwitches.resize( mSwitches.size() + 1 ); }


	private://///////////////////////////////////////////////////////////////////////////////////

// Operators

									/** Disable copy instance of this class. */
		Parser&						operator=( const Parser& )					{ return *this; }


	public://///////////////////////////////////////////////////////////////////////////////////

// Source data

		size_t						mEnd;						// Length of source text( last position )
		const wstring&				mText;						// Source text


// Intermediate results

		vector<Block>				mBlocks;					// Stack of scopes
		vector<wstring>				mCases;						// Array of cases switch expresions
																// CASE v_letter - that's it

		hmap<Variable*,wstring>		mDefaults;					// Default values for variables
		
		vector<Command*>			mNextCases;					// Stack for GO TO commands for IF/CASE to move to the next ELSIF, ELSE, WHEN command, sample if is:
																// 10. WHEN 'A' THEN	-- Here we add GO TO <next> IF NOT 'A' to mCmdsGoNextCase
																// 11. PRINT 'A';
																// 12. WHEN 'B' THEN

		vector<vector<Command*> >	mSwitches;					// Stack for GO TO commands for IF/CASE to finish blocks, sample of use:
																// 10. WHEN 'A' THEN
																// 11. PRINT 'A';
																// 12. GOTO <end>		-- This command stored into mCmdsGoSwitchFinish[ if/case nesting level ]
																//						-- to skip all next WHEN/ELSIF/ELSE blocks
																// 13. WHEN 'B' THEN
																// ...
																// 20. END CASE <end>

		bool						mEndReached;				// TRUE if we have reached last END token
		wstring						mErr;						// Current error's string
		int							mException;					// TODO
		size_t						mLine;						// Current line
		vector<Loop>				mLoops;						// Positions of loops start
		size_t						mPos;						// Current position
		vector<ESection>			mSections;					// Stack of sections( its types )
		wstring						mToken;						// Current token


// References

		VMachine&					mVM;						// Parent Virtual Machine

		
// Save point to return to previous token

		size_t						mLineR;						// Current line
		size_t						mPosR;						// Position in text
		wstring						mTokenR;					// Token's value
		

// Temporary data
		
		wstring						mLabel;						// Label for current block/loop
		wstring						mLabelNext;					// Label for next block/loop
		vector<wstring>				mVarNames;					// Names of variables in current declaration statement
		
		
// Internal data
		
		jmp_buf						mJBuf;
};


/**********************************************************************************************/
SP_NAMESPACE_END
