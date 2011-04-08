#ifndef ivtools_string_h
#define ivtools_string_h

#if defined(__cplusplus)

#if defined(sun) && !defined(solaris)
extern "C" {
int strcasecmp(const char*, const char*);
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

#include_next <string.h>
#endif
