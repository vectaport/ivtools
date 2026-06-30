/*
 * ACE-lite (issue #147).  src/ACE-lite/tests/reactor_loop.cc
 * Standalone Phase-2 unit test for ACE_Reactor / ACE_Event_Handler.  Exercises
 * exactly the reactor API AceDispatcher and comhandler use:
 *   register_handler(fd, eh, READ_MASK), remove_handler(fd, RWE_MASK),
 *   schedule_timer(eh, NULL, delay), cancel_timer(id), handle_events(&tv).
 * Plus the -1 => retire+handle_close contract and reentrant handle_events.
 * Build:
 *   g++ -I src/include src/ACE-lite/{reactor,event_handler,sock}.c \
 *       src/ACE-lite/tests/reactor_loop.cc
 * Exit 0 = pass, 1 = fail.
 */

#include <ACE-lite/Reactor.h>
#include <ACE-lite/Event_Handler.h>
#include <ACE-lite/Time_Value.h>
#include <ACE-lite/Test_and_Set.h>
#include <ACE-lite/Synch.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int failures = 0;
static void check(int cond, const char* what) {
    printf("%s: %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

// A read handler that drains its fd and counts callbacks.  Returns -1 (retire)
// on EOF, like a real socket handler.
class ReadHandler : public ACE_Event_Handler {
public:
    ReadHandler(ACE_HANDLE fd) : fd_(fd), inputs_(0), closed_(0), total_(0) {}
    virtual ACE_HANDLE get_handle() const { return fd_; }
    virtual int handle_input(ACE_HANDLE) {
        char buf[64];
        ssize_t n = ::read(fd_, buf, sizeof(buf));
        inputs_++;
        if (n <= 0) return -1;   // EOF -> reactor retires us
        total_ += n;
        return 0;
    }
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask) { closed_++; return 0; }
    ACE_HANDLE fd_;
    int inputs_, closed_, total_;
};

// A timer handler that counts expirations; optionally re-enters handle_events.
class TimerHandler : public ACE_Event_Handler {
public:
    TimerHandler() : fires_(0), reenter_(0), reactor_for_reentry_(0) {}
    virtual int handle_timeout(const ACE_Time_Value&, const void*) {
        fires_++;
        if (reenter_ && reactor_for_reentry_) {
            ACE_Time_Value zero(0, 0);
            reactor_for_reentry_->handle_events(&zero);  // reentrancy probe
        }
        return 0;
    }
    int fires_, reenter_;
    ACE_Reactor* reactor_for_reentry_;
};

// Registered for READ+WRITE; returns -1 on input (asks to retire).  Its
// handle_output must NOT fire in the same cycle once it has retired.
class RWHandler : public ACE_Event_Handler {
public:
    RWHandler(ACE_HANDLE fd) : fd_(fd), outputs_(0) {}
    virtual ACE_HANDLE get_handle() const { return fd_; }
    virtual int handle_input(ACE_HANDLE) {
        char b[16];
        ssize_t n = ::read(fd_, b, sizeof(b));
        (void)n;
        return -1;             // retire me
    }
    virtual int handle_output(ACE_HANDLE) { outputs_++; return 0; }
    ACE_HANDLE fd_;
    int outputs_;
};

int main() {
    ACE_Reactor reactor;

    // --- a connected fd pair to drive read events ---
    int sv[2];
    check(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "socketpair");
    ReadHandler rh(sv[0]);

    // 3-arg register (the AceDispatcher form).
    check(reactor.register_handler(sv[0], &rh, ACE_Event_Handler::READ_MASK) == 0,
          "register_handler(fd, eh, READ_MASK)");

    // No data yet -> a short wait dispatches nothing.
    ACE_Time_Value shortw(0, 1000);
    check(reactor.handle_events(&shortw) == 0, "handle_events times out with no input");

    // Write -> one input dispatched, payload delivered.
    write(sv[1], "hello", 5);
    ACE_Time_Value w1(1, 0);
    int n = reactor.handle_events(&w1);
    check(n == 1 && rh.inputs_ == 1 && rh.total_ == 5, "handle_events dispatches handle_input");

    // Close peer -> EOF -> handler returns -1 -> retired + handle_close called.
    close(sv[1]);
    ACE_Time_Value w2(1, 0);
    reactor.handle_events(&w2);
    check(rh.closed_ == 1, "EOF: handle_input -1 => retire + handle_close");
    // Now removed: a further wait dispatches nothing more for this fd.
    ACE_Time_Value w3(0, 1000);
    int after = reactor.handle_events(&w3);
    check(after == 0, "retired handler no longer dispatched");
    close(sv[0]);

    // --- one-shot timer ---
    TimerHandler th;
    int id = reactor.schedule_timer(&th, 0, ACE_Time_Value(0, 20000));
    check(id > 0, "schedule_timer returns id");
    ACE_Time_Value w4(1, 0);
    reactor.handle_events(&w4);
    check(th.fires_ == 1, "one-shot timer fires once");
    ACE_Time_Value w5(0, 30000);
    reactor.handle_events(&w5);
    check(th.fires_ == 1, "one-shot timer does not refire");

    // --- cancel_timer before it fires ---
    TimerHandler th2;
    int id2 = reactor.schedule_timer(&th2, 0, ACE_Time_Value(0, 50000));
    check(reactor.cancel_timer(id2) == 1, "cancel_timer(id) cancels pending timer");
    ACE_Time_Value w6(0, 80000);
    reactor.handle_events(&w6);
    check(th2.fires_ == 0, "cancelled timer never fires");

    // --- reentrant handle_events from within a timer callback ---
    TimerHandler th3;
    th3.reenter_ = 1;
    th3.reactor_for_reentry_ = &reactor;
    reactor.schedule_timer(&th3, 0, ACE_Time_Value(0, 10000));
    ACE_Time_Value w7(1, 0);
    reactor.handle_events(&w7);
    check(th3.fires_ == 1, "reentrant handle_events inside a callback is safe");

    // --- a -1 from handle_input must exclude the fd from same-cycle output ---
    int sv2[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv2);
    RWHandler rw(sv2[0]);
    reactor.register_handler(sv2[0], &rw,
                             ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK);
    write(sv2[1], "x", 1);     // read-ready; the socket is also write-ready
    ACE_Time_Value w8(1, 0);
    reactor.handle_events(&w8);
    check(rw.outputs_ == 0,
          "handle_input -1 excludes the fd from handle_output in the same cycle");
    // And it is fully retired: another turn produces no further output.
    ACE_Time_Value w9(0, 1000);
    reactor.handle_events(&w9);
    check(rw.outputs_ == 0, "retired fd stays out of write dispatch");
    close(sv2[0]);
    close(sv2[1]);

    // --- remove_handler(eh) by handler pointer (the drawlink form) ---
    int sv3[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv3);
    ReadHandler rh2(sv3[0]);
    reactor.register_handler(sv3[0], &rh2, ACE_Event_Handler::READ_MASK);
    reactor.remove_handler(&rh2, ACE_Event_Handler::READ_MASK);  // by handler, not fd
    write(sv3[1], "x", 1);
    ACE_Time_Value w10(0, 1000);
    reactor.handle_events(&w10);
    check(rh2.inputs_ == 0, "remove_handler(eh) by pointer stops dispatch");
    close(sv3[0]);
    close(sv3[1]);

    // --- ACE_Time_Value::zero ---
    check(ACE_Time_Value::zero.sec() == 0 && ACE_Time_Value::zero.usec() == 0,
          "ACE_Time_Value::zero is 0");

    // --- signal handler: register an ACE_Test_and_Set, raise the signal,
    //     verify handle_signal set the flag (the SIGINT quit pattern) ---
    ACE_Test_and_Set<ACE_Null_Mutex, sig_atomic_t> quit;
    check(quit.is_set() == 0, "Test_and_Set flag starts clear");
    reactor.register_handler(SIGUSR1, &quit);
    raise(SIGUSR1);
    check(quit.is_set() == 1, "register_handler(signum) -> handle_signal sets the flag");

    printf("\nreactor_loop: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
