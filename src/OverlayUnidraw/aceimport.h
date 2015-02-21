/*
 * Copyright (c) 1996-1999 Vectaport Inc.
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

#ifndef _unidraw_import_handler_
#define _unidraw_import_handler_

#include <stdio.h>
#include <signal.h>
#include <ace/Acceptor.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/Test_and_Set.h>

class OvImportCmd;
#include <fstream.h>
#include <iosfwd>

// GNU HURD has no fixed limit
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 4096
#endif

//: handler for import by socket into OverlayUnidraw.
class UnidrawImportHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{

public:
  UnidrawImportHandler ();

  virtual void destroy (void);  
  // ensure dynamic deallocation.

  virtual int open (void *);    
  // hook for opening handlers.
  virtual int close (u_long);   
  // hook for closing handlers.

protected:
  virtual int handle_input (ACE_HANDLE); 
  // called when input ready on 'ACE_HANDLE'.
  virtual int handle_timeout (const ACE_Time_Value &tv, 
			      const void *arg); 
  // called when timeout occurs.

  char peer_name_[MAXHOSTNAMELEN + 1];
  // host we are connected to.

  OvImportCmd* _import_cmd; // associated import command
#if __GNUC__<3
  filebuf* _filebuf;
#else				 
  fileptr_filebuf* _filebuf;        // associated input buffer#
#endif
  istream* _inptr;          // associated input stream
  FILE* _infptr;            // associated FILE*
};

//: an ACE_Test_and_Set Singleton for Ctrl-C.
typedef ACE_Singleton<ACE_Test_and_Set <ACE_Null_Mutex, sig_atomic_t>, ACE_Null_Mutex> 
	IMPORT_QUIT_HANDLER;

//: acceptor specialized on UnidrawImportHandler.
typedef ACE_Acceptor <UnidrawImportHandler, ACE_SOCK_ACCEPTOR> 
	UnidrawImportAcceptor;

#endif /* _unidraw_import_handler_ */

#endif /* HAVE_ACE */
