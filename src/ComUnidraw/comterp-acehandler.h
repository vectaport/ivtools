/*
 * Copyright (c) 1996,1999 Vectaport Inc.
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

#ifndef _unidraw_comterp_handler_
#define _unidraw_comterp_handler_

#include <stdio.h>
#include <signal.h>
#include <ComTerp/comhandler.h>

class ComEditor;

//: specialized ComterpHandler for integration into Unidraw event loop.
class UnidrawComterpHandler : public ComterpHandler
{

public:
  // = Initialization and termination methods.
  UnidrawComterpHandler (ComTerpServ* serv=NULL);

};

//: Acceptor specialized for use with Unidraw and ComTerp.
typedef ACE_Acceptor <UnidrawComterpHandler, ACE_SOCK_ACCEPTOR> 
	UnidrawComterpAcceptor;

#endif /* _unidraw_comterp_handler_ */

#endif /* HAVE_ACE */
