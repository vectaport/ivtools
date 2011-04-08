/*
 * Copyright (c) 1996 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#ifndef _ace_dispatcher_h
#define _ace_dispatcher_h

#ifdef HAVE_ACE

#include <Dispatch/dispatcher.h>
#include <Dispatch/iohandler.h>
#include <OS/table.h>

class ACE_Reactor;

declareTable(TimerTable,IOHandler*,int)

// A Dispatcher implemented with an ACE_Reactor.  Make sure that
// Dispatcher::_instance is set to one of these when using InterViews
// with ACE.

class AceDispatcher : public Dispatcher {
public:
    AceDispatcher();
    AceDispatcher(ACE_Reactor*);
    virtual ~AceDispatcher();

    ACE_Reactor* reactor() { return _reactor; }

  //    virtual void link(int fd, DispatcherMask, IOHandler*);
  //    virtual IOHandler* handler(int fd, DispatcherMask) const;
  //    virtual void unlink(int fd);

    virtual void startTimer(long sec, long usec, IOHandler*);
    virtual void stopTimer(IOHandler*);

  //    virtual boolean setReady(int fd, DispatcherMask);
    virtual void dispatch();
    virtual boolean dispatch(long& sec, long& usec);
protected:
    virtual void attach(int fd, DispatcherMask, IOHandler*);
    virtual void detach(int fd);

    ACE_Reactor* _reactor;
    TimerTable* _table;
};

#endif

#endif
