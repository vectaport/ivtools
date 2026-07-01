/*
 * ACE-lite (issue #147).  src/ACE-lite/reactor.c
 * (ACE-lite reimplements a subset of the ACE interface; ACE is (c) the DOC
 * group -- see src/ACE-lite/NOTICE.)
 *
 * ACE_Reactor -- a self-contained select() demultiplexer that drops into the
 * same slot as libACE's reactor (AceDispatcher forwards into it; socket
 * handlers register with it).
 *
 * handle_events() is written to survive reentrancy: a handler can call
 * update() -> handle_events() recursively, and a callback (or that nested
 * call) can remove handlers out from under an in-progress dispatch.  So we
 * snapshot the ready set, re-verify each handler is still registered before
 * invoking it, and defer removals -- mirroring the null-checked tables the base
 * InterViews Dispatcher uses for the same reason.
 */

#include <ACE-lite/Reactor.h>

#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <set>
#include <vector>

#ifndef NSIG
#define NSIG 64
#endif

// ACE_Time_Value::zero -- the shared 0 duration declared in Time_Value.h.
const ACE_Time_Value ACE_Time_Value::zero;

// Signal handlers are process-global, so the signum->handler table and the
// trampoline are file-static.  handle_signal must be async-signal-safe (the
// only registered use, ACE_Test_and_Set, just sets a sig_atomic_t flag).
static ACE_Event_Handler* acelite_sig_handlers[NSIG];

extern "C" void acelite_sig_trampoline(int signo) {
    if (signo >= 0 && signo < NSIG && acelite_sig_handlers[signo]) {
        acelite_sig_handlers[signo]->handle_signal(signo);
    }
}

ACE_Reactor::ACE_Reactor() : next_timer_id_(1) {}

ACE_Reactor::~ACE_Reactor() {}

/* ----------------------------------------------------------- registration */

void ACE_Reactor::bind(ACE_HANDLE h, ACE_Event_Handler* eh,
                       ACE_Reactor_Mask mask) {
    if (mask & ACE_Event_Handler::READ_MASK)   read_[h] = eh;
    if (mask & ACE_Event_Handler::WRITE_MASK)  write_[h] = eh;
    if (mask & ACE_Event_Handler::EXCEPT_MASK) except_[h] = eh;
    if (eh) eh->reactor(this);
}

int ACE_Reactor::register_handler(ACE_Event_Handler* eh,
                                  ACE_Reactor_Mask mask) {
    if (eh == 0) return -1;
    ACE_HANDLE h = eh->get_handle();
    if (h == ACE_INVALID_HANDLE) return -1;
    bind(h, eh, mask);
    return 0;
}

int ACE_Reactor::register_handler(ACE_HANDLE handle, ACE_Event_Handler* eh,
                                  ACE_Reactor_Mask mask) {
    if (handle == ACE_INVALID_HANDLE || eh == 0) return -1;  // never bind a null
    bind(handle, eh, mask);
    return 0;
}

int ACE_Reactor::remove_handler(ACE_HANDLE handle, ACE_Reactor_Mask mask) {
    if (mask & ACE_Event_Handler::READ_MASK)   read_.erase(handle);
    if (mask & ACE_Event_Handler::WRITE_MASK)  write_.erase(handle);
    if (mask & ACE_Event_Handler::EXCEPT_MASK) except_.erase(handle);
    return 0;
}

int ACE_Reactor::remove_handler(ACE_Event_Handler* eh, ACE_Reactor_Mask mask) {
    if (eh == 0) return -1;
    return remove_handler(eh->get_handle(), mask);
}

int ACE_Reactor::register_handler(int signum, ACE_Event_Handler* new_sh) {
    if (signum < 0 || signum >= NSIG || new_sh == 0) {
        return -1;
    }
    new_sh->reactor(this);
    acelite_sig_handlers[signum] = new_sh;
    return ::signal(signum, acelite_sig_trampoline) == SIG_ERR ? -1 : 0;
}

// Retire a handle on the -1/EOF path: drop every mask and notify the handler
// so it can close its socket (ACE_Svc_Handler::handle_close does this).
void ACE_Reactor::retire(ACE_HANDLE h, ACE_Reactor_Mask mask) {
    std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = read_.find(h);
    ACE_Event_Handler* eh = (it != read_.end()) ? it->second : 0;
    if (!eh) { it = write_.find(h);  if (it != write_.end())  eh = it->second; }
    if (!eh) { it = except_.find(h); if (it != except_.end()) eh = it->second; }
    remove_handler(h, ACE_Event_Handler::RWE_MASK);
    if (eh && !(mask & ACE_Event_Handler::DONT_CALL)) {
        eh->handle_close(h, mask);
    }
}

/* ---------------------------------------------------------------- timers */

int ACE_Reactor::schedule_timer(ACE_Event_Handler* eh, const void* arg,
                                const ACE_Time_Value& delay,
                                const ACE_Time_Value& interval) {
    if (eh == 0) return -1;
    timeval now;
    gettimeofday(&now, 0);
    Timer t;
    t.id = next_timer_id_++;
    t.eh = eh;
    t.arg = arg;
    t.expire = ACE_Time_Value(now) + delay;
    t.interval = interval;
    eh->reactor(this);
    timers_.push_back(t);
    return t.id;
}

int ACE_Reactor::cancel_timer(int timer_id, const void** arg) {
    for (std::vector<Timer>::iterator it = timers_.begin();
         it != timers_.end(); ++it) {
        if (it->id == timer_id) {
            if (arg) *arg = it->arg;
            timers_.erase(it);
            return 1;
        }
    }
    return 0;
}

int ACE_Reactor::cancel_timer(ACE_Event_Handler* eh) {
    int n = 0;
    for (std::vector<Timer>::iterator it = timers_.begin();
         it != timers_.end(); ) {
        if (it->eh == eh) { it = timers_.erase(it); n++; }
        else ++it;
    }
    return n;
}

bool ACE_Reactor::earliest_timer(ACE_Time_Value& tv) const {
    if (timers_.empty()) return false;
    tv = timers_[0].expire;
    for (size_t i = 1; i < timers_.size(); i++) {
        if (timers_[i].expire < tv) tv = timers_[i].expire;
    }
    return true;
}

// Fire every timer whose expiry has passed.  Snapshot first: a callback may
// schedule/cancel timers (directly or via a reentrant handle_events).
int ACE_Reactor::expire_timers() {
    timeval nowtv;
    gettimeofday(&nowtv, 0);
    ACE_Time_Value now(nowtv);

    std::vector<int> due;
    for (size_t i = 0; i < timers_.size(); i++) {
        if (!(timers_[i].expire > now)) due.push_back(timers_[i].id);
    }

    int fired = 0;
    for (size_t k = 0; k < due.size(); k++) {
        // Re-find by id each time: the vector may have been mutated.
        int idx = -1;
        for (size_t i = 0; i < timers_.size(); i++) {
            if (timers_[i].id == due[k]) { idx = (int)i; break; }
        }
        if (idx < 0) continue;  // already cancelled/fired by an earlier callback
        ACE_Event_Handler* eh = timers_[idx].eh;
        const void* arg = timers_[idx].arg;
        ACE_Time_Value interval = timers_[idx].interval;
        int id = timers_[idx].id;
        bool oneshot = (interval.sec() == 0 && interval.usec() == 0);

        // Retire/reschedule BEFORE firing.  handle_timeout can re-enter
        // handle_events() (e.g. via update()), and the nested expire_timers()
        // would otherwise still see this timer as due and fire it again --
        // unbounded recursion.  A one-shot is removed; an interval timer is
        // pushed to its next expiry (so the reentrant pass sees it in the
        // future).
        if (oneshot) {
            timers_.erase(timers_.begin() + idx);
        } else {
            timers_[idx].expire = now + interval;
        }

        int rc = eh->handle_timeout(now, arg);
        fired++;

        // An interval timer whose handler asked to stop (rc < 0) is cancelled
        // now (re-find: the callback may already have removed it).
        if (!oneshot && rc < 0) {
            for (size_t i = 0; i < timers_.size(); i++) {
                if (timers_[i].id == id) { timers_.erase(timers_.begin() + i); break; }
            }
        }
    }
    return fired;
}

/* ------------------------------------------------------------ event loop */

int ACE_Reactor::handle_events() { return handle_events((ACE_Time_Value*)0); }

int ACE_Reactor::handle_events(ACE_Time_Value& max_wait_time) {
    // The reference form decrements the caller's budget by the time actually
    // spent, as real ACE does, so a loop like `while (tv > zero)
    // handle_events(tv)` terminates instead of spinning.
    timeval before;
    gettimeofday(&before, 0);
    int n = handle_events(&max_wait_time);
    timeval after;
    gettimeofday(&after, 0);
    ACE_Time_Value elapsed = ACE_Time_Value(after) - ACE_Time_Value(before);
    if (elapsed >= max_wait_time) max_wait_time = ACE_Time_Value::zero;
    else max_wait_time -= elapsed;
    return n;
}

int ACE_Reactor::handle_events(ACE_Time_Value* max_wait_time) {
    fd_set rset, wset, eset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&eset);

    // FD_SET/FD_ISSET on a descriptor >= FD_SETSIZE indexes past the fixed-size
    // fd_set -- undefined behavior (stack corruption).  Guard every use: a fd
    // that large is skipped rather than corrupting memory.  ivtools' workloads
    // keep fd numbers well under FD_SETSIZE; lifting the limit entirely means
    // moving this select() loop to poll() (no FD_SETSIZE bound), noted as future
    // work in issue #147.
    int maxfd = -1;
    for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = read_.begin();
         it != read_.end(); ++it) {
        if (it->first < 0 || it->first >= FD_SETSIZE) continue;
        FD_SET(it->first, &rset);
        if (it->first > maxfd) maxfd = it->first;
    }
    for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = write_.begin();
         it != write_.end(); ++it) {
        if (it->first < 0 || it->first >= FD_SETSIZE) continue;
        FD_SET(it->first, &wset);
        if (it->first > maxfd) maxfd = it->first;
    }
    for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = except_.begin();
         it != except_.end(); ++it) {
        if (it->first < 0 || it->first >= FD_SETSIZE) continue;
        FD_SET(it->first, &eset);
        if (it->first > maxfd) maxfd = it->first;
    }

    // Bound the wait by the soonest timer and the caller's cap (smaller wins).
    timeval tvbuf;
    timeval* tvp = 0;
    ACE_Time_Value timer_at, now_tv;
    bool have_timer = earliest_timer(timer_at);
    if (have_timer) {
        timeval nowtv;
        gettimeofday(&nowtv, 0);
        ACE_Time_Value wait = timer_at - ACE_Time_Value(nowtv);
        // Clamp any non-positive wait to zero.  A timer that's already due (or
        // fired a few usec late) gives wait <= 0, often as (sec=0, usec<0) -- a
        // negative tv_usec makes select() fail with EINVAL, which would drop the
        // overdue timer.  `<= zero` catches the (0, usec<0) case that a plain
        // `sec() < 0` test misses; a 0 timeout makes select() return at once so
        // expire_timers() runs.
        if (wait <= ACE_Time_Value::zero) wait.set(0, 0);
        tvbuf = (timeval)wait;
        tvp = &tvbuf;
    }
    if (max_wait_time) {
        timeval cap = (timeval) * max_wait_time;
        if (tvp == 0 || cap.tv_sec < tvbuf.tv_sec ||
            (cap.tv_sec == tvbuf.tv_sec && cap.tv_usec < tvbuf.tv_usec)) {
            tvbuf = cap;
            tvp = &tvbuf;
        }
    }

    int nready = (maxfd >= 0 || tvp)
        ? ::select(maxfd + 1, &rset, &wset, &eset, tvp)
        : 0;
    if (nready < 0) {
        return -1;  // EINTR etc.; caller loops again
    }

    int dispatched = 0;
    // Once a handler returns -1 in any of read/write/except, it is retiring:
    // exclude it from the remaining masks this same cycle (ACE removes a
    // handler on the first -1; calling its other hooks afterward -- on an
    // object about to be torn down -- is wrong), and retire each fd once.
    std::set<ACE_HANDLE> retiring;

    if (nready > 0) {
        // Snapshot ready fds, then dispatch re-verifying registration each
        // time (a callback may have removed handlers, incl. via reentrancy).
        std::vector<ACE_HANDLE> rfds, wfds, efds;
        for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = read_.begin();
             it != read_.end(); ++it)
            if (it->first >= 0 && it->first < FD_SETSIZE && FD_ISSET(it->first, &rset))
                rfds.push_back(it->first);
        for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = write_.begin();
             it != write_.end(); ++it)
            if (it->first >= 0 && it->first < FD_SETSIZE && FD_ISSET(it->first, &wset))
                wfds.push_back(it->first);
        for (std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = except_.begin();
             it != except_.end(); ++it)
            if (it->first >= 0 && it->first < FD_SETSIZE && FD_ISSET(it->first, &eset))
                efds.push_back(it->first);

        for (size_t i = 0; i < rfds.size(); i++) {
            if (retiring.count(rfds[i])) continue;
            std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = read_.find(rfds[i]);
            if (it == read_.end()) continue;  // removed mid-dispatch
            int rc = it->second->handle_input(rfds[i]);
            dispatched++;
            if (rc < 0) retiring.insert(rfds[i]);
        }
        for (size_t i = 0; i < wfds.size(); i++) {
            if (retiring.count(wfds[i])) continue;  // already retiring from read
            std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = write_.find(wfds[i]);
            if (it == write_.end()) continue;
            int rc = it->second->handle_output(wfds[i]);
            dispatched++;
            if (rc < 0) retiring.insert(wfds[i]);
        }
        for (size_t i = 0; i < efds.size(); i++) {
            if (retiring.count(efds[i])) continue;
            std::map<ACE_HANDLE, ACE_Event_Handler*>::iterator it = except_.find(efds[i]);
            if (it == except_.end()) continue;
            int rc = it->second->handle_exception(efds[i]);
            dispatched++;
            if (rc < 0) retiring.insert(efds[i]);
        }
    }

    for (std::set<ACE_HANDLE>::iterator it = retiring.begin();
         it != retiring.end(); ++it) {
        retire(*it, ACE_Event_Handler::RWE_MASK);
    }

    dispatched += expire_timers();
    return dispatched;
}
