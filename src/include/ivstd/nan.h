#ifndef ivtools_nan_h
#define ivtools_nan_h

#if defined(__GLIBC__) && (__GLIBC__==2 && __GLIBC_MINOR__>0 || __GLIBC__>2)
#include <bits/nan.h>
#else
#include_next <nan.h>
#endif

#endif
