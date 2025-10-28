#pragma once
#include <tchar.h>

namespace var_sys{

typedef float         var_float;
typedef signed long   var_long;
typedef unsigned long var_word;

const var_word VAR_STR_LEN=127;

typedef TCHAR var_char;
typedef var_char var_string[VAR_STR_LEN+1];

} //namespace var_sys
