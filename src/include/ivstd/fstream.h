#ifndef _iv_fstream_
#define _iv_fstream_
#include <fstream>
#include <cstdio>
#define input std::ios::in
#define output std::ios::out
#if !defined(__APPLE_CC__) || __APPLE_CC__==1
#include <ext/stdio_filebuf.h>
#define fileptr_filebuf __gnu_cxx::stdio_filebuf<char>
#else
#define fileptr_filebuf filebuf
#endif
#if !defined(__APPLE_CC__) || __APPLE_CC__==1
#define FILEBUF(bufname, fptr, mode)		\
     fileptr_filebuf bufname(fptr, mode)
#define FILEBUFP(bufname, fptr, mode)	\
     fileptr_filebuf* bufname = new fileptr_filebuf(fptr, mode)
#else
#define FILEBUF(bufname, fptr, mode)		\
     fileptr_filebuf bufname; \
     bufname.__open(fileno(fptr), mode); 
#define FILEBUFP(bufname, fptr, mode) \
     fileptr_filebuf* bufname = new fileptr_filebuf; \
     bufname -> __open(fileno(fptr),mode);
#endif

#endif
