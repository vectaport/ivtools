#ifndef _iv_fstream_
#define _iv_fstream_
#include_next <fstream.h>
#if __GNUG__>=3
#define input ios_base::in
#define output ios_base::out
#else
#define input "r"
#define output "w"
#endif
#endif
