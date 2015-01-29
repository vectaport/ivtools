#ifndef ivtools_math_h
#define ivtools_math_h

#if defined(__cplusplus)
#include_next <math.h>
#else
#if defined(__APPLE__)
#define __DARWIN_UNIX03 1
#endif
#include </usr/include/math.h>
#endif

#if defined(sun) && defined(__svr4__)
#include <nan.h>
#define isinf(n) IsINF(n)
#endif

#endif
