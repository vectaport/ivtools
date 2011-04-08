/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#include <ComTerp/comhandler.h>

#include <ComTerp/_comterp.h>
#include <ComTerp/_comutil.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/ctrlfunc.h>
#include <OS/math.h>
#include <iostream.h>
#include <string.h>
#if __GNUC__>=3
#include <fstream.h>
#endif

#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

ComTerpServ::ComTerpServ(int bufsize, int fd)
: ComTerp()
{
    _bufsiz = bufsize;
    _instr = new char[_bufsiz];
    _outstr = new char[_bufsiz];
    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outptr = this;
    _outfunc = (outfuncptr)&ComTerpServ::s_fputs;
    _fd = fd;
    if (_fd>=0)
      _fptr = fdopen(_fd, "rw");
    else
      _fptr = stdin;
#ifdef HAVE_ACE
    _handler = nil;
#endif

    /* inform the parser which infunc is the oneshot infunc */
    _oneshot_infunc = (infuncptr)&s_fgets;

    _logger_mode = 0;
}

ComTerpServ::~ComTerpServ() {
    delete [] _instr;
    delete [] _outstr;
    if (_fptr != stdin)
      fclose(_fptr);
}

void ComTerpServ::load_string(const char* expr) {
    _inpos = 0;

    /* copy string into buffer, ensuring it ends with a newline */
    int count=0;
    char* inptr = _instr;
    char* exptr = (char*) expr;
    char ch;
    do {
        ch = *exptr++;
	*inptr++ = ch;
    } while (ch && count++<_bufsiz-2);
    if (!ch && count>0 && *(inptr-2) != '\n') {
	    *(inptr-1) = '\n';
	    *(inptr) = '\0';
    } else if (count==_bufsiz-2) {
            *(inptr) = '\n';
	    *(inptr+1) = '\0';
    }
}

char* ComTerpServ::s_fgets(char* s, int n, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    char* instr = server->_instr;
    char* outstr = s;
    int& inpos = server->_inpos;
    int& bufsize = server->_bufsiz;

    int outpos;

    /* copy characters until n-1 characters are transferred, */
    /* the input buffer is exhausted, or a newline is found. */
    for (outpos = 0; outpos < n-1 && inpos < bufsize-1 && instr[inpos] != '\n' && instr[inpos] != '\0';)
	outstr[outpos++] = instr[inpos++];

    /* copy the newline character if there is room */
    if (outpos < n-1 && inpos < bufsize-1 && instr[inpos] == '\n')
	outstr[outpos++] = instr[inpos++];

    /* append a null byte */
    outstr[outpos] = '\0';

    return s;
}

int ComTerpServ::s_feof(void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    int& inpos = server->_inpos;

    return inpos == -1;
}

int ComTerpServ::s_ferror(void* serv) {
    return 0;
}

int ComTerpServ::s_fputs(const char* s, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    char* outstr = server->_outstr;
    int& outpos = server->_outpos;
    int& bufsize = server->_bufsiz;

    for (; outpos < bufsize-1 && s[outpos]; outpos++)
	outstr[outpos] = s[outpos];
    outstr[outpos] = '\0';
    return 1;
}

char* ComTerpServ::fd_fgets(char* s, int n, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    int fd = Math::max(server->_fd, 1);
#if __GNUC__<3
    char* instr;
    filebuf fbuf;
    fbuf.attach(fd);
    istream in (&fbuf);
    in.gets(&instr);
#elif __GNUC__==3 && __GNUC_MINOR__<1
    char instr[BUFSIZ];
    FILE* ifptr = fdopen(fd, "r");
    fileptr_filebuf fbuf(ifptr, ios_base::in);
    istream in (&fbuf);
    in.get(instr, BUFSIZ, '\n');  // needs to be generalized with <vector.h>
#else
    char instr[BUFSIZ];
    fileptr_filebuf fbuf(fd, ios_base::in, false, static_cast<size_t>(BUFSIZ));
    istream in (&fbuf);
    in.get(instr, BUFSIZ, '\n');  // needs to be generalized with <vector.h>
#endif
    server->_instat = in.good(); 
  
    char* outstr = s;
    int inpos = 0;
    int& bufsize = server->_bufsiz;

    int outpos;

    /* copy characters until n-1 characters are transferred, */
    /* or the input buffer is exhausted */
    for (outpos = 0; outpos < n-1 && inpos < bufsize-1 && 
	   instr[inpos] != '\n' && instr[inpos] != '\0';)
	outstr[outpos++] = instr[inpos++];

    /* add a newline character if there is room */
    if (outpos < n-1 && inpos < bufsize-1)
	outstr[outpos++] = '\n';

    /* append a null byte */
    outstr[outpos] = '\0';

#if __GNUC__==3 && __GNUC_MINOR__<1
    if (ifptr) fclose(ifptr);
#endif

    return s;
}

int ComTerpServ::fd_fputs(const char* s, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    char* outstr = server->_outstr;
    int& outpos = server->_outpos;
    int& bufsize = server->_bufsiz;

    int fd = (int)server->_fd;
#if __GNUC__<3
    filebuf fbuf;
    fbuf.attach(fd);
#elif __GNUC__==3 && __GNUC_MINOR__<1
    FILE* ofptr = fdopen(fd, "w");
    fileptr_filebuf fbuf(ofptr, ios_base::out);
#else
    fileptr_filebuf fbuf(fd, ios_base::out, false, static_cast<size_t>(BUFSIZ));
#endif
    ostream out(&fbuf);
    for (; outpos < bufsize-1 && s[outpos]; outpos++)
	out.put(s[outpos]);
    out.flush();
    outpos = 0;
#if __GNUC__==3 && __GNUC_MINOR__<1
    if (ofptr) fclose(ofptr);
#endif
    return 1;
}

int ComTerpServ::run(boolean one_expr, boolean nested) {

    char buffer[BUFSIZ];
    char errbuf[BUFSIZ];
    errbuf[0] = '\0';
    int status = 0;

    _inptr = _fptr;
    _infunc = (infuncptr)&fgets;
    _eoffunc = (eoffuncptr)&ffeof;
    _errfunc = (errfuncptr)&fferror;
    _fd = handler() ? handler()->get_handle() : fileno(stdout);
    _outfunc = (outfuncptr)&fd_fputs;
    _linenum = 0;

#if 1
    ComTerp::run(one_expr, nested);
#else
    while (!feof(_fptr) && !quitflag()) {
	
	if (read_expr()) {
            err_str( errbuf, BUFSIZ, "comterp" );
	    if (strlen(errbuf)==0) {
		eval_expr();
		err_str( errbuf, BUFSIZ, "comterp" );
		if (strlen(errbuf)==0) {
		    if (quitflag()) {
		        status = -1;
			break;
		    } else
			print_stack_top();
		    err_str( errbuf, BUFSIZ, "comterp" );
		}
	    } 
            if (strlen(errbuf)>0) {
		cout << errbuf << "\n";
    		errbuf[0] = '\0';
		status = -1;
            }
	}
    }
#endif

    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outptr = this;
    _outfunc = (outfuncptr)&ComTerpServ::s_fputs;
    return status;
}

int ComTerpServ::runfile(const char* filename) {
    /* save enough state as needed by this interpreter */
#if 0
    void* save_inptr = _inptr;
    infuncptr save_infunc = _infunc;
    outfuncptr save_outfunc = _outfunc;
    eoffuncptr save_eoffunc = _eoffunc;
    errfuncptr save_errfunc = _errfunc;
    int save_linenum = _linenum;
#else
    push_servstate();
#endif
    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outfunc = nil;
    _linenum = 0;

    const int bufsiz = BUFSIZ*BUFSIZ;
    char inbuf[bufsiz];
    char outbuf[bufsiz];
    inbuf[0] = '\0';
#if __GNUC__<3
    filebuf ibuf;
    ibuf.open(filename, "r");
#else
    fileptr_filebuf ibuf(fopen(filename, "r"), ios_base::in);
#endif
    istream istr(&ibuf);
    ComValue* retval = nil;
    int status = 0;
   

    /* save tokens to restore after the file has run */
    int toklen;
    postfix_token* tokbuf = copy_postfix_tokens(toklen);
    int tokoff = _pfoff;
    
    while( istr.good()) {
        istr.getline(inbuf, bufsiz-1);
	if (istr.eof())
	  break;
	if (*inbuf) load_string(inbuf);
	if (*inbuf && read_expr()) {
	    if (eval_expr(true)) {
	        err_print( stderr, "comterp" );
#if __GNUC__<3
	        filebuf obuf(handler() ? handler()->get_handle() : 1);
#elif __GNUC__==3 && __GNUC_MINOR__<1
                FILE* ofptr = fdopen(handler() ? handler()->get_handle() : 1, "w"); 
	        fileptr_filebuf obuf(ofptr, ios_base::out);
#else
		fileptr_filebuf obuf(handler()&&handler()->get_handle()>0 
				     ? (int)handler()->get_handle() : 1, 
				     ios_base::out, false, static_cast<size_t>(BUFSIZ));
#endif
		ostream ostr(&obuf);
		ostr.flush();
#if __GNUC__==3 && __GNUC_MINOR__<1
                if (ofptr) fclose(ofptr);
#endif
		status = -1;
	    } else if (quitflag()) {
	        status = 1;
	        break;
	    } else {
	        /* save last thing on stack */
	        retval = new ComValue(pop_stack());
	    }
	} else 	if (*inbuf) {
	  err_print( stderr, "comterp" );
#if __GNUC__<3
	  filebuf obuf(handler() ? handler()->get_handle() : 1);
#elif __GNUC__==3 && __GNUC_MINOR__<1
          FILE* ofptr = fdopen(handler() ? handler()->get_handle() : 1, "w"); 
	  fileptr_filebuf obuf(ofptr, ios_base::out);
#else
	  fileptr_filebuf obuf(handler()&&handler()->get_handle()>0 
			       ? (int)handler()->get_handle() : 1, 
			       ios_base::out, false, static_cast<size_t>(BUFSIZ));

#endif
	  ostream ostr(&obuf);
	  ostr.flush();
#if __GNUC__==3 && __GNUC_MINOR__<1
          if (ofptr) fclose(ofptr);
#endif
	  status = -1;
	}
    }

    load_postfix(tokbuf, toklen, tokoff);
    delete tokbuf;

    if (retval) {
        push_stack(*retval);
	delete retval;
    } else
        push_stack(ComValue::nullval());

#if 0
    _inptr = save_inptr;
    _infunc = save_infunc;
    _outfunc = save_outfunc;
    _eoffunc = save_eoffunc;
    _errfunc = save_errfunc;
    _linenum = save_linenum;
#else
    pop_servstate();
#endif

    return status;
}

ComValue& ComTerpServ::run(const char* expression, boolean nested) {
    _errbuf[0] = '\0';

#if 0
    postfix_token* save_pfbuf = _pfbuf;
    int save_pfoff = _pfoff;
    int save_pfnum = _pfnum;
    int save_bufptr = _bufptr;
    int save_linenum = _linenum;
    int save_just_reset = _just_reset;
    char* save_buffer = _buffer;
#else
    push_servstate();
#endif
    _buffer = new char[_bufsiz];
    _bufptr = 0;
    _buffer[_bufptr] = '\0';
#if 0
    if (save_pfoff) {
#endif
      _pfbuf =  new postfix_token[_pfsiz];
      _pfoff = 0;
#if 0
    }
    ComValue* save_pfcomvals = _pfcomvals;
#endif
    _pfcomvals = nil;

    if (expression) {
        load_string(expression);
#if 0
	infuncptr save_infunc = _infunc;
	eoffuncptr save_eoffunc = _eoffunc;
	errfuncptr save_errfunc = _errfunc;
	void* save_inptr = _inptr;
#endif
	_infunc = (infuncptr)&ComTerpServ::s_fgets;
	_eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
	_errfunc = (errfuncptr)&ComTerpServ::s_ferror;
	_inptr = this;
        read_expr();
#if 0
	_infunc = save_infunc;
	_eoffunc = save_eoffunc;
	_errfunc = save_errfunc;
	_inptr = save_inptr;
#endif
        err_str(_errbuf, BUFSIZ, "comterp");
    }
    if (!*_errbuf) {
	eval_expr(nested);
	err_str(_errbuf, BUFSIZ, "comterp");
    }

#if 0
    _pfnum = save_pfnum;
    _bufptr = save_bufptr;
    delete _buffer;
    _buffer = save_buffer;
    _linenum = save_linenum;
    _just_reset = save_just_reset;
    if (save_pfoff) {
      delete _pfbuf;
      _pfbuf =  save_pfbuf;
      _pfoff = save_pfoff;
    }
    delete [] _pfcomvals;
    _pfcomvals = save_pfcomvals;
#else
    pop_servstate();
#endif

    return *_errbuf ? ComValue::nullval() : pop_stack();
}

ComValue& ComTerpServ::run(postfix_token* tokens, int ntokens) {
    _errbuf[0] = '\0';

#if 0
    postfix_token* save_pfbuf = _pfbuf;
    int save_pfnum = _pfnum;
    int save_pfoff = _pfoff;
    int save_bufptr = _bufptr;
    int save_linenum = _linenum;
    int save_just_reset = _just_reset;
#else
    push_servstate();
#endif
    _pfbuf = tokens;
    _pfnum = ntokens;
    _pfoff = 0;

    eval_expr();
    err_str(_errbuf, BUFSIZ, "comterp");

    ComValue& retval = *_errbuf ? ComValue::nullval() : pop_stack();
#if 0
    _pfbuf = save_pfbuf;
    _pfnum = save_pfnum;
    _pfoff = save_pfoff;
    _bufptr = save_bufptr;
    _linenum = save_linenum;
    _just_reset = save_just_reset;
#else
    _pfbuf = nil;
    pop_servstate();
#endif
    return retval;
}

postfix_token* ComTerpServ::gen_code(const char* script, int& ntoken) {
    load_string(script);
    read_expr();
    return copy_postfix_tokens(ntoken);
}

void ComTerpServ::read_string(const char* script) {
    load_string(script);
    read_expr();
}

void ComTerpServ::add_defaults() {
  if (!_defaults_added) {
    ComTerp::add_defaults();
    add_command("remote", new RemoteFunc(this));
    add_command("eval", new EvalFunc(this));
  }
}
