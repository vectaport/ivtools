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
#include <ComTerp/strmfunc.h>
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

// #define TIMING_TEST
#if defined(TIMING_TEST)
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
  long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
  result->tv_sec = diff / 1000000;
  result->tv_usec = diff % 1000000;

  return (diff<0);
}

/* Return 1 if the sum is negative, otherwise 0.  */
int timeval_add(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
  long int sum = (t2->tv_usec + 1000000 * t2->tv_sec) + (t1->tv_usec + 1000000 * t1->tv_sec);
  result->tv_sec = sum / 1000000;
  result->tv_usec = sum % 1000000;

  return (sum<0);
}

void timeval_print(struct timeval *tv)
{
  char buffer[30];
  time_t curtime;

  printf("%ld.%06ld\n", tv->tv_sec, tv->tv_usec);
  curtime = tv->tv_sec;
#if 0
  strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
  printf(" = %s.%06ld\n", buffer, tv->tv_usec);
#endif
}
#endif /* defined(TIMING_TEST) */

ComTerpServ::ComTerpServ(int linesize, int fd)
: ComTerp()
{
    _linesize = linesize;
    _instr = new char[_linesize];
    _outstr = new char[_linesize];
    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outptr = this;
    _outfunc = (outfuncptr)&ComTerpServ::s_fputs;
    _fd = fd;
    _outpos = 0;
    if (_fd>=0)
      _fptr = fdopen(_fd, "rw");
    else
      _fptr = stdin;
#ifdef HAVE_ACE
    _handler = nil;
#endif

    /* inform the parser which infunc is the oneshot infunc */
    _oneshot_infunc = (infuncptr)&s_fgets;
  
    /* initialize shadow copy too */
    __oneshot_infunc = (infuncptr)&s_fgets;

    _logger_mode = 0;
    _delete_later = 0;
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
    } while (ch && count++<_linesize-2);
    if (!ch && count>0 && *(inptr-2) != '\n') {
	    *(inptr-1) = '\n';
	    *(inptr) = '\0';
    } else if (count==_linesize-2) {
            *(inptr) = '\n';
	    *(inptr+1) = '\0';
    }
}

char* ComTerpServ::s_fgets(char* s, int n, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    char* instr = server->_instr;
    char* outstr = s;
    int& inpos = server->_inpos;
    int& linesize = server->_linesize;

    int outpos;

    /* copy characters until n-1 characters are transferred, */
    /* the input buffer is exhausted, or a newline is found. */
    for (outpos = 0; outpos < n-1 && inpos < linesize-1 && instr[inpos] != '\n' && instr[inpos] != '\0';)
	outstr[outpos++] = instr[inpos++];

    /* copy the newline character if there is room */
    if (outpos < n-1 && inpos < linesize-1 && instr[inpos] == '\n')
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
    int& linesize = server->_linesize;

    for (; outpos < linesize-1 && s[outpos]; outpos++)
	outstr[outpos] = s[outpos];
    outstr[outpos] = '\0';
    return 1;
}

char* ComTerpServ::fd_fgets(char* s, int n, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    int fd = Math::max(server->_fd, 1);
    char instr[BUFSIZ];
    FILE* ifptr = fd==0 ? stdin : server->handler()->rdfptr();
    fileptr_filebuf fbuf(ifptr, ios_base::in);
    istream in (&fbuf);
    in.get(instr, BUFSIZ, '\n');  // needs to be generalized with <vector.h>
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

    return s;
}

int ComTerpServ::fd_fputs(const char* s, void* serv) {
    ComTerpServ* server = (ComTerpServ*)serv;
    char* outstr = server->_outstr;
    int& outpos = server->_outpos;
    int& bufsize = server->_bufsiz;

    int fd = (int)server->_fd;
    FILE* ofptr = fd==1 ? stdout : server->handler()->wrfptr();
    fileptr_filebuf fbuf(ofptr, ios_base::out);
    ostream out(&fbuf);
    for (; outpos < bufsize-1 && s[outpos]; outpos++)
	out.put(s[outpos]);
    out.flush();
    outpos = 0;
    return 1;
}

int ComTerpServ::run(boolean one_expr, boolean nested) {
    char buffer[_linesize];
    char errbuf[_linesize];
    errbuf[0] = '\0';
    int status = 0;

    _inptr = _fptr;
    _infunc = (infuncptr)&fgets;
    _eoffunc = (eoffuncptr)&ffeof;
    _errfunc = (errfuncptr)&fferror;
    _fd = handler() ? handler()->get_handle() : (_fd > 0 ? _fd : fileno(stdout));
    _outfunc = (outfuncptr)&fd_fputs;
    _linenum = 0;

    ComTerp::run(one_expr, nested);

    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outptr = this;
    _outfunc = (outfuncptr)&ComTerpServ::s_fputs;
    return status;
}

int ComTerpServ::runfile(const char* filename, boolean popen_flag) {
    /* save enough state as needed by this interpreter */
    push_servstate();
    _inptr = this;
    _infunc = (infuncptr)&ComTerpServ::s_fgets;
    _eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
    _errfunc = (errfuncptr)&ComTerpServ::s_ferror;
    _outfunc = nil;
    _linenum = 0;

    _linesize = BUFSIZ*BUFSIZ;
    char inbuf[_linesize];
    char outbuf[_linesize];
    inbuf[0] = '\0';
    FILE* ifptr = NULL;
    ifptr = popen_flag ? popen(filename, "r") : fopen(filename, "r");
    if (!ifptr) {
      fprintf(stderr, "Unable to open file %s to run\n", filename);
      pop_servstate();
      return -1;
    }
    fileptr_filebuf ibuf(ifptr, ios_base::in);
    istream istr(&ibuf);
    ComValue* retval = nil;
    int status = 0;
   

    /* save tokens to restore after the file has run */
    int toklen;
    postfix_token* tokbuf = copy_postfix_tokens(toklen);
    int tokoff = _pfoff;

    int last_status = 0;
    while( istr.good()) {
#if defined(TIMING_TEST)
        static struct timeval tvBefore, tvAfter, tvParse, tvConvert, tvDiff;
#endif
        *inbuf='\0';
        istr.getline(inbuf, _linesize-1);
	if (istr.eof() && !*inbuf)  // deal with last line without new-line
	  break;
        if (_linenum==0 && !*inbuf) { // run a dummy space in to initialize parser
            inbuf[0]=' ';
            inbuf[1]='\0';
        }
	if (*inbuf)
            load_string(inbuf);
        else
            increment_linenum();
	if (*inbuf && (last_status=read_expr())) {
#if defined(TIMING_TEST)
	    static int initialized=0;
	    if (!initialized) {
	      initialized = 1;
	      gettimeofday(&tvBefore, NULL);
	      gettimeofday(&tvAfter, NULL);
	      timeval_subtract(&tvParse, &tvBefore, &tvBefore);
	      timeval_subtract(&tvConvert, &tvBefore, &tvBefore);
	    }
	    gettimeofday(&tvAfter, NULL);
	    timeval_subtract(&tvDiff, &tvAfter, &tvBefore);
	    timeval_add(&tvParse, &tvDiff, &tvParse);
	    tvBefore = tvAfter;
	    fprintf(stderr, "Parse Time:  ");
	    timeval_print(&tvParse);
#endif
	    if (eval_expr(true)) {
	        err_print( stderr, "comterp" );
                FILE* ofptr = handler() ? handler()->wrfptr() : stdout; 
	        fileptr_filebuf obuf(ofptr, ios_base::out);
		ostream ostr(&obuf);
		ostr.flush();
		status = -1;
	    } else if (quitflag()) {
                delete retval; retval = nil; // remove prior retval
	        status = 1;
	        break;
	    } else if (!func_for_next_expr() && val_for_next_func().is_null() /* && muted()!=1 */) {
#if defined(TIMING_TEST)
	        gettimeofday(&tvAfter, NULL);
		timeval_subtract(&tvDiff, &tvAfter, &tvBefore);
		timeval_add(&tvConvert, &tvDiff, &tvConvert);
		tvBefore = tvAfter;
		fprintf(stderr, "Convert Time:  ");
		timeval_print(&tvConvert);
#endif
	        if (stack_top().is_stream() && autostream()) {
		  ComValue streamv(stack_top());
		  do {
		    pop_stack();
		    NextFunc::execute_impl(this, streamv);
		  } while (stack_top().is_known());
		  pop_stack();
		} else {
		  /* save last thing on stack */  
		  if(retval) delete retval;
		  retval = new ComValue(pop_stack());
		}
	    }

	    
	} else 	if (*inbuf) {
	  err_print( stderr, "comterp" );
          FILE* ofptr = handler() ? handler()->wrfptr() : stdout; 
	  fileptr_filebuf obuf(ofptr, ios_base::out);
	  ostream ostr(&obuf);
	  ostr.flush();
	  status = -1;
	}

    }

    if(last_status==0 && *inbuf) {
        COMERR_SET1( ERR_UNEXPECTED_EOF, _linenum );
        *inbuf = '\0';
        parser_reset();
    }

    load_postfix(tokbuf, toklen, tokoff);
    delete tokbuf;
    ibuf.close();
    if (ifptr) 
        if(popen_flag)
            pclose(ifptr);
        else
            fclose(ifptr);

    if (retval) {
        push_stack(*retval);
	delete retval;
    } else
        if (!quitflag()) 
            push_stack(ComValue::nullval());

    pop_servstate();

    return status;
}

ComValue ComTerpServ::run(const char* expression, boolean nested) {
    _errbuf[0] = '\0';

    push_servstate();
    _pfcomvals = nil;

    if (expression) {
        load_string(expression);
	_infunc = (infuncptr)&ComTerpServ::s_fgets;
	_eoffunc = (eoffuncptr)&ComTerpServ::s_feof;
	_errfunc = (errfuncptr)&ComTerpServ::s_ferror;
	_inptr = this;
        read_expr();
        err_str(_errbuf, BUFSIZ, "comterp");
    }
    if (!*_errbuf) {
	eval_expr(nested);
	err_str(_errbuf, BUFSIZ, "comterp");
    }

    pop_servstate();

    return *_errbuf ? ComValue::nullval() : pop_stack();
}

ComValue ComTerpServ::run(postfix_token* tokens, int ntokens) {
    _errbuf[0] = '\0';

    push_servstate();
    _pfbuf = tokens;
    _pfnum = ntokens;
    _pfoff = 0;

    eval_expr();
    err_str(_errbuf, BUFSIZ, "comterp");

    ComValue retval(*_errbuf ? ComValue::nullval() : pop_stack());
    _pfbuf = nil;
    pop_servstate();
    return retval;
}

postfix_token* ComTerpServ::gen_code(const char* script, int& ntoken) {
    push_servstate();
    load_string(script);
    read_expr();
    postfix_token* copied_tokens = copy_postfix_tokens(ntoken);
    pop_servstate();
    return copied_tokens;
}

void ComTerpServ::read_string(const char* script) {
    load_string(script);
    read_expr();
}

void ComTerpServ::add_defaults() {
  if (!_defaults_added) {
    ComTerp::add_defaults();
    add_command("remote", new RemoteFunc(this));
    add_command("socket", new SocketFunc(this));
    add_command("eval", new EvalFunc(this));
  }
}
