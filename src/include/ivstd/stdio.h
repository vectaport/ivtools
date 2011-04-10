#ifndef ivtools_stdio_h
#define ivtools_stdio_h

#include <string.h> /* to work-around RedHat 7.* problem */

#if defined(__cplusplus)
#include_next <stdio.h>
#else
#include </usr/include/stdio.h>
#endif

#if defined(__cplusplus)
#if defined(sun) && !defined(solaris)
extern "C" {
int pclose(FILE*);
}
#endif
#endif

#endif
