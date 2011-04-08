#ifndef ivtools_signal_h
#define ivtools_signal_h


#if defined(__cplusplus)
#if __linux
#undef __need_sig_atomic_t
#undef __need_sigset_t
#endif
#include_next <signal.h>
#undef NULL
#define NULL 0
#else
#include </usr/include/signal.h>
#endif

#endif
