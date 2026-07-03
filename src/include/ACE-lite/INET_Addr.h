/*
 * ACE-lite (issue #147).  src/include/ACE-lite/INET_Addr.h
 * ACE_INET_Addr -- an IPv4 host/port address wrapping sockaddr_in, the subset
 * of ACE's class the socket classes and consumer code use.
 */

#ifndef _acelite_INET_Addr_h
#define _acelite_INET_Addr_h

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

class ACE_INET_Addr {
public:
    ACE_INET_Addr();
    // Wildcard local address on `port' (INADDR_ANY), for binding a listener.
    ACE_INET_Addr(u_short port);
    // `port' on `host' (name or dotted-quad), for connecting.
    ACE_INET_Addr(u_short port, const char* host);

    int set(u_short port, const char* host);

    // Reverse-resolved host name (falls back to dotted-quad on failure).
    const char* get_host_name() const;
    // "host:port" into `buffer'; returns 0 on success, -1 if truncated.
    int addr_to_string(char* buffer, size_t size) const;

    u_short get_port_number() const;

    // Raw access for the socket classes (bind/connect/accept).
    sockaddr* get_addr() const { return (sockaddr*)&inet_addr_; }
    int get_size() const { return sizeof(inet_addr_); }
    void set_addr(const sockaddr_in& sin) { inet_addr_ = sin; }

private:
    sockaddr_in inet_addr_;
    mutable char host_name_[256];
};

#endif /* _acelite_INET_Addr_h */
