/**********************************************************************************************/
/* Command.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

// SP
#include "Command.h"
#include "Cursor.h"
#include "Field.h"
#include "Unicode.h"
#include "VMachine.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Command::Command(
	VMachine*	vm,
	ECommand	id,
	int			catchIndex )
:
// References
	mpExpr		( NULL ),
	mpStmt		( NULL ),
	mpVM		( vm ),

// Execution data
	mCatchIndex	( catchIndex ),

// Properties
	mCmdID		( id ),
	mObject		( -1 )
{
}

/**********************************************************************************************/
Command::~Command( void )
{
	delete mpExpr;
	delete mpStmt;
}


//////////////////////////////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Command* Command::Clone( VMachine* vm )
{
	Command* res = new Command( vm, mCmdID, mCatchIndex );

	if( mpExpr ) res->mpExpr = mpExpr->Clone( vm );
	if( mpStmt ) res->mpStmt = mpStmt->Clone( vm );
	
	res->mFields	= mFields;
	res->mObject	= mObject;
	res->mParam		= mParam;

	return res;
}

/**********************************************************************************************/
bool Command::Execute( void )
{
	switch( mCmdID )
	{
		case kCmdAssign		: return DoAssign();
		case kCmdClose		: return DoCloseCursor();
		case kCmdExec		: return DoExec();
		case kCmdFetch		: return DoFetch();
		case kCmdGoIf		: return DoGoIf();
		case kCmdGoIfNot	: return DoGoIfNot();
		case kCmdOpen		: return DoOpenCursor();
		case kCmdParam		: return DoReadParam();
		case kCmdPrint		: return DoPrint();
		case kCmdReset		: return mpVM->mVariables[ mObject ]->mIsNULL = true;
		case kCmdReturn		: return DoReturn();
		case kCmdSelect		: return DoSelect();

		case kCmdGo			: mpVM->mPos = mParam.ToInt();
		case kCmdNope		: return true;
		
		default:;
	}

	return false;
}

/**********************************************************************************************/
wstring Command::Explain( void )
{
	wstring	res;

	wstring exp	 = mpExpr && mpExpr->mpText ? *mpExpr->mpText : res;
	wstring pm	 = mParam.ToString();
	wstring st	 = mpStmt && mpStmt->mpText ? *mpStmt->mpText : res;

	switch( mCmdID )
	{
		case kCmdClose	:
		case kCmdFetch	:
		case kCmdOpen	:
		{
			wstring name = L" CURSOR " + mpVM->GetCursorName( mpVM->mCursors[ mObject ] );
			
			switch( mCmdID )
			{
				case kCmdClose	: return L"CLOSE" + name;
				case kCmdFetch	: return L"FETCH" + name;
				case kCmdOpen	: return L"OPEN"  + name;
				
				default:;
			}
		}
		break;

		case kCmdExec		: return L"EXEC " + ( exp.length() ? exp : st );
		case kCmdGo			: return L"JMP " + pm;
		case kCmdGoIf		: return L"JMP " + pm + L" IF " + exp;
		case kCmdGoIfNot	: return L"JMP " + pm + L" IF NOT " + exp;
		case kCmdNope		: return L"NOPE";
		case kCmdParam		: return L"READ PARAMETER " + mpVM->GetVariableName( mpVM->mVariables[ mObject ] );
		case kCmdPrint		: return L"PRINT " + exp;
		case kCmdReset		: return mpVM->GetVariableName( mpVM->mVariables[ mObject ] ) + L" := NULL";
		case kCmdReturn		: return L"RETURN " + exp;
		case kCmdSelect		: return L"SELECT" + pm;

		case kCmdAssign		:
		{
			if( mObject >= 0 )
				return mpVM->GetVariableName( mpVM->mVariables[ mObject ] ) + L" := " + exp;
			
			wstring str;
			
			size_t count = mFields.size();
			for( size_t i = 0 ; i < count ; ++i )
			{
				if( str.length() )
					str += ',';

				str += mpVM->GetVariableName( mpVM->mVariables[ mFields[ i ] ] );
			}
			
			return str + L" := " + exp;
		}
		
		default:;
	}

	return res;
}


//////////////////////////////////////////////////////////////////////////
// internal execute methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
bool Command::DoAssign( void )
{
	// Select into record
	size_t count = mFields.size();
	if( count )
	{
		vector<Value> vls = mpExpr->get_Values();
		if( vls.size() && count != vls.size() )
		{
			mpVM->PrintError( "Different source/destination count on select into" );
			return false;
		}
		
		mpVM->mVariables[ mpVM->mVarFound	 ]->AssignValue( vls.size() > 0 );
		mpVM->mVariables[ mpVM->mVarNotFound ]->AssignValue( vls.size() == 0 );
		
		count = min( count, vls.size() );
		for( size_t i = 0 ; i < count ; ++i )
			mpVM->mVariables[ mFields[ i ] ]->AssignValue( vls[ i ] );
	}
	// Select into variable or usual assign
	else
	{
		const Value& vl = mpExpr->get_Value();
		
		if( mpExpr->mKind == kExpression )
			mpVM->mVariables[ mpVM->mVarRowCount ]->AssignValue( vl.mType != kUnknown );
		
		mpVM->mVariables[ mObject ]->AssignValue( vl );
	}

	return true;
}

/**********************************************************************************************/
bool Command::DoCloseCursor( void )
{
	// TODO: exceptions

	mpVM->mCursors[ mObject ]->Close();
	return true;
}

/**********************************************************************************************/
bool Command::DoExec( void )
{
	bool res = mpStmt ?
		mpStmt->Execute() :
		SqliteExecute( mpVM->mpDB, mpExpr->get_Value().ToString() ) == SQLITE_OK;

	if( !res )
		mpVM->PrintError();
	
	return res;
}

/**********************************************************************************************/
bool Command::DoFetch( void )
{
	// TODO: exceptions

	Cursor* cr = mpVM->mCursors[ mObject ];

	// TODO: exception
	if( !cr->mpStmt || !cr->mpStatQ )
		return true;

	cr->mpStatQ->ApplyBinds();
	int res = sqlite3_step( cr->mpStmt );
	bool step = ( res == SQLITE_ROW );

	// Update %FOUND and %NOTFOUND properties
	cr->GetVarFound()->AssignValue( step );
	cr->GetVarNotFound()->AssignValue( !step );

	// TODO: exception
	if( !step )
		return true;

	Value vl;

	// Read values from cursor
	int count = min<int>( mFields.size(), sqlite3_column_count( cr->mpStmt ) );
	for( int i = 0 ; i < count ; ++i )
	{
		Variable* var = mpVM->mVariables[ mFields[ i ] ];
		ReadValue( cr->mpStmt, vl, i, var->mValue.mType );
		var->AssignValue( vl );
	}

	return true;
}

/**********************************************************************************************/
bool Command::DoGoIf( void )
{
	if( mpExpr->get_Value().ToInt() )
		mpVM->mPos = mParam.ToInt();

	return true;
}

/**********************************************************************************************/
bool Command::DoGoIfNot( void )
{
	if( !mpExpr->get_Value().ToInt() )
		mpVM->mPos = mParam.ToInt();

	return true;
}

/**********************************************************************************************/
bool Command::DoOpenCursor( void )
{
	// TODO: exceptions

	mpVM->mCursors[ mObject ]->Open();
	return true;
}

/**********************************************************************************************/
bool Command::DoPrint( void )
{
	const Value& vl = mpExpr->get_Value();
	if( vl.mType == kUnknown )
		mpVM->Print( "<NULL>" );
	else
		mpVM->PrintW( vl.ToString() );
	
	return true;
}

/**********************************************************************************************/
bool Command::DoReadParam( void )
{
	Variable* var = mpVM->mVariables[ mObject ];
	
	if( mpVM->mArgcRun <= mObject )
	{
		var->AssignValue( mpExpr->get_Value() );
		return true;
	}
	
	sqlite3_value* vl = mpVM->mpArgv[ mObject ];
	switch( sqlite3_value_type( vl ) )
	{
		case kDouble	: var->AssignValue( sqlite3_value_double( vl ) ); break;
		case kInt		: var->AssignValue( sqlite3_value_int( vl ) ); break;
		
		case kText		:
		{
			const char* str = (const char*) sqlite3_value_text( vl );
			wstring wstr;
			ConvertFromUTF8( str, strlen( str ), wstr );
			
			var->AssignValue( wstr );
		}
		break;
		
		default:
			var->AssignValue( Value() );
			break;
	}
	
	return true;
}

/**********************************************************************************************/
bool Command::DoReturn( void )
{
	if( mpExpr )
	{
		sqlite3_context* ctx = mpVM->mpContext;

		Value vl = mpExpr->get_Value();
		if( mpVM->mType != kUnknown && vl.mType != mpVM->mType )
			vl.Convert( mpVM->mType );

		switch( vl.mType )
		{
			case kDouble	: RESULT_DOUBLE( vl.ToDouble() ); break;
			case kInt		: RESULT_INT( vl.ToInt() ); break;
			case kText		: RESULT_WTEXT( vl.ToString() ); break;

			default:
				sqlite3_result_null( mpVM->mpContext );
		}
	}
	
	return false;
}

/**********************************************************************************************/
bool Command::DoSelect( void )
{
	sqlite3_stmt* stmt = SqliteSelect( mpVM->mpDB, L"SELECT" + mParam.ToString() );
	if( !stmt )
	{
		mpVM->PrintError();
		return false;
	}
	
	int fcount = sqlite3_column_count( stmt );
	
	vector<vector<wstring> > tbl;
	vector<wstring>	row( fcount );
	vector<size_t> w( fcount, 2 );
	
	// Header
	for( int i = 0 ; i < fcount ; ++i )
	{
		const char* str = (const char*) sqlite3_column_name( stmt, i );
		FROM_UTF8( str, row[ i ] );
		w[ i ] = max( w[ i ], row[ i ].length() );
	}
	tbl.push_back( row );

	// Records
	int res = sqlite3_step( stmt );
	while( res == SQLITE_ROW )
	{
		for( int i = 0 ; i < fcount ; ++i )
		{
			const char* str = (const char*) sqlite3_column_text( stmt, i );
			if( str )
				FROM_UTF8( str, row[ i ] )
			else
				row[ i ].clear();
			
			w[ i ] = max( w[ i ], row[ i ].length() );
		}

		tbl.push_back( row );
		res = sqlite3_step( stmt );
	}

	// Horizontal line
	string fline( "+-" );
	for( int j = 0 ; j < fcount ; ++j )
	{
		fline.append( w[ j ], '-' );

		if( j < fcount - 1 )
			fline += "-+-";
	}
	fline += "-+";
	mpVM->Print( &fline[ 0 ] );
			
	// Print records
	size_t count = tbl.size();
	for( size_t i = 0 ; i < count ; ++i )
	{
		wstring line( L"| " );
		
		vector<wstring>& row = tbl[ i ];
		for( int j = 0 ; j < fcount ; ++j )
		{
			line += row[ j ];
			line.append( w[ j ] - row[ j ].length(), ' ' );
			
			if( j < fcount - 1 )
				line += L" | ";
		}

		line += L" |";
		mpVM->PrintW( line );
		
		if( !i )
			mpVM->Print( &fline[ 0 ] );
	}
	
	if( count > 1 )
		mpVM->Print( &fline[ 0 ] );
	
	FINALIZE_STATEMENT( stmt );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// internal methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
void Command::Throw( void )
{
	// TODO:
	mpVM->Catch( mCatchIndex );
}


/**********************************************************************************************/
SP_NAMESPACE_END
