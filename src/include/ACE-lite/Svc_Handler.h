/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Svc_Handler.h
 * ACE_Svc_Handler<PEER_STREAM, SYNCH> -- the per-connection handler base that
 * ivtools' ComterpHandler and UnidrawImportHandler derive from.  It is an
 * ACE_Event_Handler holding a connected peer stream; the reactor dispatches its
 * handle_input, and on removal routes handle_close -> close() -> destroy()
 * (the lifecycle ComterpHandler overrides).
 */

#ifndef _acelite_Svc_Handler_h
#define _acelite_Svc_Handler_h

#include <ACE-lite/Event_Handler.h>
#include <ACE-lite/Reactor.h>
#include <ACE-lite/Synch.h>
#include <sys/types.h>

template <class PEER_STREAM, class SYNCH_STRATEGY>
class ACE_Svc_Handler : public ACE_Event_Handler {
public:
    // ACE's signature is (thread_manager, message_queue, reactor); ivtools only
    // ever passes the reactor (or nothing).  The first two are accepted and
    // ignored.  Serves as the default ctor too.
    ACE_Svc_Handler(void* = 0, void* = 0, ACE_Reactor* r = 0)
        : ACE_Event_Handler(r) {}
    virtual ~ACE_Svc_Handler() {}

    // The connected socket.
    PEER_STREAM& peer() { return peer_; }

    virtual ACE_HANDLE get_handle() const { return peer_.get_handle(); }
    virtual void set_handle(ACE_HANDLE h) { peer_.set_handle(h); }

    // Hooks subclasses override.
    virtual int open(void* = 0) { return 0; }
    virtual int close(u_long /*flags*/ = 0) { this->destroy(); return 0; }

    // ACE_Svc_Handler::destroy: unregister and delete.  ComterpHandler overrides
    // this with its own teardown (and deliberately does not delete itself), so
    // for it this default is not used.
    virtual void destroy() {
        if (this->reactor()) {
            this->reactor()->remove_handler(
                this->get_handle(),
                ACE_Event_Handler::RWE_MASK | ACE_Event_Handler::DONT_CALL);
        }
        delete this;
    }

    // The reactor calls this when retiring the handler (e.g. handle_input
    // returned -1); route it to the close()/destroy() lifecycle.
    virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE,
                             ACE_Reactor_Mask mask =
                                 ACE_Event_Handler::ALL_EVENTS_MASK) {
        return this->close((u_long)mask);
    }

protected:
    PEER_STREAM peer_;
};

#endif /* _acelite_Svc_Handler_h */
