/*
 * ACE-lite -- a small, name-compatible in-tree subset of ACE (issue #147).
 * src/include/ACE-lite/OS.h -- base types and OS-handle definitions shared by
 * the ACE-lite headers.  Declares the same ACE_* names the consumer code uses,
 * so source compiles unchanged against either backend (selected by EXTERN_ACE).
 */

#ifndef _acelite_OS_h
#define _acelite_OS_h

// Like ace/OS.h, centralize the common OS/portability headers so consumer
// source that leaned on ACE pulling these in transitively still compiles.
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

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

// Default host/port the consumer code falls back to (matches ACE's values).
#ifndef ACE_DEFAULT_SERVER_HOST
#define ACE_DEFAULT_SERVER_HOST "localhost"
#endif
#ifndef ACE_DEFAULT_SERVER_PORT_STR
#define ACE_DEFAULT_SERVER_PORT_STR "10002"
#endif

// The slice of ACE_OS ivtools calls (comhandler/aceimport: strncpy; main: atoi).
namespace ACE_OS {
    inline char* strncpy(char* dst, const char* src, size_t n) {
        return ::strncpy(dst, src, n);
    }
    inline int atoi(const char* s) { return ::atoi(s); }
}

#endif /* _acelite_OS_h */
