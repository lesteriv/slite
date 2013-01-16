/**********************************************************************************************/		
/* Unicode.h																				  */
/*																							  */
/* Copyright Paradigma Software, 2001-2011													  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Functions

				/** Converts string from standard wstring to array of UTF8 characters. */
void	        ConvertToUTF8(
					const wstring&	src,
                    vector<char>&	out );

				/** Converts string in UTF8 to standard wstring. */
void			ConvertFromUTF8(
					const char*		src,
					size_t			count,
                    wstring&		out );

				/** Converts string in UTF8 to standard wstring with UTF16 inside. */
void			ConvertUTF8ToUTF16(
					const char*		rsrc,
					size_t			count,
					wstring&		buf );


/**********************************************************************************************/
SP_NAMESPACE_END
