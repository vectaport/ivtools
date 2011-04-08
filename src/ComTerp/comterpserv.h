/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

/*
 * ComTerpServ is a ComTerp with added ability to load its input from a buffer.
 */

#ifndef comterpserv_h
#define comterpserv_h

/*
 * Server-oriented interpreter deals with strings
 */

#include <ComTerp/comterp.h>

class ComTerpServState;

//: extended ComTerp that works with buffered IO.
class ComTerpServ : public ComTerp {
public:
    ComTerpServ(int bufsize = 1024*1024, int fd = -1);
    // construct with optional 'bufsize', and on an optional 'fd'.
    ~ComTerpServ();

    void load_string(const char*);
    // load string to be interpreted into buffer.
    void read_string(const char*);
    // load string to be interpreted into buffer, and read postfix
    // tokens from it.
    postfix_token* gen_code(const char*, int& codelen);
    // generate buffer of length 'codelen' of postfix tokens ready
    // to be converted into ComValue objects and executed.

    virtual int run(boolean one_expr=false, boolean nested=false);
    // run this interpreter until quit or exit command.
    virtual ComValue& run(const char*, boolean nested=false);
    // interpret and return value of expression.  'nested' flag used
    // to indicated nested call to the run() method, to avoid
    // re-initialization.
    virtual ComValue& run(postfix_token*, int);
    // execute a buffer of postfix tokens and return the value.
    
    virtual int runfile(const char*);
    // run interpreter on commands read from a file.

    void add_defaults();
    // add a default list of ComFunc objects to this interpreter.

    virtual boolean is_serv() { return true; } 
    // flag to test if ComTerp or ComTerpServ

    int& npause() { return _npause; }
    // return (reference to) number of pauses

    ComTerpServState* top_servstate();
    // return pointer to top state on ComTerpServ state stack

    void push_servstate();
    // push ComTerpServ state for later retrieval

    void pop_servstate();
    // pop ComTerpServ state that was saved earlier

protected:

    static char* s_fgets(char* s, int n, void* serv);
    // signature like fgets used to copy input from a buffer.
    static int s_feof(void* serv);
    // signature like feof used to relay end-of-file.
    static int s_ferror(void* serv);
    // signature like ferror used to relay error info.
    static int s_fputs(const char* s, void* serv);
    // signature like fputs used to copy output back to buffer.
    static char* fd_fgets(char* s, int n, void* serv);
    // signature like fgets used to explicitly read from an filedescriptor.
    static int fd_fputs(const char* s, void* serv);
    // signature like fputs used to explicitly read from an filedescriptor.

protected:
    char* _instr;
    int _inpos;
    char* _outstr;
    int _outpos;
    int _fd;
    FILE* _fptr;
    int _instat;
    int _npause;
    int _logger_mode;

    ComTerpServState* _ctsstack;  // stack of ComTerpServ state
    int _ctsstack_top;
    unsigned int _ctsstack_siz;

    friend class ComterpHandler;
    friend class ComTerpIOHandler;
};

//: object for holding ComTerpServ state
// object that holds the state of a ComTerpServ
// which allows for nested and recursive use of a singular ComTerpServ
class ComTerpServState {
public:
  ComTerpServState() {}
  ComTerpServState(ComTerpServState& ctss) { *this = ctss; }
  // copy constructor.

  postfix_token*& pfbuf() { return _pfbuf; }
  int& pfnum() { return _pfnum; }
  int& pfoff() { return _pfoff; }
  int& bufptr() { return _bufptr; }
  int& bufsiz() { return _bufsiz; }
  int& linenum() { return _linenum; }
  int& just_reset() { return _just_reset; }
  char*& buffer() { return _buffer; }
  ComValue*& pfcomvals() { return _pfcomvals; }
  infuncptr& infunc() { return _infunc; }
  eoffuncptr& eoffunc() { return _eoffunc; }
  errfuncptr& errfunc() { return _errfunc; }
  void*& inptr() { return _inptr; }

protected:

  postfix_token* _pfbuf;
  int _pfnum;
  int _pfoff;
  int _bufptr;
  int _linenum;
  int _just_reset;
  char* _buffer;
  int _bufsiz;
  ComValue* _pfcomvals;
  infuncptr _infunc;
  eoffuncptr _eoffunc;
  errfuncptr _errfunc;
  void* _inptr;
};
#endif
