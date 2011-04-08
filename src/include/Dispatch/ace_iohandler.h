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

#ifndef _ace_iohandler_h
#define _ace_iohandler_h

#ifdef HAVE_ACE

#include <ace/Event_Handler.h>
#include <Dispatch/iohandler.h>

// An ACE wrapper around InterViews IOHandler.  This makes possible
// usage of InterViews IOHandlers with ACE_Dispatcher.

class ACE_IO_Handler : public ACE_Event_Handler {
public:
    ACE_IO_Handler(IOHandler*);

    IOHandler* iohandler() { return _iohandler; }

  virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
  // Called when input events occur (e.g., connection or data).

  virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);
  // Called when output events are possible (e.g., flow control
  // abates).

  virtual int handle_exception (ACE_HANDLE fd = ACE_INVALID_HANDLE);
  // Called when execption events occur (e.g., SIGURG).

  virtual int handle_timeout (const ACE_Time_Value &tv, 
			      const void *arg = 0);
  // Called when timer expires.

protected:
    IOHandler* _iohandler;
};

#endif

#endif
