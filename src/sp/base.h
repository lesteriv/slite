/**********************************************************************************************/		
/* Base.h																					  */
/*																							  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

// SQLITE
#include "../third/sqlite/sqlite3.h"

// SP
#include "Macroses.h"
#include "Enums.h"

// STD
#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <map>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wctype.h>

/**********************************************************************************************/		
using namespace std;

/**********************************************************************************************/
typedef unsigned char uchar;
#define hmap unordered_map
#define hset unordered_set

/**********************************************************************************************/
SP_NAMESPACE_START
class Cursor;
class Expression;
class Parser;
class Statement;
class Variable;
class VMachine;
class Value;
struct Field;
SP_NAMESPACE_END


/**********************************************************************************************/
using namespace SP;
