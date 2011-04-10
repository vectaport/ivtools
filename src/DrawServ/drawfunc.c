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
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>
#endif

#include <DrawServ/draweditor.h>
#include <DrawServ/drawfunc.h>

#define TITLE "DrawServFunc"

/*****************************************************************************/

DrawServFunc::DrawServFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawServFunc::execute() {
#ifndef HAVE_ACE

  reset_stack();
  fprintf(stderr, "rebuild ivtools with ACE support to get full drawserv functionality\n");
  push_stack(ComValue::nullval());

#else

  ComValue hostv(stack_arg(0, true));
  static int port_sym = symbol_add("port");
  ComValue default_port(20002);
  ComValue portv(stack_key(port_sym, false, default_port, true));
  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  if (hostv.is_string() && portv.is_known()) {

    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    ACE_INET_Addr addr (portnum, hoststr);
    ACE_SOCK_Stream socket;
    ACE_SOCK_Connector conn;
    if (conn.connect (socket, addr) == -1) {
      ACE_ERROR ((LM_ERROR, "%p\n", "open"));
      push_stack(ComValue::nullval());
      return;
    }

    if (socket.close () == -1)
        ACE_ERROR ((LM_ERROR, "%p\n", "close"));

    push_stack(ComValue::zeroval());
  } 
    
  return;

#endif

}

