/**********************************************************************************************/		
/* Utils.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license   																		  	  */
/**********************************************************************************************/

#pragma once

// SP
#include "base.h"
#include "Cursor.h"
#include "Type.h"

/**********************************************************************************************/
SP_NAMESPACE_START


/**********************************************************************************************/
// Functions

					/** Bind value to the statement. */
void				BindValue( 
						sqlite3_stmt*	stmt,
						int				index,
						const Value&	vl );

					/** Converts name of type to constant. */
EValueType			ConvertTypeNameToEnum( const wstring& name );

					/** Returns type of column from statement. */
EValueType			GetColumnType(
						sqlite3_stmt*	stmt,
						int				index );

					/** Returns type of column by name. */
EValueType			GetColumnType(
						sqlite3*		db,
						const wstring&	table,
						const wstring&	field );

					/** Returns TRUE if name valid to be used for variable, function etc.*/
bool				IsValidName( const wstring& name );

					/** Returns TRUE if type valid to be used for variable, function etc.*/
bool				IsValidType( const wstring& name );

					/** Returns lowercased string. */
inline wstring		Lower( const wstring& rs )
					{
						wstring s = rs;
						transform( s.begin(), s.end(), s.begin(), towlower );
						return s;
					}
					
					/** Replace ' to '' for use string in query. */
inline void			QuoteString( wstring& s )
					{
						size_t len = s.length();
						for( size_t i = 0 ; i < len ; ++i )
						{
							if( s[ i ] == '\'' )
							{
								s.insert( i, L"'" );
								++i; ++len;
							}
						}
					}

					/** Read and store table's structure. */
void				ReadTableStructure(
						Type&			type,
						sqlite3*		db,
						const wstring&	table );

					/** Read single value from statement. */
void				ReadValue(
						sqlite3_stmt*	stmt,
						Value&			vl,
						int				col		= 0,
						EValueType		type	= kUnknown );
	
					/** Sets pointer to first non-space character in string. */
inline void			SkipTabulation( const char*& p )
					{
						while( isspace( *p ) )
							++p;
					}

					/** Execute query. */
int					SqliteExecute(
						sqlite3*		db,
						const wstring&	query );
	
					/** Creates cursor from query result. */
Cursor*				SqliteQuery(
						sqlite3*		db,
						const wstring&	query );

					/** 'Converts' query to prepared statement. */
sqlite3_stmt*		SqliteSelect(
						sqlite3*		db,
						const wstring&	query );

					/** Makes string lowercased. */
inline wstring&		ToLower( wstring& s )
					{
						transform( s.begin(), s.end(), s.begin(), towlower );
						return s;
					}

					/** Converts string token to constant. */
EToken				ToToken( const wstring& s );

					/** Remove space characters from begin of string. */
void				TrimLeft( wstring& s );

					/** Remove space characters from end of string. */
void				TrimRight( wstring& s );

					/** Remove space characters from begin and end of string. */
inline void			TrimAll( wstring& s )
					{
						TrimLeft( s );
						TrimRight( s );
					}

			
/**********************************************************************************************/
// Additional string functions

					/** Converts number to string. */
inline wstring		DoubleToStr( double v )
					{
						wchar_t buf[ 64 ];
						swprintf( buf, 64, L"%.10g", v );
						return buf;
					}

					/** Converts number to string. */
inline wstring		IntToStr( int v )
					{
						wchar_t buf[ 32 ];
						swprintf( buf, 32, L"%d", v );
						return buf;
					}


/**********************************************************************************************/
SP_NAMESPACE_END
