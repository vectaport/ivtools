/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

// Dispatcher provides an interface to the "select" system call.

#include <Dispatch/dispatcher.h>
#include <Dispatch/iohandler.h>
#include <OS/memory.h>
#include <OS/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#undef NULL
#include <sys/param.h>
#if defined(AIXV3) || defined(svr4) || defined(AIXV4)
#include <sys/select.h>
#endif
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

/* no standard place for this */
extern "C" {
#if defined(hpux)
//    extern int select(size_t, int*, int*, int*, struct timeval*);
#else
#if !defined(AIXV3) && !defined(svr4) && !defined(__lucid) && !defined(linux)
    extern int select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
#endif
#endif
#if defined(__DECCXX) || defined(sun)
#if !defined(__svr4__)
    extern int gettimeofday(struct timeval*, struct timezone*);
#endif
#endif
#if defined(sun)
    extern int sigvec(int sig, struct sigvec* vec, struct sigvec* ovec);
#endif
}

#if defined(__GLIBC__) && (__GLIBC__==2 && __GLIBC_MINOR__>0 || __GLIBC__>2) && __GNUC__<3
#define fds_bits __fds_bits
#endif

Dispatcher* Dispatcher::_instance;

class FdMask : public fd_set {
public:
    FdMask();
    void zero();
    void setBit(int);
    void clrBit(int);
    boolean isSet(int);
    boolean anySet() const;
    int numSet() const;
};

FdMask::FdMask() {
    zero();
}

void FdMask::zero() { Memory::zero(this, sizeof(FdMask)); }
void FdMask::setBit(int fd) { FD_SET(fd,this); }
void FdMask::clrBit(int fd) { FD_CLR(fd,this); }
boolean FdMask::isSet(int fd) { return FD_ISSET(fd,this); }

boolean FdMask::anySet() const {
    const int mskcnt = howmany(FD_SETSIZE,NFDBITS);
    for (int i = 0; i < mskcnt; i++) {
	if (fds_bits[i]) {
	    return true;
	}
    }
    return false;
}

int FdMask::numSet() const {
    const int mskcnt = howmany(FD_SETSIZE,NFDBITS);
    int n = 0;
    for (int i = 0; i < mskcnt; i++) {
	if (fds_bits[i]) {
	    for (int j = 0; j < NFDBITS; j++) {
		if ((fds_bits[i] & (1 << j)) != 0) {
		    n += 1;
		}
	    }
	}
    }
    return n;
}

/*
 * Operations on timeval structures.
 */

const long ONE_SECOND = 1000000;

timeval operator+(timeval src1, timeval src2) {
    timeval sum;
    sum.tv_sec = src1.tv_sec + src2.tv_sec;
    sum.tv_usec = src1.tv_usec + src2.tv_usec;
    if (sum.tv_usec >= ONE_SECOND) {
	sum.tv_usec -= ONE_SECOND;
	sum.tv_sec++;
    } else if (sum.tv_sec >= 1 && sum.tv_usec < 0) {
	sum.tv_usec += ONE_SECOND;
	sum.tv_sec--;
    }
    return sum;
}

timeval operator-(timeval src1, timeval src2) {
    timeval delta;
    delta.tv_sec = src1.tv_sec - src2.tv_sec;
    delta.tv_usec = src1.tv_usec - src2.tv_usec;
    if (delta.tv_usec < 0) {
	delta.tv_usec += ONE_SECOND;
	delta.tv_sec--;
    } else if (delta.tv_usec >= ONE_SECOND) {
	delta.tv_usec -= ONE_SECOND;
	delta.tv_sec++;
    }
    return delta;
}

boolean operator>(timeval src1, timeval src2) {
    if (src1.tv_sec > src2.tv_sec) {
	return true;
    } else if (src1.tv_sec == src2.tv_sec && src1.tv_usec > src2.tv_usec) {
	return true;
    } else {
	return false;
    }
}

boolean operator<(timeval src1, timeval src2) {
    if (src1.tv_sec < src2.tv_sec) {
	return true;
    } else if (src1.tv_sec == src2.tv_sec && src1.tv_usec < src2.tv_usec) {
	return true;
    } else {
	return false;
    }
}

/*
 * Interface to timers.
 */

struct Timer {
    Timer(timeval t, IOHandler* h, Timer* n);

    timeval timerValue;
    IOHandler* handler;
    Timer* next;
};

class TimerQueue {
public:
    TimerQueue();
    virtual ~TimerQueue();

    boolean isEmpty() const;
    static timeval zeroTime();
    timeval earliestTime() const;
    static timeval currentTime();

    void insert(timeval, IOHandler*);
    void remove(IOHandler*);
    void expire(timeval);
private:
    Timer* _first;
    static timeval _zeroTime;
};

Timer::Timer(timeval t, IOHandler* h, Timer* n) :
    timerValue(t),
    handler(h),
    next(n) {}

timeval TimerQueue::_zeroTime;

TimerQueue::TimerQueue() :
    _first(nil) {}

TimerQueue::~TimerQueue() {
    Timer* doomed = _first;
    while (doomed != nil) {
	Timer* next = doomed->next;
	delete doomed;
	doomed = next;
    }
}

inline boolean TimerQueue::isEmpty() const {
    return _first == nil;
}

inline timeval TimerQueue::zeroTime() {
    return _zeroTime;
}

inline timeval TimerQueue::earliestTime() const {
    return _first->timerValue;
}

timeval TimerQueue::currentTime() {
    timeval curTime;
#if defined(svr4) && !defined(__GNUC__)
    gettimeofday(&curTime);
#else
    struct timezone curZone;
    gettimeofday(&curTime, &curZone);
#endif
    return curTime;
}

void TimerQueue::insert(timeval futureTime, IOHandler* handler) {
    if (isEmpty() || futureTime < earliestTime()) {
	_first = new Timer(futureTime, handler, _first);
    } else {
	Timer* before = _first;
	Timer* after = _first->next;
	while (after != nil && futureTime > after->timerValue) {
	    before = after;
	    after = after->next;
	}
	before->next = new Timer(futureTime, handler, after);
    }
}

void TimerQueue::remove(IOHandler* handler) {
    Timer* before = nil;
    Timer* doomed = _first;
    while (doomed != nil && doomed->handler != handler) {
	before = doomed;
	doomed = doomed->next;
    }
    if (doomed != nil) {
	if (before == nil) {
	    _first = doomed->next;
	} else {
	    before->next = doomed->next;
	}
	delete doomed;
    }
}

void TimerQueue::expire(timeval curTime) {
    while (!isEmpty() && earliestTime() < curTime) {
	Timer* expired = _first;
	_first = _first->next;
	expired->handler->timerExpired(curTime.tv_sec, curTime.tv_usec);
	delete expired;
    }
}

/*
 * Interface to child process handling.
 */

struct Child {
    Child(pid_t pid, IOHandler* h, Child* n);

    pid_t pid;		// process's PID
    int status;		// wait status
    IOHandler* handler;	// associated handler
    Child* next;
};

class ChildQueue {
public:
    ChildQueue();
    virtual ~ChildQueue();

    boolean isEmpty() const;
    boolean isReady() const;

    void insert(pid_t, IOHandler*);
    void remove(IOHandler*);
    void notify();
    void setStatus(pid_t, int status);
private:
    Child* _first;            // queue head
    boolean _ready;           // something is ready
};

Child::Child(pid_t p, IOHandler* h, Child* n) {
    pid = p;
    status = -1;
    handler = h;
    next = n;
}

ChildQueue::ChildQueue() {
    _first = nil;
    _ready = false;
}

ChildQueue::~ChildQueue() {
    Child* doomed = _first;
    while (doomed != nil) {
	Child* next = doomed->next;
	delete doomed;
	doomed = next;
    }
}

inline boolean ChildQueue::isEmpty() const { return _first == nil; }
inline boolean ChildQueue::isReady() const { return _ready; }

void ChildQueue::insert(pid_t p, IOHandler* handler) {
    if (isEmpty()) {
	_first = new Child(p, handler, _first);
    } else {
	Child* before = _first;
	Child* after = _first->next;
	while (after != nil && p > after->pid) {
	    before = after;
	    after = after->next;
	}
	before->next = new Child(p, handler, after);
    }
}

void ChildQueue::remove(IOHandler* handler) {
    Child* before = nil;
    Child* doomed = _first;
    while (doomed != nil && doomed->handler != handler) {
	before = doomed;
	doomed = doomed->next;
    }
    if (doomed != nil) {
	if (before == nil) {
	    _first = doomed->next;
	} else {
	    before->next = doomed->next;
	}
	delete doomed;
    }
}

void ChildQueue::setStatus(pid_t p, int status) {
    for (Child* c = _first; c != nil; c = c->next) {
	if (c->pid == p) {
	    c->status = status;
	    _ready = true;
	    break;
	}
    }
}

void ChildQueue::notify() {
    Child** prev = &_first;
    Child* c;

    while ((c = *prev) != nil) {
	if (c->status != -1) {
	    c->handler->childStatus(c->pid, c->status);
	    *prev = c->next;
	    delete c;
	} else {
	    prev = &c->next;
	}
    }
    _ready = false;
}

#if defined(__svr4__) && defined(sun) && NOFILE==20
#undef NOFILE
#define NOFILE 256
#endif

#ifndef NOFILE
#define NOFILE 256
#endif

Dispatcher::Dispatcher() {
    _nfds = 0;
    _rmask = new FdMask;
    _wmask = new FdMask;
    _emask = new FdMask;
    _rmaskready = new FdMask;
    _wmaskready = new FdMask;
    _emaskready = new FdMask;
    _rtable = new IOHandler*[NOFILE];
    _wtable = new IOHandler*[NOFILE];
    _etable = new IOHandler*[NOFILE];
    _queue = new TimerQueue;
    _cqueue = new ChildQueue;
    for (int i = 0; i < NOFILE; i++) {
	_rtable[i] = nil;
	_wtable[i] = nil;
	_etable[i] = nil;
    }
}

Dispatcher::~Dispatcher() {
    delete _rmask;
    delete _wmask;
    delete _emask;
    delete _rmaskready;
    delete _wmaskready;
    delete _emaskready;
    delete [] _rtable;
    delete [] _wtable;
    delete [] _etable;
    delete _queue;
    delete _cqueue;
}

Dispatcher& Dispatcher::instance() {
    if (_instance == nil) {
	_instance = new Dispatcher;
    }
    return *_instance;
}

void Dispatcher::instance(Dispatcher* d) { _instance = d; }

IOHandler* Dispatcher::handler(int fd, DispatcherMask mask) const {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    IOHandler* cur = nil;
    if (mask == ReadMask) {
	cur = _rtable[fd];
    } else if (mask == WriteMask) {
	cur = _wtable[fd];
    } else if (mask == ExceptMask) {
	cur = _etable[fd];
    } else {
	abort();
    }
    return cur;
}

void Dispatcher::link(int fd, DispatcherMask mask, IOHandler* handler) {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    attach(fd, mask, handler);
}

void Dispatcher::unlink(int fd) {
    if (fd < 0 || fd >= NOFILE) {
	abort();
    }
    detach(fd);
}

void Dispatcher::attach(int fd, DispatcherMask mask, IOHandler* handler) {
    if (mask == ReadMask) {
	_rmask->setBit(fd);
	_rtable[fd] = handler;
    } else if (mask == WriteMask) {
	_wmask->setBit(fd);
	_wtable[fd] = handler;
    } else if (mask == ExceptMask) {
	_emask->setBit(fd);
	_etable[fd] = handler;
    } else {
	abort();
    }
    if (_nfds < fd+1) {
	_nfds = fd+1;
    }
}

void Dispatcher::detach(int fd) {
    _rmask->clrBit(fd);
    _rtable[fd] = nil;
    _wmask->clrBit(fd);
    _wtable[fd] = nil;
    _emask->clrBit(fd);
    _etable[fd] = nil;
    if (_nfds == fd+1) {
	while (_nfds > 0 && _rtable[_nfds-1] == nil &&
	       _wtable[_nfds-1] == nil && _etable[_nfds-1] == nil
	) {
	    _nfds--;
	}
    }
}

void Dispatcher::startTimer(long sec, long usec, IOHandler* handler) {
    timeval deltaTime;
    deltaTime.tv_sec = sec;
    deltaTime.tv_usec = usec;
    _queue->insert(TimerQueue::currentTime() + deltaTime, handler);
}

void Dispatcher::stopTimer(IOHandler* handler) {
    _queue->remove(handler);
}

void Dispatcher::startChild(int pid, IOHandler* handler) {
    _cqueue->insert(pid, handler);
}

void Dispatcher::stopChild(IOHandler* handler) {
    _cqueue->remove(handler);
}

boolean Dispatcher::setReady(int fd, DispatcherMask mask) {
    if (handler(fd, mask) == nil) {
	return false;
    }
    if (mask == ReadMask) {
	_rmaskready->setBit(fd);
    } else if (mask == WriteMask) {
	_wmaskready->setBit(fd);
    } else if (mask == ExceptMask) {
	_emaskready->setBit(fd);
    } else {
	return false;
    }
    return true;
}

void Dispatcher::dispatch() {
    dispatch(nil);
}

boolean Dispatcher::dispatch(long& sec, long& usec) {
    timeval howlong;
    timeval prevTime;
    timeval elapsedTime;

    howlong.tv_sec = sec;
    howlong.tv_usec = usec;
    prevTime = TimerQueue::currentTime();

    boolean success = dispatch(&howlong);

    elapsedTime = TimerQueue::currentTime() - prevTime;
    if (howlong > elapsedTime) {
	howlong = howlong - elapsedTime;
    } else {
	howlong = TimerQueue::zeroTime(); /* Used all of timeout */
    }

    sec = howlong.tv_sec;
    usec = howlong.tv_usec;
    return success;
}

boolean Dispatcher::dispatch(timeval* howlong) {
    FdMask rmaskret;
    FdMask wmaskret;
    FdMask emaskret;
    int nfound;

    if (anyReady()) {
	nfound = fillInReady(rmaskret, wmaskret, emaskret);
    } else {
	nfound = waitFor(rmaskret, wmaskret, emaskret, howlong);
    }

    notify(nfound, rmaskret, wmaskret, emaskret);

    return (nfound != 0);
}

boolean Dispatcher::anyReady() const {
    return
       _rmaskready->anySet() || _wmaskready->anySet() || _emaskready->anySet();
}

int Dispatcher::fillInReady(
    FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret
) {
    rmaskret = *_rmaskready;
    wmaskret = *_wmaskready;
    emaskret = *_emaskready;
    _rmaskready->zero();
    _wmaskready->zero();
    _emaskready->zero();
    return rmaskret.numSet() + wmaskret.numSet() + emaskret.numSet();
}

#if defined(__GNUC__) && !defined(__GNUC_MINOR__)
void Dispatcher::sigCLD() {
#else
void Dispatcher::sigCLD(...) {
#endif
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
	Dispatcher::instance()._cqueue->setStatus(pid, status);
    }
}

#ifndef fxSIGHANDLER
#define fxSIGHANDLER
#endif
#ifndef fxSIGVECHANDLER
#define fxSIGVECHANDLER
#endif
#ifndef fxSIGACTIONHANDLER
#define fxSIGACTIONHANDLER
#endif

#ifndef SA_INTERRUPT
#define SA_INTERRUPT 0
#endif

#ifndef SIGCLD   /* attempt to correct for glibc2 signal names */
#define SIGCLD SIGCHLD
#endif 

int Dispatcher::waitFor(
    FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret, timeval* howlong
) {
    int nfound;
#ifdef SV_INTERRUPT                   /* BSD-style */
#if defined(sun) && defined(__GNUC__)
    static struct sigaction sv, osv;
#else
    static struct sigvec sv, osv;
#endif
#else
#ifdef SA_NOCLDSTOP                   /* POSIX */
    static struct sigaction sa, osa;
#else                                 /* System V-style */
    void (*osig)();
#endif
#endif

    if (!_cqueue->isEmpty()) {
#ifdef SV_INTERRUPT                   /* BSD-style */
#if defined(sun) && defined(__GNUC__)
	sv.sa_handler = fxSIGVECHANDLER(&Dispatcher::sigCLD);
	sv.sa_flags = SV_INTERRUPT;
	sigaction(SIGCLD, &sv, &osv);
#else
	sv.sv_handler = (void (*)(int)) fxSIGVECHANDLER(&Dispatcher::sigCLD);
	sv.sv_flags = SV_INTERRUPT;
	sigvec(SIGCLD, &sv, &osv);
#endif
#else
#ifdef SA_NOCLDSTOP                   /* POSIX */
#if defined(__CYGWIN__) || defined(hpux) || defined(linux) || defined(sun) && defined(__svr4__)
	sa.sa_handler = (void (*)(int))(&Dispatcher::sigCLD);
#else
	sa.sa_handler = fxSIGACTIONHANDLER(&Dispatcher::sigCLD);
#endif
	sa.sa_flags = SA_INTERRUPT;
	sigaction(SIGCLD, &sa, &osa);
#else                                 /* System V-style */
	osig = (void (*)())signal(SIGCLD, fxSIGHANDLER(&Dispatcher::sigCLD));
#endif
#endif
    }

    do {
	rmaskret = *_rmask;
	wmaskret = *_wmask;
	emaskret = *_emask;
	howlong = calculateTimeout(howlong);

#if defined(hpux)
 	nfound = select(
	    _nfds, (int*)&rmaskret, (int*)&wmaskret, (int*)&emaskret, howlong
	);
#else
 	nfound = select(_nfds, &rmaskret, &wmaskret, &emaskret, howlong);
#endif
    } while (nfound < 0 && !handleError());
    if (!_cqueue->isEmpty()) {
#ifdef SV_INTERRUPT                   /* BSD-style */
#if defined(sun) && defined(__GNUC__)
	sigaction(SIGCLD, &osv, (struct sigaction*) 0);
#else
	sigvec(SIGCLD, &osv, (struct sigvec*) 0);
#endif
#else
#ifdef SA_NOCLDSTOP                   /* POSIX */
	sigaction(SIGCLD, &osa, (struct sigaction*) 0);
#else                                 /* System V-style */
	(void) signal(SIGCLD, fxSIGHANDLER(osig));
#endif
#endif
    }

    return nfound;		/* Timed out or input available */
}

void Dispatcher::notify(
    int nfound, FdMask& rmaskret, FdMask& wmaskret, FdMask& emaskret
) {
    for (int i = 0; i < _nfds && nfound > 0; i++) {
	if (rmaskret.isSet(i)) {
#if 0
	    int status = _rtable[i]->inputReady(i);
#else
            int status = (_rtable[i] ? _rtable[i]->inputReady(i) : 0);
#endif
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_rmaskready->setBit(i);
	    }
	    nfound--;
	}
	if (wmaskret.isSet(i)) {
#if 0
	    int status = _wtable[i]->outputReady(i);
#else
            int status = (_wtable[i] ? _wtable[i]->outputReady(i) : 0);
#endif
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_wmaskready->setBit(i);
	    }
	    nfound--;
	}
	if (emaskret.isSet(i)) {
#if 0
	    int status = _etable[i]->exceptionRaised(i);
#else
            int status = (_etable[i] ? _etable[i]->exceptionRaised(i) : 0);
#endif
	    if (status < 0) {
		detach(i);
	    } else if (status > 0) {
		_emaskready->setBit(i);
	    }
	    nfound--;
	}
    }

    if (!_queue->isEmpty()) {
	_queue->expire(TimerQueue::currentTime());
    }
    if (_cqueue->isReady()) {
	_cqueue->notify();
    }
}

timeval* Dispatcher::calculateTimeout(timeval* howlong) const {
    static timeval timeout;

    if (!_queue->isEmpty()) {
	timeval curTime;

	curTime = TimerQueue::currentTime();
	if (_queue->earliestTime() > curTime) {
	    timeout = _queue->earliestTime() - curTime;
	    if (howlong == nil || *howlong > timeout) {
		howlong = &timeout;
	    }
	} else {
	    timeout = TimerQueue::zeroTime();
	    howlong = &timeout;
	}
    }
    return howlong;
}

boolean Dispatcher::handleError() {
    switch (errno) {
    case EBADF:
	checkConnections();
	break;
    case EINTR:
	if (_cqueue->isReady()) {
	    return true;
	}
	break;
    default:
	perror("Dispatcher: select");
	exit(1);
	/*NOTREACHED*/
    }
    return false;	// retry select;
}

void Dispatcher::checkConnections() {
    FdMask rmask;
    timeval poll = TimerQueue::zeroTime();

    for (int fd = 0; fd < _nfds; fd++) {
	if (_rtable[fd] != nil) {
	    rmask.setBit(fd);
#if defined(hpux)
	    if (select(fd+1, (int*)&rmask, nil, nil, &poll) < 0) {
#else
	    if (select(fd+1, &rmask, nil, nil, &poll) < 0) {
#endif
		detach(fd);
	    }
	    rmask.clrBit(fd);
	}
    }
}
