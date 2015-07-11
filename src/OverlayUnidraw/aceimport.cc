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

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <OverlayUnidraw/aceimport.h>

#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovimport.h>
#include <IVGlyph/importchooser.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <ComTerp/comhandler.h>
#include <fstream.h>
#include <stdio.h>

#include <iostream>
using std::cout;
using std::cerr;

/*****************************************************************************/

UnidrawImportHandler::UnidrawImportHandler ()
{
    Iterator i;
    unidraw->First(i);
    OverlayEditor* ed = (OverlayEditor*)unidraw->GetEditor(i);
    OverlayIdrawComp* topcomp = (OverlayIdrawComp*) ed->GetComponent();
    topcomp->ResetIndexedGS();
    _import_cmd = new OvImportCmd(unidraw->GetEditor(i), &ImportChooser::instance());
    _inptr = nil;
    _filebuf = nil;
    _infptr = nil;
}

void
UnidrawImportHandler::destroy (void)
{
    cerr << this->peer_name_ << " disconnected from import port\n";
#if 0
    ComterpHandler::reactor_singleton()->cancel_timer (this);
#endif
    this->peer ().close ();
    if (_infptr) {
      fclose(_infptr);
      _infptr = nil;
    }
}

int
UnidrawImportHandler::handle_timeout (const ACE_Time_Value &,
				 const void *arg)
{
    ACE_ASSERT (arg == this);
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) handling timeout from this = %u\n", this));
    return 0;
}

int
UnidrawImportHandler::handle_input (ACE_HANDLE fd)
{
    if (!_infptr) _infptr = fdopen(fd, "r");
    FILEBUF(fbuf, _infptr, ios_base::in);
    istream in(&fbuf);
    int ch = in.get();
    if (ch != EOF && in.good()) {
      in.unget();
      _import_cmd->instream(&in);
      _import_cmd->Execute();
    }
    return -1;  /* only return -1, which indicates input handling is fini */
}

int
UnidrawImportHandler::open (void *)
{
  ACE_INET_Addr addr;
  
  if (this->peer ().get_remote_addr (addr) == -1)
    return -1;
  else
    {
      ACE_OS::strncpy (this->peer_name_, 
		       addr.get_host_name (), 
		       MAXHOSTNAMELEN + 1);

      if (ComterpHandler::reactor_singleton()->register_handler 
	  (this, ACE_Event_Handler::READ_MASK) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "(%P|%t) can't register with reactor\n"), -1);
#if 0
      else if (ComterpHandler::reactor_singleton()->schedule_timer
	  (this, 
	  (const void *) this, 
	   ACE_Time_Value (10), 
	   ACE_Time_Value (10)) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "can'(%P|%t) t register with reactor\n"), -1);
#endif
      else
        cerr << this->peer_name_ << " connected to import port\n";
      return 0;
    }
}

int
UnidrawImportHandler::close (u_long)
{
  delete _filebuf;
  delete _inptr;
  this->destroy ();
  return 0;
}

#endif /* HAVE_ACE */
