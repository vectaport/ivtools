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

#include <ComTerp/iofunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>
#include <OS/math.h>
#include <iostream.h>
#include <strstream.h>
#if __GNUC__>=3
#include <fstream.h>
#endif

#define TITLE "IoFunc"

/*****************************************************************************/

PrintFunc::PrintFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PrintFunc::execute() {
  ComValue formatstr(stack_arg(0));
  ComValue printval(stack_arg(1));
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
  reset_stack();

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
    strmbuf = new strstreambuf();
  }
#else
  streambuf* strmbuf = nil;
  if (stringflag.is_false() && strflag.is_false() &&
      symbolflag.is_false() && symflag.is_false()) {
    fileptr_filebuf * fbuf = nil;
    if (comterp()->handler()) {
      fbuf = new fileptr_filebuf(comterp()->handler() && comterp()->handler()->wrfptr() 
			 ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
    } else
      fbuf = new fileptr_filebuf(errflag.is_false() ? stdout : stderr, ios_base::out);
    strmbuf = fbuf;
  } else
    strmbuf = new strstreambuf();
#endif
  ostream out(strmbuf);

  int narg = nargs();
  if (narg==1) {

    if (formatstr.is_string())
      out << formatstr.symbol_ptr();
    else
      out << formatstr;  // which could be arbitrary ComValue

  } else {
    switch( printval.type() )
      {
      case ComValue::SymbolType:
      case ComValue::StringType:
	out_form(out, fstr, symbol_pntr( printval.symbol_ref()));
	break;
	
      case ComValue::BooleanType:
	out_form(out, fstr, printval.boolean_ref());
	break;
	
      case ComValue::CharType:
	out_form(out, fstr, printval.char_ref());
	break;	    
	
      case ComValue::UCharType:
	out_form(out, fstr, printval.uchar_ref());
	break;
	
      case ComValue::IntType:
	out_form(out, fstr, printval.int_ref());
	break;
	
      case ComValue::UIntType:
	out_form(out, fstr, printval.uint_ref());
	break;
	
      case ComValue::LongType:
	out_form(out, fstr, printval.long_ref());
	break;
	
      case ComValue::ULongType:
	out_form(out, fstr, printval.ulong_ref());
	break;
	
      case ComValue::FloatType:
	out_form(out, fstr, printval.float_ref());
	break;
	
      case ComValue::DoubleType:
	out_form(out, fstr, printval.double_ref());
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
	out_form(out, fstr, nil);
	break;
	
      default:
	break;
      }
  }


  if (stringflag.is_true() || strflag.is_true()) {
    out << '\0';
    ComValue retval(((strstreambuf*)strmbuf)->str());
    push_stack(retval);
  } else if (symbolflag.is_true() || symflag.is_true()) {
    out << '\0';
    int symbol_id = symbol_add(((strstreambuf*)strmbuf)->str());
    ComValue retval(symbol_id, ComValue::SymbolType);
    push_stack(retval);
  }
  delete strmbuf;
}

