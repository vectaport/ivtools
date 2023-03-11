#ifndef ivtools_nan_h
#define ivtools_nan_h

#if defined(__GLIBC__) && (__GLIBC__==2 && __GLIBC_MINOR__>0 || __GLIBC__>2)
#include <math.h>
#elif !defined(__CYGWIN__) && !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__APPLE__)
#include_next <nan.h>
#else
#include <math.h>
#endif
#if defined(__sun__) && defined(__svr4__)
#include <ieeefp.h>
#endif

#if defined(__sun__) && defined(__svr4__) || defined(__CYGWIN__) || defined(__linux__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__APPLE__)
#define isnanorinf(dval) (dval==NAN || dval==INFINITY )
#elif defined(__alpha)
#define isnanorinf(dval) (IsNANorINF(dval))
#else
#define isnanorinf(dval) (dval==NAN || dval==INFINITY )
#endif



#endif
