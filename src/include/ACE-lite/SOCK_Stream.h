/*
 * ACE-lite (issue #147).  src/include/ACE-lite/SOCK_Stream.h
 * ACE_SOCK_Stream -- a connected-TCP file-descriptor wrapper.  The consumer
 * code mostly reads/writes the raw get_handle(), but recv/send/close/enable/
 * get_remote_addr are provided to match ACE.
 */

#ifndef _acelite_SOCK_Stream_h
#define _acelite_SOCK_Stream_h

#include <ACE-lite/OS.h>
#include <ACE-lite/INET_Addr.h>
#include <stddef.h>

class ACE_SOCK_Stream {
public:
    ACE_SOCK_Stream() : handle_(ACE_INVALID_HANDLE) {}
    ACE_SOCK_Stream(ACE_HANDLE h) : handle_(h) {}

    ACE_HANDLE get_handle() const { return handle_; }
    void set_handle(ACE_HANDLE h) { handle_ = h; }

    ssize_t recv(void* buf, size_t n) const;
    ssize_t send(const void* buf, size_t n) const;

    // Apply a flag to the fd (only ACE_NONBLOCK is used); 0 ok, -1 on error.
    int enable(int value) const;

    int get_remote_addr(ACE_INET_Addr& addr) const;

    int close();

private:
    ACE_HANDLE handle_;
};

// ACE spells the peer-stream template argument this way.
#ifndef ACE_SOCK_STREAM
#define ACE_SOCK_STREAM ACE_SOCK_Stream
#endif

#endif /* _acelite_SOCK_Stream_h */
