/*
 * ACE-lite -- a small, name-compatible in-tree subset of ACE (issue #147).
 * src/include/ACE-lite/OS.h -- base types and OS-handle definitions shared by
 * the ACE-lite headers.  Declares the same ACE_* names the consumer code uses,
 * so source compiles unchanged against either backend (selected by EXTERN_ACE).
 */

#ifndef _acelite_OS_h
#define _acelite_OS_h

#include <fcntl.h>
#include <sys/types.h>

// A handle is a POSIX file descriptor.
typedef int ACE_HANDLE;

// The invalid-handle sentinel (matches ACE).
#ifndef ACE_INVALID_HANDLE
#define ACE_INVALID_HANDLE (-1)
#endif

// Bit set passed to ACE_SOCK_Stream::enable(); only non-blocking is used.
#ifndef ACE_NONBLOCK
#define ACE_NONBLOCK O_NONBLOCK
#endif

#endif /* _acelite_OS_h */
