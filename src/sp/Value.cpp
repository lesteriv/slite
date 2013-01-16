/**********************************************************************************************/		
/* Value.cpp																				  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

// SP
#include "Value.h"

/**********************************************************************************************/
SP_NAMESPACE_START


//////////////////////////////////////////////////////////////////////////
// constructors
//////////////////////////////////////////////////////////////////////////
	
	
/**********************************************************************************************/		
Value::Value( void )
:
	mType( kUnknown )
{
}

/**********************************************************************************************/		
Value::Value( double value )
:
	mDouble	( value ),
	mType	( kDouble )
{
}

/**********************************************************************************************/		
Value::Value( int value )
:
	mInt	( value ),
	mType	( kInt )
{
}

/**********************************************************************************************/		
Value::Value( const wstring& value )
:
	mStr	( value ),
	mType	( kText )
{
}

/**********************************************************************************************/		
Value::Value( const Value& value )
:
	mType( value.mType )
{
	switch( mType )
	{
		case kDouble	: mDouble	= value.mDouble; break;
		case kInt		: mInt		= value.mInt; break;
		case kText		: mStr		= value.mStr; break;
		default:;
	}
}


//////////////////////////////////////////////////////////////////////////
// type methods
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/		
void Value::Convert( EValueType type )
{
	if( mType == type )
		return;

	switch( type )
	{
		case kDouble	: *this = ToDouble(); break;
		case kInt		: *this = ToInt(); break;
		case kText		: *this = ToString(); break;
		default:;
	}
}


//////////////////////////////////////////////////////////////////////////
// access data
//////////////////////////////////////////////////////////////////////////


/**********************************************************************************************/		
double Value::ToDouble( void ) const
{
	if( mType == kInt )
		mDouble = mInt;
	else if( mType != kDouble )
		mDouble = wcstod( mStr.c_str(), NULL );

	return mDouble;
}

/**********************************************************************************************/		
int Value::ToInt( void ) const
{
	if( mType == kDouble )
		mInt = (int) mDouble;
	else if( mType != kInt )
		mInt = wcstol( mStr.c_str(), NULL, 10 );

	return mInt;
}

/**********************************************************************************************/		
const wstring& Value::ToString( void ) const
{
	if( mType == kDouble )
		mStr = DoubleToStr( mDouble );
	else if( mType == kInt )
		mStr = IntToStr( mInt ); 
		
	return mStr;
}


/**********************************************************************************************/
SP_NAMESPACE_END
