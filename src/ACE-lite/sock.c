/*
 * ACE-lite (issue #147).  src/ACE-lite/sock.c
 * (ACE-lite reimplements a subset of the ACE interface; ACE is (c) the DOC
 * group -- see src/ACE-lite/NOTICE for attribution and the relationship.)
 * Implementations of the ACE-lite address and socket classes
 * (ACE_INET_Addr, ACE_SOCK_Stream, ACE_SOCK_Connector, ACE_SOCK_Acceptor).
 * ACE_Time_Value is header-inline; the templates are header-only.
 */

#include <ACE-lite/INET_Addr.h>
#include <ACE-lite/SOCK_Stream.h>
#include <ACE-lite/SOCK_Connector.h>
#include <ACE-lite/SOCK_Acceptor.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* ---------------------------------------------------------------- INET_Addr */

ACE_INET_Addr::ACE_INET_Addr() {
    memset(&inet_addr_, 0, sizeof(inet_addr_));
    inet_addr_.sin_family = AF_INET;
    host_name_[0] = '\0';
}

ACE_INET_Addr::ACE_INET_Addr(u_short port) {
    memset(&inet_addr_, 0, sizeof(inet_addr_));
    inet_addr_.sin_family = AF_INET;
    inet_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_addr_.sin_port = htons(port);
    host_name_[0] = '\0';
}

ACE_INET_Addr::ACE_INET_Addr(u_short port, const char* host) {
    set(port, host);
}

int ACE_INET_Addr::set(u_short port, const char* host) {
    memset(&inet_addr_, 0, sizeof(inet_addr_));
    inet_addr_.sin_family = AF_INET;
    inet_addr_.sin_port = htons(port);
    host_name_[0] = '\0';

    if (host == 0 || *host == '\0') {
        inet_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        return 0;
    }
    // Try dotted-quad first (no resolver round-trip), then a name lookup.
    in_addr_t numeric = inet_addr(host);
    if (numeric != (in_addr_t)-1) {
        inet_addr_.sin_addr.s_addr = numeric;
        return 0;
    }
    struct hostent* he = gethostbyname(host);
    if (he == 0 || he->h_addr_list[0] == 0) {
        return -1;
    }
    memcpy(&inet_addr_.sin_addr, he->h_addr_list[0], he->h_length);
    return 0;
}

const char* ACE_INET_Addr::get_host_name() const {
    if (host_name_[0] != '\0') {
        return host_name_;
    }
    struct hostent* he = gethostbyaddr((const char*)&inet_addr_.sin_addr,
                                       sizeof(inet_addr_.sin_addr), AF_INET);
    if (he != 0 && he->h_name != 0) {
        strncpy(host_name_, he->h_name, sizeof(host_name_) - 1);
        host_name_[sizeof(host_name_) - 1] = '\0';
    } else {
        const char* dotted = inet_ntoa(inet_addr_.sin_addr);
        strncpy(host_name_, dotted ? dotted : "", sizeof(host_name_) - 1);
        host_name_[sizeof(host_name_) - 1] = '\0';
    }
    return host_name_;
}

int ACE_INET_Addr::addr_to_string(char* buffer, size_t size) const {
    const char* dotted = inet_ntoa(inet_addr_.sin_addr);
    int n = snprintf(buffer, size, "%s:%u",
                     dotted ? dotted : "", (unsigned)ntohs(inet_addr_.sin_port));
    return (n < 0 || (size_t)n >= size) ? -1 : 0;
}

u_short ACE_INET_Addr::get_port_number() const {
    return ntohs(inet_addr_.sin_port);
}

/* -------------------------------------------------------------- SOCK_Stream */

ssize_t ACE_SOCK_Stream::recv(void* buf, size_t n) const {
    return ::recv(handle_, buf, n, 0);
}

ssize_t ACE_SOCK_Stream::send(const void* buf, size_t n) const {
    return ::send(handle_, buf, n, 0);
}

int ACE_SOCK_Stream::enable(int value) const {
    int flags = fcntl(handle_, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    return fcntl(handle_, F_SETFL, flags | value);
}

int ACE_SOCK_Stream::get_remote_addr(ACE_INET_Addr& addr) const {
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getpeername(handle_, (sockaddr*)&sin, &len) == -1) {
        return -1;
    }
    addr.set_addr(sin);
    return 0;
}

int ACE_SOCK_Stream::close() {
    if (handle_ == ACE_INVALID_HANDLE) {
        return 0;
    }
    int rc = ::close(handle_);
    handle_ = ACE_INVALID_HANDLE;
    return rc;
}

/* ----------------------------------------------------------- SOCK_Connector */

int ACE_SOCK_Connector::connect(ACE_SOCK_Stream& stream,
                                const ACE_INET_Addr& remote_addr) {
    ACE_HANDLE h = ::socket(AF_INET, SOCK_STREAM, 0);
    if (h == ACE_INVALID_HANDLE) {
        return -1;
    }
    if (::connect(h, remote_addr.get_addr(), remote_addr.get_size()) == -1) {
        ::close(h);
        return -1;
    }
    stream.set_handle(h);
    return 0;
}

/* ------------------------------------------------------------ SOCK_Acceptor */

int ACE_SOCK_Acceptor::open(const ACE_INET_Addr& local_addr, int reuse_addr) {
    ACE_HANDLE h = ::socket(AF_INET, SOCK_STREAM, 0);
    if (h == ACE_INVALID_HANDLE) {
        return -1;
    }
    if (reuse_addr) {
        int one = 1;
        setsockopt(h, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
    if (::bind(h, local_addr.get_addr(), local_addr.get_size()) == -1 ||
        ::listen(h, SOMAXCONN) == -1) {
        ::close(h);
        return -1;
    }
    handle_ = h;
    return 0;
}

int ACE_SOCK_Acceptor::accept(ACE_SOCK_Stream& new_stream,
                              ACE_INET_Addr* remote_addr) const {
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    ACE_HANDLE h = ::accept(handle_, (sockaddr*)&sin, &len);
    if (h == ACE_INVALID_HANDLE) {
        return -1;
    }
    new_stream.set_handle(h);
    if (remote_addr != 0) {
        remote_addr->set_addr(sin);
    }
    return 0;
}

int ACE_SOCK_Acceptor::close() {
    if (handle_ == ACE_INVALID_HANDLE) {
        return 0;
    }
    int rc = ::close(handle_);
    handle_ = ACE_INVALID_HANDLE;
    return rc;
}
