/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Reactor.h
 * ACE_Reactor -- a self-contained select()-based event demultiplexer, the
 * drop-in for libACE's reactor.  It sits in the exact same slot: AceDispatcher
 * forwards the InterViews Dispatcher's attach/dispatch into it, and socket
 * handlers (ComterpHandler) register with it directly, so GUI and socket fds
 * are serviced by one select loop -- as ivtools has always spliced them.
 *
 * Only the API ivtools calls is provided (see AceDispatcher and comhandler):
 * register_handler / remove_handler / schedule_timer / cancel_timer /
 * handle_events.
 */

#ifndef _acelite_Reactor_h
#define _acelite_Reactor_h

#include <ACE-lite/OS.h>
#include <ACE-lite/Event_Handler.h>
#include <ACE-lite/Time_Value.h>
#include <map>
#include <vector>

class ACE_Reactor {
public:
    ACE_Reactor();
    ~ACE_Reactor();

    // Register `eh' for `mask' events.  The first form uses eh->get_handle().
    int register_handler(ACE_Event_Handler* eh, ACE_Reactor_Mask mask);
    int register_handler(ACE_HANDLE handle, ACE_Event_Handler* eh,
                         ACE_Reactor_Mask mask);

    // Stop dispatching `handle' for the events in `mask'.
    int remove_handler(ACE_HANDLE handle, ACE_Reactor_Mask mask);

    // Schedule `eh' to fire handle_timeout after `delay' (and every `interval'
    // thereafter, if nonzero).  Returns a timer id (>=0), or -1 on failure.
    int schedule_timer(ACE_Event_Handler* eh, const void* arg,
                       const ACE_Time_Value& delay,
                       const ACE_Time_Value& interval = ACE_Time_Value());
    // Cancel by timer id, or every timer of a handler.  Returns # cancelled.
    int cancel_timer(int timer_id, const void** arg = 0);
    int cancel_timer(ACE_Event_Handler* eh);

    // Wait once for and dispatch ready events / expired timers.  Blocks until
    // something happens (or `max_wait_time', if given, elapses).  Returns the
    // number of handlers dispatched, 0 on timeout, -1 on error.
    int handle_events();
    int handle_events(ACE_Time_Value* max_wait_time);
    int handle_events(ACE_Time_Value& max_wait_time);

private:
    struct Timer {
        int id;
        ACE_Event_Handler* eh;
        const void* arg;
        ACE_Time_Value expire;    // absolute fire time
        ACE_Time_Value interval;  // 0 => one-shot
    };

    void bind(ACE_HANDLE, ACE_Event_Handler*, ACE_Reactor_Mask);
    // Remove from all masks and call handle_close (the -1/EOF retirement path).
    void retire(ACE_HANDLE, ACE_Reactor_Mask mask);
    // Soonest timer expiry into `tv'; returns false if no timers pending.
    bool earliest_timer(ACE_Time_Value& tv) const;
    int expire_timers();

    std::map<ACE_HANDLE, ACE_Event_Handler*> read_;
    std::map<ACE_HANDLE, ACE_Event_Handler*> write_;
    std::map<ACE_HANDLE, ACE_Event_Handler*> except_;
    std::vector<Timer> timers_;
    int next_timer_id_;
};

#endif /* _acelite_Reactor_h */
