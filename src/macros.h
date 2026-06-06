#pragma once

#ifdef DEBUG_MODE
  #define DEBUG(expr) std::cout << expr
#else
  #define DEBUG(expr)
#endif

#define println_(expr) std::cout << expr << '\n'
