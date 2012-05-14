/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1998,1999 Vectaport Inc.
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
 * collection of io functions
 */

#if !defined(_iofunc_h)
#define _iofunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class ComValue;

//: value printing command for ComTerp.
// [str]=print([fmtstr] [val [val1 [... valn]]] :string|:str :symbol|:sym :out :err :file fileobj :prefix str) -- print value(s) with optional format string
class PrintFunc : public ComFunc {
public:
    PrintFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "[str]=%s([fmtstr] [val [val1 [... valn]]] :string|:str :symbol|:sym :out :err :file fileobj :prefix str) -- print value with optional format string"; }

    static int format_extent(const char* fstr);
};

//: open file command
// fileobj=open([filename modestr] :pipe :out :err) -- open file command
class OpenFileFunc : public ComFunc {
public:
    OpenFileFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "fileobj=open([filename [modestr]] :pipe :in :out :err) -- open file command"; }
};

//: close file command
// close(fileobj) -- close file command
class CloseFileFunc : public ComFunc {
public:
    CloseFileFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "close(fileobj) -- close file command"; }
};

class FileObj {
 public:
  FileObj(const char* filename, const char* mode, int pipeflag);
  FileObj(FILE* fptr);
  virtual ~FileObj();
  const char* filename() { return _filename; }
  const char* mode() { return _mode; }
  FILE* fptr() { return _fptr; }

 protected:
  char* _filename;
  char* _mode;
  FILE* _fptr;
  int _pipe;

  CLASS_SYMID("FileObj");
};


//: get string from FileObj
// str=gets(fileobj) -- gets a new-line terminated string from a file
class GetStringFunc : public ComFunc {
public:
    GetStringFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "str=%s(fileobj) -- gets a new-line terminated string from a file"; }
};

//: return command line argument
// str=arg(n) -- return command line argument
class GetArgFunc : public ComFunc {
public:
    GetArgFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "str=%s(n) -- return command line argument"; }
};

//: return number of command line arguments
// n=narg() -- return number of command line arguments
class NumArgFunc : public ComFunc {
public:
    NumArgFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "n=%s() -- return number of command line arguments"; }
};

#endif /* !defined(_iofunc_h) */
