/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#include <ComUnidraw/comterp-iohandler.h>

#include <Unidraw/unidraw.h>

#include <ComTerp/comterpserv.h>

#include <Dispatch/dispatcher.h>


/*****************************************************************************/

ComTerpIOHandler::ComTerpIOHandler(ComTerpServ* comterp, FILE* fptr)
{
    _fptr = fptr;
    _fd = fileno(fptr);
    _fptr_opened = false;
    _comterp = comterp;
    _buffer = new char[BUFSIZ];
    link();
}

ComTerpIOHandler::ComTerpIOHandler(ComTerpServ* comterp, int fd) 
{
    _fd = fd;
    _fptr = fdopen(fd, "r");
    _fptr_opened = true;
    _comterp = comterp;
    _buffer = new char[BUFSIZ];
    link();
}

ComTerpIOHandler::~ComTerpIOHandler() {
    unlink();
    delete _buffer;
    if (_fptr_opened) 
	fclose(_fptr);
}

int ComTerpIOHandler::inputReady(int i) 
{
    /* invoke comterp to crank on one line */
#if 1   
    fgets( _buffer, BUFSIZ, _fptr);
    if (feof(_fptr)) return -1;
    _comterp->load_string(_buffer);
#else
    _comterp->_infunc = (infuncptr)&ComTerpServ::fd_fgets;
#endif
    _comterp->_fd = i;
    _comterp->_outfunc = (outfuncptr)&ComTerpServ::fd_fputs;

    boolean done = false;
    while (!done) {
      if (_comterp->read_expr()) {
	if (_comterp->eval_expr())
	  err_print( stderr, "comterp" );
	else if (_comterp->quitflag()) 
	  return 0;
	else {
	  if (unidraw->updated()) unidraw->Update(true);
	  _comterp->print_stack_top();
	}
      } else {
	if (err_cnt()>0) 
	  err_print( stderr, "comterp");
	done = 1;
      }
    }
    return 0;
}

void ComTerpIOHandler::link() 
{
    Dispatcher::instance().link
	(_fd, Dispatcher::ReadMask, this);
}

void ComTerpIOHandler::unlink() 
{
    Dispatcher::instance().unlink(_fd);
}


