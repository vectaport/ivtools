/*
 * Copyright (c) 1994 Vectaport Inc.
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

#include <AceDispatch/ace_iohandler.h>

ACE_IO_Handler::ACE_IO_Handler(IOHandler* handler) : ACE_Event_Handler() {
    _iohandler = handler;
}

int ACE_IO_Handler::handle_input (ACE_HANDLE fd) {
    return _iohandler->inputReady(fd);
}

int ACE_IO_Handler::handle_output (ACE_HANDLE fd) {
    return _iohandler->outputReady(fd);
}

int ACE_IO_Handler::handle_exception (ACE_HANDLE fd) {
    return _iohandler->exceptionRaised(fd);
}

int ACE_IO_Handler::handle_timeout (const ACE_Time_Value &tv, const void *arg) {
    _iohandler->timerExpired(tv.sec(), tv.usec());
    return 0;
}

#endif

