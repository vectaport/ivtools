/*
 * ACE-lite (issue #147).  src/include/ACE-lite/SOCK_Connector.h
 * ACE_SOCK_Connector -- actively opens a TCP connection (socket()+connect())
 * and hands the connected fd to an ACE_SOCK_Stream.
 */

#ifndef _acelite_SOCK_Connector_h
#define _acelite_SOCK_Connector_h

#include <ACE-lite/SOCK_Stream.h>
#include <ACE-lite/INET_Addr.h>

class ACE_SOCK_Connector {
public:
    ACE_SOCK_Connector() {}

    // Connect `stream' to `remote_addr'; 0 on success, -1 on failure.
    int connect(ACE_SOCK_Stream& stream, const ACE_INET_Addr& remote_addr);
};

#endif /* _acelite_SOCK_Connector_h */
