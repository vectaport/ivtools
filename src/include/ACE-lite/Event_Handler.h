/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Event_Handler.h
 * ACE_Event_Handler -- the base class registered with ACE_Reactor.  ivtools'
 * ACE_IO_Handler (the InterViews IOHandler adapter) and, later, ACE_Svc_Handler
 * derive from it.  Only the slice ivtools uses is declared.
 */

#ifndef _acelite_Event_Handler_h
#define _acelite_Event_Handler_h

#include <ACE-lite/OS.h>
#include <ACE-lite/Time_Value.h>

typedef unsigned long ACE_Reactor_Mask;

class ACE_Reactor;

class ACE_Event_Handler {
public:
    // Event masks (the bit values are private to ACE-lite; consumers use the
    // names, e.g. ACE_Event_Handler::READ_MASK).
    enum {
        NULL_MASK   = 0,
        READ_MASK   = (1 << 0),
        WRITE_MASK  = (1 << 1),
        EXCEPT_MASK = (1 << 2),
        TIMER_MASK  = (1 << 3),
        RWE_MASK    = READ_MASK | WRITE_MASK | EXCEPT_MASK,
        ALL_EVENTS_MASK = RWE_MASK | TIMER_MASK,
        DONT_CALL   = (1 << 4)
    };

    virtual ~ACE_Event_Handler();

    // The fd this handler services; ACE_INVALID_HANDLE unless overridden
    // (ACE_Svc_Handler / the acceptor return their socket).
    virtual ACE_HANDLE get_handle() const;
    virtual void set_handle(ACE_HANDLE);

    // Demultiplexing hooks.  Return -1 to have the reactor remove (and close)
    // this handler, 0 to keep it.
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int handle_exception(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    // Return -1 to cancel a (one-shot already-fired) timer's handler.
    virtual int handle_timeout(const ACE_Time_Value& current_time,
                               const void* act = 0);
    // Called by the reactor when the handler is removed.
    virtual int handle_close(ACE_HANDLE fd = ACE_INVALID_HANDLE,
                             ACE_Reactor_Mask mask = ALL_EVENTS_MASK);

    ACE_Reactor* reactor() const { return reactor_; }
    void reactor(ACE_Reactor* r) { reactor_ = r; }

protected:
    ACE_Event_Handler(ACE_Reactor* r = 0) : reactor_(r) {}

    ACE_Reactor* reactor_;
};

#endif /* _acelite_Event_Handler_h */
