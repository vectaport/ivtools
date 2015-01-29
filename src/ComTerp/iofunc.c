/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1999 Vectaport Inc.
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

#include <ComTerp/iofunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>
#include <OS/math.h>
#include <iostream.h>
#include <strstream>
#if __GNUC__>=3
#include <fstream.h>
#endif
#include <streambuf>

#define TITLE "IoFunc"

using std::streambuf;

/*****************************************************************************/
int FileObj::_symid = -1;

FileObj::FileObj(const char* filename, const char* mode, int pipeflag) {
  _filename = strnew(filename);
  _mode = strnew(mode);
  _pipe = pipeflag;
  _fptr = _pipe ? popen(filename, mode) : fopen(filename, mode);
}

FileObj::FileObj(FILE* fptr) {
  _filename = NULL;
  _mode = NULL;
  _pipe = false;
  _fptr = fptr;
}

FileObj::~FileObj() { 
  if( _fptr && _filename) _pipe ? pclose(_fptr) : fclose(_fptr);
  delete _filename;
  delete _mode;
}

/*****************************************************************************/
int PipeObj::_symid = -1;

PipeObj::PipeObj(const char* command) {
  _command = strnew(command);
  _pid = popen2(command, &_wrfd, &_rdfd);
  _wrfptr = _rdfptr = NULL;
}

void PipeObj::close() {
  if(_wrfptr) fclose(_wrfptr);
  if(_rdfptr) fclose(_rdfptr);
  ::close(_wrfd);
  ::close(_rdfd);
  _wrfptr = NULL;
  _rdfptr = NULL;
  _wrfd = -1;
  _rdfd = -1;
}

PipeObj::~PipeObj() { 
  delete _command;
  close();
}

/*****************************************************************************/

PrintFunc::PrintFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PrintFunc::execute() {
  ComValue formatstr(stack_arg(0));
  static int str_symid = symbol_add("str");
  ComValue strflag(stack_key(str_symid));
  static int string_symid = symbol_add("string");
  ComValue stringflag(stack_key(string_symid));
  static int sym_symid = symbol_add("sym");
  ComValue symflag(stack_key(sym_symid));
  static int symbol_symid = symbol_add("symbol");
  ComValue symbolflag(stack_key(symbol_symid));
  static int err_symid = symbol_add("err");
  ComValue errflag(stack_key(err_symid));
  static int out_symid = symbol_add("out");
  ComValue outflag(stack_key(out_symid));
  static int file_symid = symbol_add("file");
  ComValue fileobjv(stack_key(file_symid));
  static int prefix_symid = symbol_add("prefix");
  ComValue prefixv(stack_key(prefix_symid));

  const char* fstr = formatstr.is_string() ? formatstr.string_ptr() : "nil";
  ComValue::comterp(comterp());

#if __GNUC__<3
  streambuf* strmbuf = nil;
  if (stringflag.is_false() && strflag.is_false() &&
      symbolflag.is_false() && symflag.is_false()) {
    filebuf * fbuf = new filebuf();
    strmbuf = fbuf;
    if (comterp()->handler()) {
      int fd = Math::max(1, comterp()->handler()->get_handle());
      fbuf->attach(fd);
    } else
      fbuf->attach(fileno(errflag.is_false() ? stdout : stderr));
  } else {
    strmbuf = new std::strstreambuf();
  }
#else
  streambuf* strmbuf = nil;
  if (stringflag.is_false() && strflag.is_false() &&
      symbolflag.is_false() && symflag.is_false()) {
    fileptr_filebuf * fbuf = nil;
    if (comterp()->handler() && fileobjv.is_unknown() && errflag.is_false() && outflag.is_false()) {
      fbuf = new fileptr_filebuf(comterp()->handler() && comterp()->handler()->wrfptr() 
			 ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
    } else if (fileobjv.is_known()) {
      FileObj *fileobj = (FileObj*)fileobjv.geta(FileObj::class_symid());
      if (fileobj) 
        fbuf = new fileptr_filebuf(fileobj->fptr(), ios_base::out);
      else {
        PipeObj *pipeobj = (PipeObj*)fileobjv.geta(PipeObj::class_symid());
        fbuf = new fileptr_filebuf(pipeobj ? pipeobj->wrfptr() : stdout, ios_base::out);
      }
    } else 
      fbuf = new fileptr_filebuf(errflag.is_false() ? stdout : stderr, ios_base::out);
    strmbuf = fbuf;
  } else
    strmbuf = new std::strstreambuf();
#endif
  ostream out(strmbuf);

  int narg = nargsfixed();
  if (narg==1) {

    if (formatstr.is_string() && !prefixv.is_string())
      out << formatstr.symbol_ptr();
    else {
      if (prefixv.is_string()) out << prefixv.symbol_ptr();
      out << formatstr;  // which could be arbitrary ComValue
      if (prefixv.is_string()) out << "\n";
    }

  } else {
    const char* fstrptr = fstr;
    int curr=1;
    while (curr<narg) {

      char fbuf[BUFSIZ];
      ComValue printval(stack_arg(curr));
      curr++;

      int i=0;
      if(curr<narg) {
        int flen;
        while(*fstrptr && !(flen=format_extent(fstrptr)) && i<BUFSIZ-1) fbuf[i++] = *fstrptr++;
        if(*fstrptr && flen+i<BUFSIZ-1) {
          strncpy(fbuf+i, fstrptr, flen);
          i += flen;
          fstrptr += flen;
        }
        while(*fstrptr && !format_extent(fstrptr) && i<BUFSIZ-1) fbuf[i++] = *fstrptr++;
        fbuf[i] = '\0';
      } else
        strncpy(fbuf, fstrptr, BUFSIZ);

      switch( printval.type() )
      {
      case ComValue::SymbolType:
      case ComValue::StringType:
	out_form(out, fbuf, symbol_pntr( printval.symbol_ref()));
	break;
	
      case ComValue::BooleanType:
	out_form(out, fbuf, printval.boolean_ref());
	break;
	
      case ComValue::CharType:
	out_form(out, fbuf, printval.char_ref());
	break;	    
	
      case ComValue::UCharType:
	out_form(out, fbuf, printval.uchar_ref());
	break;
	
      case ComValue::IntType:
	out_form(out, fbuf, printval.int_ref());
	break;
	
      case ComValue::UIntType:
	out_form(out, fbuf, printval.uint_ref());
	break;
	
      case ComValue::LongType:
	out_form(out, fbuf, printval.long_ref());
	break;
	
      case ComValue::ULongType:
	out_form(out, fbuf, printval.ulong_ref());
	break;
	
      case ComValue::FloatType:
	out_form(out, fbuf, printval.float_ref());
	break;
	
      case ComValue::DoubleType:
	out_form(out, fbuf, printval.double_ref());
	break;
	
      case ComValue::ArrayType: 
      {
	
        ALIterator i;
        AttributeValueList* avl = printval.array_val();
        avl->First(i);
        boolean first = true;
        while (!avl->Done(i)) {
          ComValue val(*avl->GetAttrVal(i));
          push_stack(formatstr);
          push_stack(val);
          exec(2,0);
          avl->Next(i);
          if (!avl->Done(i)) out << "\n";
        }
      }
      break;
      
      case ComValue::BlankType:
	out << "<blank>";
	break;
	
      case ComValue::UnknownType:
	out_form(out, fbuf, nil);
	break;
	
      default:
	break;
      }
    }
  }


  reset_stack();
  if (stringflag.is_true() || strflag.is_true()) {
    out << '\0';
    ComValue retval(((std::strstreambuf*)strmbuf)->str());
    push_stack(retval);
  } else if (symbolflag.is_true() || symflag.is_true()) {
    out << '\0';
    int symbol_id = symbol_add(((std::strstreambuf*)strmbuf)->str());
    ComValue retval(symbol_id, ComValue::SymbolType);
    push_stack(retval);
  } else
    push_stack(ComValue::blankval());

  delete strmbuf;

}

int PrintFunc::format_extent(const char* fstr) {

  /* %[flags][width][.precision][length]specifier */
  int len=0;

  if (*fstr!='%')
    return 0;
  else
    len++;

  /* flags: minus, plus, space, pound, zero */
  while(fstr[len] == '-' || fstr[len] == '+' || fstr[len] == ' ' || fstr[len] == '#' || fstr[len] == '0')
    len++;

  /* width: 0-9 or star */
  if(fstr[len]=='*') 
    len++;
  else 
    while(fstr[len] >= '0' && fstr[len] <= '9')
      len++;

  /* .precision */
  if(fstr[len]=='.') {
    if(fstr[len]=='*') 
      len+=2;
    else {
      len++;
      if(fstr[len] < '0' || fstr[len] > '9')
        return 0;
      while(fstr[len] >= '0' && fstr[len] <= '9')
        len++;
    }
  }
  
  /* length: h, l, or L */
  while(fstr[len] == 'h' || fstr[len] == 'l' || fstr[len] == 'L')
    len++;

  /* specifier */
  if(fstr[len] == 'c' || fstr[len] == 'd' || fstr[len] == 'i' || fstr[len] == 'e' || fstr[len] == 'E' ||
     fstr[len] == 'f' || fstr[len] == 'g' || fstr[len] == 'G' || fstr[len] == 'o' || fstr[len] == 's' ||
     fstr[len] == 'u' || fstr[len] == 'x' || fstr[len] == 'X' || fstr[len] == 'p' || fstr[len] == 'n' )
    return len+1;
  else
    return 0;
  
}

/*****************************************************************************/

OpenFileFunc::OpenFileFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void OpenFileFunc::execute() {
  ComValue filenamev(stack_arg(0));
  ComValue modev(stack_arg(1));
  static int pipe_symid = symbol_add("pipe");
  ComValue pipeflagv(stack_key(pipe_symid));
  static int in_symid = symbol_add("in");
  ComValue inflagv(stack_key(in_symid));
  static int out_symid = symbol_add("out");
  ComValue outflagv(stack_key(out_symid));
  static int err_symid = symbol_add("err");
  ComValue errflagv(stack_key(err_symid));
  reset_stack();
  
  if (inflagv.is_true()) {
      FileObj* fileobj = new FileObj(stdin);
      ComValue retval(FileObj::class_symid(), (void*)fileobj);
      push_stack(retval);
      return;
  }

  if (outflagv.is_true()) {
      FileObj* fileobj = new FileObj(stdout);
      ComValue retval(FileObj::class_symid(), (void*)fileobj);
      push_stack(retval);
      return;
  }

  if (errflagv.is_true()) {
      FileObj* fileobj = new FileObj(stderr);
      ComValue retval(FileObj::class_symid(), (void*)fileobj);
      push_stack(retval);
      return;
  }

  if (pipeflagv.is_true() && modev.is_string() && 
      (strcmp(modev.string_ptr(),"rw")==0 || strcmp(modev.string_ptr(),"wr")==0)) {
    PipeObj* pipeobj = new PipeObj(filenamev.string_ptr());
    ComValue retval(PipeObj::class_symid(), (void*)pipeobj);
    push_stack(retval);
    if (Component::use_unidraw()) {
      ComterpHandler* pipe_handler = new ComterpHandler(comterpserv());
      if (ComterpHandler::reactor_singleton()->register_handler(pipeobj->rdfd(), pipe_handler, 
                                                                ACE_Event_Handler::READ_MASK)==-1) {
        fprintf(stderr, "Trouble opening handler for pipeobj\n");
      exit(1);
      }
      pipe_handler->log_only(1);
    }
  } else {
    FileObj* fileobj = new FileObj(filenamev.string_ptr(), modev.is_string() ? modev.string_ptr() : "r", pipeflagv.is_true());
    if (fileobj->fptr())  {
      ComValue retval(FileObj::class_symid(), (void*)fileobj);
      push_stack(retval);
    } else {
      delete fileobj;
      push_stack(ComValue::nullval());
    }
  }
}

/*****************************************************************************/

CloseFileFunc::CloseFileFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void CloseFileFunc::execute() {
  ComValue fileobjv(stack_arg(0));
  reset_stack();
  FileObj *fileobj = (FileObj*)fileobjv.geta(FileObj::class_symid());
  if (fileobj && fileobj->fptr())
    fclose(fileobj->fptr());
  else {
    PipeObj *pipeobj = (PipeObj*)fileobjv.geta(PipeObj::class_symid());
    pipeobj->close();
  }
}

/*****************************************************************************/

GetStringFunc::GetStringFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GetStringFunc::execute() {
  ComValue fileobjv(stack_arg(0));
  reset_stack();
  FileObj *fileobj = (FileObj*)fileobjv.geta(FileObj::class_symid());
  FILE* fptr = NULL;
  if (fileobj && fileobj->fptr()) {
    fptr = fileobj->fptr();
  } else {
    PipeObj *pipeobj = (PipeObj*)fileobjv.geta(PipeObj::class_symid());
    if (pipeobj && pipeobj->rdfptr()) 
      fptr = pipeobj->rdfptr();
  }
  char buffer[BUFSIZ];
  char* ptr = fgets(buffer, BUFSIZ, fptr);
  if (ptr)  {
    ComValue retval(buffer);
    push_stack(retval);
  } else
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

GetArgFunc::GetArgFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GetArgFunc::execute() {
  ComValue numv(stack_arg(0));
  reset_stack();
  int arg_sym = comterp()->arg_str(numv.int_val());
  if (arg_sym>0) {
    ComValue retval(comterp()->arg_str(numv.int_val()), ComValue::StringType);
    push_stack(retval);
  } else
    push_stack(ComValue::nullval());
  return;
}

/*****************************************************************************/

NumArgFunc::NumArgFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NumArgFunc::execute() {
  reset_stack();
  ComValue retval(comterp()->narg_str(), ComValue::IntType);
  push_stack(retval);
  return;
}

