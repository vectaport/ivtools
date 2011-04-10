#ifndef _iv_fstream_
#define _iv_fstream_
#include_next <fstream.h>
#if __GNUC__>=3
#define input ios_base::in
#define output ios_base::out
#if __GNUC__>3 || __GNUC__==3 && __GNUC_MINOR__>0 /* && !defined(__APPLE__) */
#include <ext/stdio_filebuf.h>
#define fileptr_filebuf __gnu_cxx::stdio_filebuf<char>
#else
#define fileptr_filebuf filebuf
#endif
#else
#define input "r"
#define output "w"
#endif
#endif
