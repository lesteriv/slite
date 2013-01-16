/**********************************************************************************************/		
/* Enums.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Enums

enum ECommand
{
	kCmdUnknown = 0,
	kCmdAssign,
	kCmdClose,
	kCmdExec,
	kCmdFetch,
	kCmdGo,
	kCmdGoIf,
	kCmdGoIfNot,
	kCmdNope,
	kCmdOpen,
	kCmdParam,
	kCmdPrint,
	kCmdReset,
	kCmdReturn,
	kCmdSelect
};

enum EExpressionKind
{
	kConstant,
	kExpression,
	kVariable
};

enum ELoop
{
	kLoop,
	kLoopFor,
	kLoopWhile
};

enum ESection
{
	kSectionUnknown	= 0,
	kSectionBegin,
	kSectionDeclare,
	KSectionElse,
	kSectionEnd,
	kSectionException,
	kSectionIf
};

enum EToken
{
	kTokenUnknown	= 0,
	
	// SQL
	
	kTokenAlter,
	kTokenAnalyze,
	kTokenAttach,
	kTokenCommit,
	kTokenDelete,
	kTokenDetach,
	kTokenDrop,
	kTokenExplain,
	kTokenInsert,
	kTokenPragma,
	kTokenReindex,
	kTokenReplace,
	kTokenRollback,
	kTokenSavePoint,
	kTokenUpdate,
	kTokenVacuum,

	// PL/SQL
	
	kTokenBegin,
	kTokenBraceLeft,
	kTokenBraceRight,
	kTokenCase,
	kTokenClose,
	kTokenCreate,
	kTokenCursor,
	kTokenDeclare,
	kTokenDefault,
	kTokenElse,
	kTokenElsIf,
	kTokenEnd,
	kTokenException,
	kTokenExec,
	kTokenExecute,
	kTokenExit,
	kTokenFetch,
	kTokenFor,
	kTokenGoto,
	kTokenIf,
	kTokenInto,
	kTokenLess,
	kTokenLoop,
	kTokenNot,
	kTokenNull,
	kTokenOff,
	kTokenOpen,
	kTokenOn,
	kTokenOutput,
	kTokenPrint,
	kTokenRelease,
	kTokenReturn,
	kTokenReverse,
	kTokenRowType,
	kTokenSelect,
	kTokenSemicolon,
	kTokenSeparator,
	kTokenServerOutput,
	kTokenSet,
	kTokenSubType,
	kTokenThen,
	kTokenType,
	kTokenWhen,
	kTokenWhile
};

enum EValueType
{
	kUnknown	= SQLITE_NULL,
	kBLOB		= SQLITE_BLOB,
	kDouble		= SQLITE_FLOAT,
	kInt		= SQLITE_INTEGER,
	kText		= SQLITE_TEXT
};


/**********************************************************************************************/
SP_NAMESPACE_END
