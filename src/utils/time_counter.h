/**********************************************************************************************/
/* time_counter.h				                                                   			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once


/**********************************************************************************************/
class time_counter
{
	public://////////////////////////////////////////////////////////////////////////

								time_counter( void );

	public://////////////////////////////////////////////////////////////////////////

// This class API:

	// ---------------------
	// Methods

								/** Returns count of millesecond. */
		uint64_t				milliseconds( void ) const;
		
								/** Reset counter. */
		void					reset( void );
		
                                /** Format interval to human-readable string,
                                 *  for example, "12.145 seconds". */
		string					value( void ) const;


	protected://////////////////////////////////////////////////////////////////////////

		uint64_t				mStart;
};
