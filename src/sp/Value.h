/**********************************************************************************************/		
/* Value.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"
#include "Utils.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Value class uses to store value of a single variable
//
class Value
{
	public://////////////////////////////////////////////////////////////////////////

								Value( void );
								Value( double value );
								Value( int value );
								Value( const wstring& value );
								Value( const Value& value );
                                
	public://////////////////////////////////////////////////////////////////////////

// This class API:

	// -----------------------
	// Type methods

								/** Change type of value. */
		void					Convert( EValueType type );


	public://////////////////////////////////////////////////////////////////////////

	// ---------------------
	// Access data

								/** Return value as double. */
		double					ToDouble( void ) const;
		
								/** Return value as int. */
		int						ToInt( void ) const;
		
								/** Return value as string. */
		const wstring&			ToString( void ) const;

        
	public://////////////////////////////////////////////////////////////////////////

// Properties
		
        union
        {
mutable     double				mDouble;
mutable     int					mInt;
        };
        
mutable	wstring					mStr;
		EValueType				mType;
};


/**********************************************************************************************/
SP_NAMESPACE_END
