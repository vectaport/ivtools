#ifndef ivtools_stdio_h
#define ivtools_stdio_h

#include_next <stdio.h>

#if defined(__cplusplus)
#if defined(sun) && !defined(solaris)
extern "C" {
int pclose(FILE*);
}
#endif
#endif

#endif
