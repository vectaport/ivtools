/*
 * ACE-lite (issue #147).  src/ACE-lite/tests/acceptor_spawn.cc
 * Standalone Phase-3 unit test for ACE_Acceptor + ACE_Svc_Handler, exercising
 * the accept-and-spawn path and the EOF retirement lifecycle
 * (handle_input -1 => reactor retire => handle_close => close => destroy)
 * exactly as ComterpAcceptor/ComterpHandler use it.
 * Build:
 *   g++ -I src/include src/ACE-lite/{reactor,event_handler,sock,log_msg}.c \
 *       src/ACE-lite/tests/acceptor_spawn.cc
 * Exit 0 = pass, 1 = fail.
 */

#include <ACE-lite/Acceptor.h>
#include <ACE-lite/Svc_Handler.h>
#include <ACE-lite/SOCK_Acceptor.h>
#include <ACE-lite/SOCK_Connector.h>
#include <ACE-lite/SOCK_Stream.h>
#include <ACE-lite/Synch.h>
#include <ACE-lite/Reactor.h>

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int failures = 0;
static void check(int cond, const char* what) {
    printf("%s: %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

// An echo handler that registers itself on open() and self-retires on EOF, like
// a real socket service handler.  Uses the default close()/destroy() (which
// deletes it), so live_count tracks construction vs destruction.
class EchoHandler : public ACE_Svc_Handler<ACE_SOCK_Stream, ACE_NULL_SYNCH> {
public:
    static int live_count;
    static int opened;
    EchoHandler() { live_count++; }
    virtual ~EchoHandler() { live_count--; }

    virtual int open(void*) {
        opened++;
        if (this->reactor()->register_handler(this, ACE_Event_Handler::READ_MASK) == -1)
            return -1;
        return 0;
    }
    virtual int handle_input(ACE_HANDLE) {
        char buf[64];
        ssize_t n = peer().recv(buf, sizeof(buf));
        if (n <= 0) return -1;     // EOF -> reactor retires us
        peer().send(buf, n);       // echo
        return 0;
    }
};
int EchoHandler::live_count = 0;
int EchoHandler::opened = 0;

int main() {
    ACE_Reactor reactor;
    ACE_Acceptor<EchoHandler, ACE_SOCK_ACCEPTOR> acceptor;

    check(acceptor.open(ACE_INET_Addr((u_short)0), &reactor) == 0,
          "ACE_Acceptor::open(addr, reactor)");

    // Discover the bound port.
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(acceptor.get_handle(), (sockaddr*)&sin, &len);
    u_short port = ntohs(sin.sin_port);

    // Connect a client; one reactor turn should accept + spawn a handler.
    ACE_SOCK_Connector connector;
    ACE_SOCK_Stream client;
    check(connector.connect(client, ACE_INET_Addr(port, "127.0.0.1")) == 0,
          "client connect to acceptor");
    ACE_Time_Value w(1, 0);
    reactor.handle_events(&w);
    check(EchoHandler::opened == 1 && EchoHandler::live_count == 1,
          "acceptor accept-and-spawned one handler (open called)");

    // Round-trip through the spawned handler.
    client.send("ping", 4);
    ACE_Time_Value w2(1, 0);
    reactor.handle_events(&w2);
    char buf[64];
    ssize_t n = client.recv(buf, sizeof(buf) - 1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    check(strcmp(buf, "ping") == 0, "spawned handler echoed the payload");

    // Close client -> handler hits EOF -> -1 -> retire -> close -> destroy.
    client.close();
    ACE_Time_Value w3(1, 0);
    reactor.handle_events(&w3);
    check(EchoHandler::live_count == 0,
          "EOF retired the handler through handle_close->close->destroy");

    printf("\nacceptor_spawn: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
