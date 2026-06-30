/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Acceptor.h
 * ACE_Acceptor<HANDLER, PEER_ACCEPTOR> -- the accept-and-spawn template.  It is
 * itself an ACE_Event_Handler registered on the listen fd; when the fd is
 * readable it accept()s a connection, makes a HANDLER, hands it the new peer
 * stream, and calls handler->open() -- the core thing the base Dispatcher could
 * not do by itself.
 *
 * ivtools spells it: typedef ACE_Acceptor<ComterpHandler, ACE_SOCK_ACCEPTOR>
 * ComterpAcceptor;  with open(addr, reactor) in main.c.
 */

#ifndef _acelite_Acceptor_h
#define _acelite_Acceptor_h

#include <ACE-lite/Event_Handler.h>
#include <ACE-lite/Reactor.h>
#include <ACE-lite/INET_Addr.h>

template <class HANDLER, class PEER_ACCEPTOR>
class ACE_Acceptor : public ACE_Event_Handler {
public:
    ACE_Acceptor(ACE_Reactor* r = 0) : ACE_Event_Handler(r) {}
    virtual ~ACE_Acceptor() {}

    // Open the listen socket on `local_addr' and register for READ on `r'
    // (defaulting to the global reactor, as ACE does, so callers may omit it).
    // 0 on success, -1 on failure.
    int open(const ACE_INET_Addr& local_addr,
             ACE_Reactor* r = ACE_Reactor::instance()) {
        this->reactor(r);
        if (peer_acceptor_.open(local_addr) == -1) {
            return -1;
        }
        if (r && r->register_handler(this, ACE_Event_Handler::READ_MASK) == -1) {
            return -1;
        }
        return 0;
    }

    virtual ACE_HANDLE get_handle() const { return peer_acceptor_.get_handle(); }

    // Listen fd is readable: accept and spawn a handler for the new connection.
    virtual int handle_input(ACE_HANDLE) {
        HANDLER* handler = new HANDLER;
        if (peer_acceptor_.accept(handler->peer()) == -1) {
            delete handler;
            return 0;  // transient accept failure; keep listening
        }
        handler->reactor(this->reactor());
        if (handler->open((void*)this) == -1) {
            handler->close(0);
        }
        return 0;  // keep the acceptor registered
    }

protected:
    PEER_ACCEPTOR peer_acceptor_;
};

#endif /* _acelite_Acceptor_h */
