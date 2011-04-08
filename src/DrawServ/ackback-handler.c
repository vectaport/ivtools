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

#include <DrawServ/ackback-handler.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawkit.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawserv.h>

#include <IVGlyph/gdialogs.h>
#include <InterViews/window.h>

#include <vector.h>
#include <err.h>

/*****************************************************************************/

// Default constructor.

AckBackHandler::AckBackHandler ()
{
  _timer_started = false;
  _ackback_arrived = false;
  _eof_expected = false;
}

AckBackHandler::~AckBackHandler() {
  fprintf(stderr, "AckBackHandler deleted\n");
}

// Called when input becomes available on fd.

int AckBackHandler::handle_input (ACE_HANDLE fd)
{
  if (drawlink() && drawlink()->socket()) {
    vector<char> inv;
    char ch;
    int status;
    while((status = read(fd, &ch, 1))==1) inv.push_back(ch);
    inv.push_back('\0');
    if (strcmp((char*)&inv[0], "ackback(cycle)\n")==0) {
      char buffer[BUFSIZ];
      snprintf(buffer, BUFSIZ, "%s:%d", drawlink()->hostname(), drawlink()->portnum());
      GAcknowledgeDialog::map(DrawKit::Instance()->GetEditor()->GetWindow(), "Redundant connection rejected", buffer, "Redundant connection rejected");
      _eof_expected = true;
    }
    if (status == 0) {
      if (!_eof_expected) {
	char buffer[BUFSIZ];
	snprintf(buffer, BUFSIZ, "%s:%d", drawlink()->hostname(), drawlink()->portnum());
	GAcknowledgeDialog::map(DrawKit::Instance()->GetEditor()->GetWindow(), "Unexpected end-of-file on connection", buffer, "Unexpected end-of-file on connection");
      } else
	_eof_expected = false;
      cerr << "AckBack (end of file):  [" << (char*)&inv[0] << "]\n";
      drawlink()->ackhandler(nil);
      ((DrawServ*)unidraw)->linkdown(drawlink());
      return -1;
    } else if (errno != EAGAIN)
      warn(nil);
    else {
      cerr << "AckBack:  [" << (char*)&inv[0] << "]\n";
      _ackback_arrived = true;
    }
    return 0;
  } else {
    fprintf(stderr, "unexpected missing socket\n");
    return -1;
  }
}

void AckBackHandler::start_timer() {
  if (!_timer_started) {
    _timerid = ComterpHandler::reactor_singleton()->schedule_timer
      (this, (const void *) this, ACE_Time_Value (5), ACE_Time_Value (5));
    _timer_started = true;
    _ackback_arrived = false;
  }

}

int AckBackHandler::handle_timeout (const ACE_Time_Value &,
				    const void *arg)
{
  if(_timer_started && !ComterpHandler::reactor_singleton()->cancel_timer(_timerid, nil))
    cerr << "unable to cancel timerid " << _timerid << "\n";
  _timer_started = false;
  if (!_ackback_arrived) {
    fprintf(stderr, "ackback timeout\n");
    drawlink()->ackhandler(nil);
    ((DrawServ*)unidraw)->linkdown(drawlink());
    return -1;
  }

  return 0;
}

// Get the I/O handle.
ACE_HANDLE AckBackHandler::get_handle (void) const {
  return _handle;
}

 // Set the I/O handle.
void AckBackHandler::set_handle (ACE_HANDLE handle) {
  _handle = handle;
}


// Called when the object is about to be removed from the Dispatcher
// tables.

int  AckBackHandler::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
  fprintf(stderr, "AckBackHandler::handle_close called with mask 0x%x\n", mask);
  if(_timer_started && !ComterpHandler::reactor_singleton()->cancel_timer(_timerid, nil))
    cerr << "unable to cancel timerid " << _timerid << "\n";
  else
    _timer_started = false;
  if (mask == ACE_Event_Handler::TIMER_MASK || mask == ACE_Event_Handler::READ_MASK) {
    if (mask == ACE_Event_Handler::TIMER_MASK)
      if (ComterpHandler::reactor_singleton()->remove_handler(this, ACE_Event_Handler::READ_MASK|ACE_Event_Handler::TIMER_MASK)==-1)
	cerr << "drawserv: error removing ackback handler\n";
    delete this;
  }
  
  return 0;
}

#endif /* HAVE_ACE */
