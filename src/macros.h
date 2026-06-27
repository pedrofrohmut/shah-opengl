#pragma once

#include <iostream>
#include <format>

/*
  All macros here a with snake case with a trailing underscore in the model off
  foo_bar_, bar_ foo_, foo_bar_baz_, etc.

  This file is macro only so the macros are easy to locate. Constants can be placed
  elsewhere.
*/

/*
  expr is a Stream insertion stream like: "Foo: " << foo << ", Bar: " << baz
*/
#define print_(expr) \
    std::cout << expr

/*
  expr is a Stream insertion stream like: "Foo: " << foo << ", Bar: " << baz
  inserts a \n in the end
*/
#define println_(expr) \
    std::cout << expr << '\n'

/*
  We have __VA_OPT__(,) to cover the case for 0 args
  Normally is just printf(fmt_string, args...)
*/
#define printf_(fmt, ...) \
    std::cout << std::format(fmt __VA_OPT__(,) __VA_ARGS__)

/*
  Debug only logging
*/
#ifdef DEBUG_MODE
    #define debug_print_(expr) \
        print_(expr)

    #define debug_println_(expr) \
        println_(expr)

    #define debug_printf_(fmt, ...) \
        printf_(fmt, ...)
#else
    #define debug_print_(expr)
    #define debug_println_(expr)
    #define debug_printf_(fmt, ...)
#endif

/*
  Set a break point with code
 */
#ifdef WIN32
    #define breakpoint_ __debugBreak();
#else
    #include <signal.h>
    #define breakpoint_ raise(SIGTRAP);
#endif // WIN32

/*
  Wrap opengl functions with clear errors then error check after
*/
#ifdef DEBUG_MODE
    #define glCheck_(x) clearAllGlErrors(); x; checkGlErrorStatus(#x, __LINE__);
#else
    #define glCheck_(x) x;
#endif
