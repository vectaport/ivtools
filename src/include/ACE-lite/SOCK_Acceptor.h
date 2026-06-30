/*
 * ACE-lite (issue #147).  src/include/ACE-lite/SOCK_Acceptor.h
 * ACE_SOCK_Acceptor -- the low-level passive socket: socket()/bind()/listen()
 * and accept() of a new connection into an ACE_SOCK_Stream.  (The higher-level
 * ACE_Acceptor<Handler> accept-and-spawn template is built on this later.)
 */

#ifndef _acelite_SOCK_Acceptor_h
#define _acelite_SOCK_Acceptor_h

#include <ACE-lite/OS.h>
#include <ACE-lite/SOCK_Stream.h>
#include <ACE-lite/INET_Addr.h>

class ACE_SOCK_Acceptor {
public:
    ACE_SOCK_Acceptor() : handle_(ACE_INVALID_HANDLE) {}
    ACE_SOCK_Acceptor(const ACE_INET_Addr& local_addr, int reuse_addr = 1)
        : handle_(ACE_INVALID_HANDLE) { open(local_addr, reuse_addr); }

    // socket()/bind()/listen() on `local_addr'; 0 on success, -1 on failure.
    int open(const ACE_INET_Addr& local_addr, int reuse_addr = 1);

    // accept() a pending connection into `new_stream'; fill `remote_addr' if
    // non-null.  0 on success, -1 on failure.
    int accept(ACE_SOCK_Stream& new_stream, ACE_INET_Addr* remote_addr = 0) const;

    ACE_HANDLE get_handle() const { return handle_; }
    int close();

private:
    ACE_HANDLE handle_;
};

// ACE spells the acceptor template argument this way.
#ifndef ACE_SOCK_ACCEPTOR
#define ACE_SOCK_ACCEPTOR ACE_SOCK_Acceptor
#endif

#endif /* _acelite_SOCK_Acceptor_h */
