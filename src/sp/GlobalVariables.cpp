/**********************************************************************************************/
/* GlobalVariables.cpp																		  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "GlobalVariables.h"
#include "Utils.h"
#include "Variable.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// global data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
static VariableMap gGlobalVariables;


//////////////////////////////////////////////////////////////////////////
// functions
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/
Variable* AddGlobalConstant(
	const wstring&	name,
	const Value&	value )
{
	Variable* var = AddGlobalVariable( name, value );
	var->mReadOnly = true;
	
	return var;
}

/**********************************************************************************************/
Variable* AddGlobalVariable(
	const wstring&	name,
	const Value&	value )
{
	Variable* var = FindGlobalVariable( name );
	if( var )
		return var;

	var = new Variable;
	var->AssignValue( value );
	gGlobalVariables[ name ] = var;
	
	return var;
}

/**********************************************************************************************/
Variable* FindGlobalVariable( const wstring& name )
{
	auto it = gGlobalVariables.find( name );
	return it != gGlobalVariables.end() ?
		it->second :
		NULL;
}


/**********************************************************************************************/
SP_NAMESPACE_END
