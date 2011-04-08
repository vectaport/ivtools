#ifndef ivtools_string_h
#define ivtools_string_h

#if defined(__cplusplus)

#if defined(sun) && !defined(__svr4__)
extern "C" {
int strcasecmp(const char*, const char*);
int strncasecmp(const char*, const char*,int);
#undef size_t
#define size_t long unsigned int	
}
#endif


#include <ctype.h>

inline char* strlower(char* str) {
  char* s = str;
  while (*s)
    *s++ = tolower(*s);
  return str;
}

#endif


#if defined(__cplusplus)
#include_next <string.h>
#undef NULL
#define NULL 0
#else
#include </usr/include/string.h>
#endif
#include <strings.h>
#endif
