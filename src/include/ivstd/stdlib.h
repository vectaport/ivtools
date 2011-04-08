#ifndef _ivstd_stdlib
#define _ivstd_stdlib
#if defined(__cplusplus)
#include_next <stdlib.h>
#else
#include </usr/include/stdlib.h>
#endif
#if defined(__APPLE__)
typedef int socklen_t;
#endif
#endif
