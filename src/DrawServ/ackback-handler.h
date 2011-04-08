/*
 * Copyright (c) 2004 Scott E. Johnston
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

#ifndef _ackback_handler_
#define _ackback_handler_

#include <ace/Event_Handler.h>

class DrawLink;

//: specialized ACE_EventHandler for monitoring responses from outgoing connection
class AckBackHandler : public ACE_Event_Handler
{

public:
  // = Initialization and termination methods.
  AckBackHandler ();

  virtual ~AckBackHandler ();

  DrawLink* drawlink() { return _drawlink; }
  // get DrawLink associated with this handler
  void drawlink(DrawLink* link) { _drawlink = link; }
  // set DrawLink associated with this handler

  virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
  // Called when input events occur (e.g., connection or data).

  virtual int handle_timeout (const ACE_Time_Value &tv, 
			      const void *arg); 
  // called when timer goes off.

  virtual int handle_close (ACE_HANDLE handle,
                            ACE_Reactor_Mask close_mask);
  // Called when a <handle_*()> method returns -1 or when the
  // <remove_handler> method is called on an <ACE_Reactor>.  The
  // <close_mask> indicates which event has triggered the
  // <handle_close> method callback on a particular <handle>.

  virtual ACE_HANDLE get_handle (void) const;
  // Get the I/O handle.

  virtual void set_handle (ACE_HANDLE);
  // Set the I/O handle.

  void start_timer();
  // Start timer waiting for ackback

protected:
  DrawLink* _drawlink;
  int _timer_started;
  int _ackback_arrived;
  long _timerid;
  ACE_HANDLE _handle;
  int _eof_expected;

};

#endif /* _ackback_handler_ */

#endif /* HAVE_ACE */
