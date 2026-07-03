/*
 * ACE-lite (issue #147).  src/ACE-lite/tests/sock_loopback.cc
 * Standalone Phase-1 unit test for the ACE-lite address/socket layer -- no
 * reactor.  Binds a loopback acceptor on an OS-chosen port, connects to it,
 * round-trips a message both directions, and checks get_remote_addr /
 * addr_to_string / Time_Value arithmetic.  Build:
 *   g++ -I src/include src/ACE-lite/sock.c src/ACE-lite/tests/sock_loopback.cc
 * Exit 0 = pass, 1 = fail.
 */

#include <ACE-lite/INET_Addr.h>
#include <ACE-lite/SOCK_Stream.h>
#include <ACE-lite/SOCK_Connector.h>
#include <ACE-lite/SOCK_Acceptor.h>
#include <ACE-lite/Time_Value.h>

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static int failures = 0;

static void check(int cond, const char* what) {
    printf("%s: %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

// Discover the port a wildcard-bound acceptor actually got.
static u_short bound_port(ACE_HANDLE h) {
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(h, (sockaddr*)&sin, &len) == -1) return 0;
    return ntohs(sin.sin_port);
}

int main() {
    // --- Time_Value arithmetic (header-inline) ---
    ACE_Time_Value a(2, 500000), b(1, 750000);
    ACE_Time_Value d = a - b;
    check(d.sec() == 0 && d.usec() == 750000, "ACE_Time_Value(2.5)-(1.75)==0.75");
    check(a > b, "ACE_Time_Value(2.5) > (1.75)");
    ACE_Time_Value carry(0, 1500000);
    check(carry.sec() == 1 && carry.usec() == 500000, "ACE_Time_Value normalizes 1.5e6 usec");

    // --- bring up a loopback acceptor on an OS-chosen port ---
    ACE_SOCK_Acceptor acceptor;
    check(acceptor.open(ACE_INET_Addr((u_short)0)) == 0, "ACE_SOCK_Acceptor::open(port 0)");
    u_short port = bound_port(acceptor.get_handle());
    check(port != 0, "acceptor bound a nonzero port");

    // --- connect to it (kernel completes the handshake into the backlog) ---
    ACE_SOCK_Connector connector;
    ACE_SOCK_Stream client;
    check(connector.connect(client, ACE_INET_Addr(port, "127.0.0.1")) == 0,
          "ACE_SOCK_Connector::connect(127.0.0.1)");

    ACE_SOCK_Stream server;
    check(acceptor.accept(server) == 0, "ACE_SOCK_Acceptor::accept");

    // --- client -> server ---
    const char* msg = "hello ace-lite";
    check(client.send(msg, strlen(msg)) == (ssize_t)strlen(msg), "client.send");
    char buf[64];
    ssize_t n = server.recv(buf, sizeof(buf) - 1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    check(n == (ssize_t)strlen(msg) && strcmp(buf, msg) == 0, "server.recv == sent");

    // --- server -> client ---
    const char* reply = "333";
    check(server.send(reply, strlen(reply)) == (ssize_t)strlen(reply), "server.send");
    n = client.recv(buf, sizeof(buf) - 1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    check(strcmp(buf, reply) == 0, "client.recv == reply");

    // --- remote addr / addr_to_string ---
    ACE_INET_Addr peer;
    check(server.get_remote_addr(peer) == 0, "server.get_remote_addr");
    char addrbuf[64];
    check(peer.addr_to_string(addrbuf, sizeof(addrbuf)) == 0, "peer.addr_to_string");
    check(strncmp(addrbuf, "127.0.0.1:", 10) == 0, "peer addr is 127.0.0.1:<port>");

    client.close();
    server.close();
    acceptor.close();

    printf("\nsock_loopback: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
