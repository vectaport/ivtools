#ifndef ivtools_math_h
#define ivtools_math_h

#include_next <math.h>

#if defined(sun) && defined(__svr4__)
#include <nan.h>
#define isinf(n) IsINF(n)
#endif

#endif
