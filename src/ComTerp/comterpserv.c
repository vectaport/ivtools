/*
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
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/_comterp.h>
#include <streambuf.h>
#include <string.h>

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
    for (outpos = 0; outpos < n-1 && inpos < bufsize-1 && instr[inpos] != '\n';)
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

int ComTerpServ::run() {

    char buffer[BUFSIZ];
    char errbuf[BUFSIZ];
    errbuf[0] = '\0';
    int status = 0;

    while (!feof(_fptr) && !quitflag()) {
	
        fgets( buffer, BUFSIZ, _fptr);
        load_string(buffer);
	
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
    return status;
}

int ComTerpServ::runfile(const char* filename) {
    const int bufsiz = BUFSIZ;
    char inbuf[bufsiz];
    char outbuf[bufsiz];
    inbuf[0] = '\0';
    filebuf ibuf;
    ibuf.open(filename, "r");
    istream istr(&ibuf);
    ComValue* retval = nil;
    int status = 0;

    /* save tokens to restore after the file has run */
    int toklen;
    postfix_token* tokbuf = copy_postfix_tokens(toklen);
    int tokoff = _pfoff;
    
    while( !istr.eof()) {
        istr.getline(inbuf, bufsiz-1);
	load_string(inbuf);
	if (read_expr()) {
	    if (eval_expr(true)) {
	        err_print( stderr, "comterp" );
	        filebuf obuf(handler() ? handler()->get_handle() : 1);
		ostream ostr(&obuf);
		ostr << "err\n";
		ostr.flush();
		status = -1;
	    } else if (quitflag()) {
	        status = 1;
	        break;
	    } else {
	        /* save last thing on stack */
	        retval = new ComValue(pop_stack());
	    }
	}
    }

    load_postfix(tokbuf, toklen, tokoff);
    delete tokbuf;

    if (retval) {
        push_stack(*retval);
	delete retval;
    } else
        push_stack(ComValue::nullval());

    return status;
}

ComValue& ComTerpServ::run(const char* expression) {
    _errbuf[0] = '\0';

    if (expression) {
        load_string(expression);
        read_expr();
        err_str(_errbuf, BUFSIZ, "comterp");
    }
    if (!*_errbuf) {
	eval_expr();
	err_str(_errbuf, BUFSIZ, "comterp");
    }
    return *_errbuf ? ComValue::nullval() : pop_stack();
}

ComValue& ComTerpServ::run(postfix_token* tokens, int ntokens) {
    _errbuf[0] = '\0';

    _pfoff = 0;

    postfix_token* save_pfbuf = _pfbuf;
    int save_pfnum = _pfnum;

    _pfbuf = tokens;
    _pfnum = ntokens;

    eval_expr();
    err_str(_errbuf, BUFSIZ, "comterp");

    ComValue& retval = *_errbuf ? ComValue::nullval() : pop_stack();
    _pfbuf = save_pfbuf;
    _pfnum = save_pfnum;
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
  
