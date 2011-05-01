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

#ifdef HAVE_ACE
#include <AceDispatch/ace_dispatcher.h>
#include <AceDispatch/ace_iohandler.h>
#include <ace/Event_Handler.h>
#include <ace/Reactor.h>

implementTable(TimerTable,IOHandler*,int)

AceDispatcher::AceDispatcher() : Dispatcher() {
    _reactor = new ACE_Reactor();
    _table = new TimerTable(1000);
}

AceDispatcher::AceDispatcher(ACE_Reactor* ar) : Dispatcher() {
    _reactor = ar;
    _table = new TimerTable(1000);
}

AceDispatcher::~AceDispatcher() {
    delete _reactor;
    delete _table;
}

void AceDispatcher::attach(int fd, DispatcherMask mask, IOHandler* handler) {
    if (mask == ReadMask) {
      //	_rmask->setBit(fd);
        _reactor->register_handler((ACE_HANDLE)fd,
				   new ACE_IO_Handler(handler),
				   ACE_Event_Handler::READ_MASK);
	_rtable[fd] = handler;
    } else if (mask == WriteMask) {
      //	_wmask->setBit(fd);
        _reactor->register_handler((ACE_HANDLE)fd,
				   new ACE_IO_Handler(handler),
				   ACE_Event_Handler::WRITE_MASK);
	_wtable[fd] = handler;
    } else if (mask == ExceptMask) {
      //	_emask->setBit(fd);
        _reactor->register_handler((ACE_HANDLE)fd,
				   new ACE_IO_Handler(handler),
				   ACE_Event_Handler::EXCEPT_MASK);
	_etable[fd] = handler;
    } else {
	abort();
    }
    if (_nfds < fd+1) {
	_nfds = fd+1;
    }
}

void AceDispatcher::detach(int fd) {
  //    _rmask->clrBit(fd);
    _reactor->remove_handler((ACE_HANDLE)fd, ACE_Event_Handler::RWE_MASK);
    _rtable[fd] = nil;
    //    _wmask->clrBit(fd);
    _wtable[fd] = nil;
    //    _emask->clrBit(fd);
    _etable[fd] = nil;
    if (_nfds == fd+1) {
	while (_nfds > 0 && _rtable[_nfds-1] == nil &&
	       _wtable[_nfds-1] == nil && _etable[_nfds-1] == nil
	) {
	    _nfds--;
	}
    }
}

void AceDispatcher::startTimer(long sec, long usec, IOHandler* handler) {
    ACE_Time_Value delta(sec, usec);
    ACE_IO_Handler* ehandler = new ACE_IO_Handler(handler);
    int timer_id = _reactor->schedule_timer(ehandler, NULL, delta);
    _table->insert(handler, timer_id);
}

void AceDispatcher::stopTimer(IOHandler* handler) {
    int timer_id;
    if (_table->find_and_remove(timer_id, handler)) {
        _reactor->cancel_timer(timer_id);
    }
}

void AceDispatcher::dispatch() {
    _reactor->handle_events();
}

boolean AceDispatcher::dispatch(long& sec, long& usec) {
    ACE_Time_Value* tv = new ACE_Time_Value(sec, usec);

    timeval curTime1;
#if defined(svr4) && !defined(__GNUC__)
    gettimeofday(&curTime1);
#else
    struct timezone curZone;
    gettimeofday(&curTime1, &curZone);
#endif

    int nfound = _reactor->handle_events(tv);

    timeval curTime2;
#if defined(svr4) && !defined(__GNUC__)
    gettimeofday(&curTime2);
#else
    gettimeofday(&curTime2, &curZone);
#endif

    ACE_Time_Value* t1 = new ACE_Time_Value(curTime1);
    ACE_Time_Value* t2 = new ACE_Time_Value(curTime2);
    ACE_Time_Value* elapsed = new ACE_Time_Value();
    ACE_Time_Value* howlong = new ACE_Time_Value();
    *elapsed = *t2 - *t1;
    if (*tv > *elapsed) {
        *howlong = *tv - *elapsed;
    }
    sec = howlong->sec();
    usec = howlong->usec();

    delete tv;
    delete t1;
    delete t2;
    delete elapsed;
    delete howlong;
    return nfound > 0;
}

#endif
