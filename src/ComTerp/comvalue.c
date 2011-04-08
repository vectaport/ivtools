/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994-1998 Vectaport Inc.
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

#include <ComTerp/comfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Attribute/aliterator.h>
#include <Attribute/paramlist.h>

#include <iostream.h>
#include <memory.h>

#include <string.h>

ComValue ComValue::_nullval;
ComValue ComValue::_trueval(1, ComValue::BooleanType);
ComValue ComValue::_falseval(0, ComValue::BooleanType);
ComValue ComValue::_blankval(ComValue::BlankType);
ComValue ComValue::_unkval(ComValue::UnknownType);
ComValue ComValue::_oneval(1, ComValue::IntType);
ComValue ComValue::_zeroval(0, ComValue::IntType);

/*****************************************************************************/

const ComTerp* ComValue::_comterp = nil;

ComValue::ComValue(ComValue& sv) {
    *this = sv;
    if (_type == AttributeValue::ArrayType)
      Resource::ref(_v.arrayval.ptr);
}

ComValue::ComValue(AttributeValue& sv) {
    *(AttributeValue*)this = sv;
    if (_type == AttributeValue::ArrayType)
      Resource::ref(_v.arrayval.ptr);
    zero_vals();
}

ComValue::ComValue() {
    type(UnknownType);
    _aggregate_type = UnknownType;
    zero_vals();
}

ComValue::ComValue(ValueType valtype) {
    type(valtype);
    _aggregate_type = UnknownType;
    zero_vals();
}

ComValue::ComValue(unsigned char v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(char v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(unsigned short v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(short v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(unsigned int v, ValueType type) : AttributeValue(v, type) {zero_vals();}
ComValue::ComValue(unsigned int kv, unsigned int kn, ValueType type) : AttributeValue(kv, kn, type) {zero_vals();}
ComValue::ComValue(int v, ValueType type) : AttributeValue(v, type) {zero_vals();}
ComValue::ComValue(unsigned long v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(long v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(float v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(double v) : AttributeValue(v) {zero_vals();}
ComValue::ComValue(int classid, void* ptr) : AttributeValue(classid, ptr) {zero_vals();}
ComValue::ComValue(AttributeValueList* avl) : AttributeValue(avl) {zero_vals();}
ComValue::ComValue(const char* string) : AttributeValue(string) {zero_vals();}
ComValue::ComValue(ComFunc* func) : AttributeValue(ComFunc::class_symid(), func) {zero_vals(); type(ComValue::CommandType); command_symid(func->funcid()); }

ComValue::~ComValue() {
}

ComValue::ComValue(postfix_token* token) {
    void* v1 = &_v;
    void* v2 = &token->v;
    memcpy(v1, v2, sizeof(double));
    switch (token->type) {
    case TOK_STRING:  type(StringType); break;
    case TOK_CHAR:    type(CharType); break;
    case TOK_DFINT:   type(IntType); break;
    case TOK_DFUNS:   type(UIntType); break;
    case TOK_LNINT:   type(LongType); break;
    case TOK_LNUNS:   type(ULongType); break;
    case TOK_FLOAT:   type(FloatType); break;
    case TOK_DOUBLE:  type(DoubleType); break;
    case TOK_EOF:     type(EofType); break;
    case TOK_COMMAND: type(SymbolType); break;
    case TOK_KEYWORD: type(KeywordType); break;
    case TOK_BLANK:   type(BlankType); break;
    default:          type(UnknownType); break;
    }
    _narg = token->narg;
    _nkey = token->nkey;
    _nids = token->nids;
    _aggregate_type = UnknownType;
    _pedepth = 0;
}

ComValue& ComValue::operator= (const ComValue& sv) {
    assignval(sv);
    _narg = sv._narg;
    _nkey = sv._nkey;
    _nids = sv._nids;
    _pedepth = sv._pedepth;
    return *this;
}
    
int ComValue::narg() const { return _narg; }
int ComValue::nkey() const { return _nkey; }
int ComValue::nids() const { return _nids; }

ostream& operator<< (ostream& out, const ComValue& sv) {
    ComValue* svp = (ComValue*)&sv;
    char* title;
    char* symbol;
    int counter;
    boolean brief = sv.comterp() ? sv.comterp()->brief() : false;
    switch( svp->type() )
	{
	case ComValue::KeywordType:
	  if (brief) 
	    out << ":" << symbol_pntr( svp->symbol_ref() ); 
	  else
	    out << "Keyword( " << symbol_pntr( svp->keyid_val() ) << 
	      ")  narg: " << svp->keynarg_val(); 
	  break;
	    
	case ComValue::SymbolType:
	  if (brief) 
	    out << symbol_pntr( svp->symbol_ref());
	  else {
	    title = "symbol( ";
	    symbol = symbol_pntr( svp->symbol_ref() );
	    out << title << symbol;
	    counter = strlen(title) + strlen(symbol);
	    while( ++counter < 32 ) out << ' ';
	    out << ")   narg " << svp->narg() << "  nkey " << svp->nkey() << 
	      "  nids: " << svp->nids();
	  }
	  break;
	    
	case ComValue::StringType:
	  if (brief)
	    ParamList::output_text(out, svp->string_ptr());
	  else {
	    out << "string(";
	    ParamList::output_text(out, svp->string_ptr());
	    out << ")";
	  }
	  break;
	    
	case ComValue::BooleanType:
	  if (brief)
	    out << svp->boolean_ref();
	  else
	    out << "boolean( " << svp->boolean_ref() << " )";
	  break;
	    
	case ComValue::CharType:
	  if (brief)
	    out << "'" << svp->char_ref() << "'";
	  else
	    out << "char( " << svp->char_ref() << ":" << (int)svp->char_ref() << " )";
	  break;	    

	case ComValue::UCharType:
	  if (brief)
	    out << svp->uchar_ref();
	  else
	    out << "uchar( " << svp->uchar_ref() << ":" << (int)svp->uchar_ref() << " )";
	  break;
	    
	case ComValue::ShortType:
	  if (brief)
	    out << svp->short_ref();
	  else
	    out << "short( " << svp->short_ref() << ":" << (int)svp->short_ref() << " )";
	  break;	    

	case ComValue::UShortType:
	  if (brief)
	    out << svp->ushort_ref();
	  else
	    out << "ushort( " << svp->ushort_ref() << ":" << (int)svp->ushort_ref() << " )";
	  break;
	    
	case ComValue::IntType:
	  if (brief)
	    out << svp->int_ref();
	  else
	    out << "int( " << svp->int_ref() << " )";
	  break;
	    
	case ComValue::UIntType:
	  if (brief)
	    out << svp->uint_ref();
	  else
	    out << "uint( " << svp->uint_ref() << " )";
	  break;
	    
	case ComValue::LongType:
	  if (brief)
	    out << svp->long_ref() << "L";
	  else
	    out << "long( " << svp->long_ref() << " )";
	  break;
	    
	case ComValue::ULongType:
	  if (brief)
	    out << svp->ulong_ref() << "L";
	  else
	    out << "ulong( " << svp->ulong_ref() << " )";
	  break;
	    
	case ComValue::FloatType:
	  if (brief)
	    out << svp->float_ref();
	  else
	    out << "float( " << svp->float_ref() << " )";
	  break;
	    
	case ComValue::DoubleType:
	  if (brief)
	    out << svp->double_ref();
	  else
	    out << "double( " << svp->double_ref() << " )";
	  break;
	    
	case ComValue::EofType:
	    out << "eof";
	    break;
	    
	case ComValue::ArrayType:
	  if (brief) {
	    ALIterator i;
	    AttributeValueList* avl = svp->array_val();
	    avl->First(i);
	    boolean first = true;
	    while (!avl->Done(i)) {
	      ComValue val(*avl->GetAttrVal(i));
	      out << val;
	      avl->Next(i);
	      if (!avl->Done(i)) out << "\n";
	    }
	  } else {
	    out << "array of length " << svp->array_len();
	    ALIterator i;
	    AttributeValueList* avl = svp->array_val();
	    avl->First(i);
	    boolean first = true;
	    while (!avl->Done(i)) {
 	        out << "\n\t" << *avl->GetAttrVal(i);
	        avl->Next(i);
	    }
	  }
	  break;
	    
	case ComValue::CommandType:
	  if (brief) 
	    out << symbol_pntr( svp->command_symid());
	  else {
	    title = "command( ";
	    symbol = symbol_pntr( svp->command_symid());
	    out << title << symbol << ")";
	  }
	  break;
	    
	case ComValue::BlankType:
	  break;

	case ComValue::ObjectType:
	  if (svp->class_symid() == Attribute::class_symid())
	    out << *((Attribute*)svp->obj_val())->Value();
	  else
	    out << "<" << symbol_pntr(svp->class_symid()) << ">";
	  break;

	case ComValue::UnknownType:
	  out << "nil";
	  break;
	    
	default:
	  break;
	}
    return out;
}

ComValue& ComValue::nullval() { 
  *&_nullval = ComValue();
  return _nullval; 
}

ComValue& ComValue::trueval() { 
  *&_trueval = ComValue(1, ComValue::BooleanType);
  return _trueval; 
}

ComValue& ComValue::falseval() { 
  *&_falseval = ComValue(0, ComValue::BooleanType);
  return _falseval; 
}

ComValue& ComValue::blankval() { 
  *&_blankval = ComValue(ComValue::BlankType);
  return _blankval;
}

ComValue& ComValue::unkval() { 
  *&_unkval = ComValue(ComValue::UnknownType);
  return _unkval;
}

ComValue& ComValue::oneval() { 
  *&_oneval = ComValue(1, ComValue::IntType);
  return _oneval;
}

ComValue& ComValue::zeroval() { 
  *&_zeroval = ComValue(0, ComValue::IntType);
  return _zeroval;
}

boolean ComValue::is_comfunc(int func_classid) {
  return is_type(CommandType) && 
    func_classid==((ComFunc*)obj_val())->classid(); 
}
