#pragma once

/*
  All macros here a with snake case with a trailing underscore in the model off
  foo_bar_, bar_ foo_, foo_bar_baz_, etc.

  This file is macro only so the macros are easy to locate. Constants can be placed
  elsewhere.
 */

#ifdef DEBUG_MODE
  #define debug_print_(expr) std::cout << expr
  #define debug_println_(expr) std::cout << expr << '\n'
#else
  #define debug_print_(expr)
  #define debug_println_(expr)
#endif

#define println_(expr) std::cout << expr << '\n'
#define print_(expr) std::cout << expr
