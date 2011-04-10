#ifndef ivtools_signal_h
#define ivtools_signal_h


#if defined(__cplusplus)
#include_next <signal.h>
#undef NULL
#define NULL 0
#else
#include </usr/include/signal.h>
#endif

#endif
