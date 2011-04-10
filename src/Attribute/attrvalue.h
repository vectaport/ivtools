/*
 * Copyright (c) 2001,2006 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
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

#if !defined(_attrvalue_h)
#define _attrvalue_h

#include <stdlib.h>
#include <OS/enter-scope.h>
#include <Attribute/classid.h>

extern "C" {
    int symbol_add(char*);
    int symbol_del(int);
    int symbol_find(char*);
    char* symbol_pntr(int);
}

class AttributeValueList;

#include <iosfwd>

//: struct for symbol value, symid + global flag for symbol value
// used in attr_value.
typedef struct {
       unsigned int symid;
       boolean globalflag;
} symval_struct;

//: void* pointer plus object classid (see macro in OverlayUnidraw/ovcomps.h)
// used in attr_value.
typedef struct {
       void *ptr;
       unsigned int type;
} objval_struct;

//: pointer to list of values, plus optional type id.
// used in attr_value.
typedef struct {
  AttributeValueList *ptr;
  unsigned int type;
} arrayval_struct;

//: void* pointer to ComFunc object plus optional type id
// used in attr_value.
typedef struct {
       void *funcptr;
       AttributeValueList *listptr;
} streamval_struct;

//: keyword symbol id, plus number of arguments that follow.
// used in attr_value.
typedef struct {
  unsigned int keyid;
  unsigned int keynarg;
} keyval_struct;

//: union for AttributeValue typed data storage.
typedef union attr_value_union
{
      char              charval;
      unsigned char     ucharval;
      short             shortval;
      unsigned short    ushortval;
      int               dfintval;
      unsigned int      dfunsval;
      long              lnintval;
      unsigned long     lnunsval;
      float             floatval;
      double            doublval;
      symval_struct     symval;
      objval_struct     objval;
      arrayval_struct   arrayval;
      streamval_struct  streamval;
      keyval_struct     keyval;
} attr_value;

typedef const char* const_char_ptr;

//: multi-type attribute value object.
class AttributeValue {
public:
    enum ValueType { UnknownType, CharType, UCharType, ShortType, UShortType, 
		     IntType, UIntType, LongType, ULongType, FloatType, DoubleType, 
                     StringType, SymbolType, ArrayType, StreamType, CommandType, KeywordType, 
                     ObjectType, EofType, BooleanType, OperatorType, BlankType,
		     ListType = ArrayType
};
    // enum for attribute value types.

    enum ValueState { UnknownState, OctState, HexState };
    // enum for states

    AttributeValue(ValueType type);
    // construct with specified type and unitialized value.
    AttributeValue(ValueType type, attr_value value);
    // construct with specified type and value struct.
    AttributeValue(AttributeValue&);
    // copy constructor.
    AttributeValue(AttributeValue*);
    // deep copy constructor.
    AttributeValue();
    // default constructor (UnknownType constructor).

    AttributeValue(char val);
    // CharType constructor.
    AttributeValue(unsigned char val);
    // UCharType constructor.
    AttributeValue(short val);
    // ShortType constructor.
    AttributeValue(unsigned short val);
    // UShortType constructor.
    AttributeValue(int val, ValueType type);
    // IntType constructor or any other int-like value.
    AttributeValue(unsigned int val, ValueType type);
    // UIntType constructor or any other unsigned-int-like value including SymbolType.
    AttributeValue(unsigned int keysym, unsigned int narg, ValueType=KeywordType);
    // KeywordType constructor (or can be used for ObjectType).
    AttributeValue(long val);
    // LongType constructor.
    AttributeValue(unsigned long val);
    // ULongType constructor.
    AttributeValue(float val);
    // FloatType constructor.
    AttributeValue(double);
    // DoubleType constructor.
    AttributeValue(int class_symid, void* objptr);
    // ObjectType constructor.
    AttributeValue(AttributeValueList* listptr);
    // ArrayType/ListType constructor.
    AttributeValue(void* comfunc, AttributeValueList* vallist);
    // StreamType constructor.
    AttributeValue(const char* val);
    // StringType constructor.

    virtual ~AttributeValue();
    // set to UnknownType and unref pointer if ArrayType/ListType or StreamType.

    void clear(); 
    // clear bytes of multi-value union

    AttributeValue& operator= (const AttributeValue&);
    // copy assignment operator.

    ValueType type() const;
    // return type enum.
    void type(ValueType);
    // set type enum.
    int type_size() { return type_size(type()); }
    // return sizeof of value of this type.
    static int type_size(ValueType);
    // return sizeof of value of given type.
    int type_symid() const;
    // return symbol id corresponding to type

    void assignval (const AttributeValue&);
    // copy contents of AttributeValue

    char& char_ref();                 // char by reference.
    unsigned char& uchar_ref();       // unsigned char by reference.
    short& short_ref();               // short by reference.
    unsigned short& ushort_ref();     // unsigned short by reference.
    int& int_ref();                   // int by reference.
    unsigned int& uint_ref();         // unsigned int by reference.
    boolean& boolean_ref();           // boolean by reference.
    long& long_ref();                 // long by reference.
    unsigned long& ulong_ref();       // unsigned long by reference.
    float& float_ref();               // float by reference.
    double& double_ref();             // double by reference.
    unsigned int& string_ref();       // string symbol id by reference.
    unsigned int& symbol_ref();       // symbol id by reference.
    void*& obj_ref();                 // void* pointer to object by reference.
    unsigned int& obj_type_ref();     // classid of object by reference.
    AttributeValueList*& array_ref(); // values in list by reference.
    unsigned int& array_type_ref();   // type of values in list by reference
    AttributeValueList*& list_ref();  // values in list by reference.
    unsigned int& list_type_ref();    // type of values in list by reference
    unsigned int& keyid_ref();        // symbol id of keyword by reference.
    unsigned int& keynarg_ref();      // number of arguments after keyword by reference.

    char char_val();                  // char by value.                             
    unsigned char uchar_val();	      // unsigned char by value.                    
    short short_val();		      // short by value.                            
    unsigned short ushort_val();      // unsigned short by value.                   
    int int_val();		      // int by value.                              
    unsigned int uint_val();	      // unsigned int by value.                     
    boolean boolean_val();	      // boolean by value.                          
    long long_val();		      // long by value.                             
    unsigned long ulong_val();	      // unsigned long by value.                    
    float float_val();		      // float by value.                            
    double double_val();	      // double by value.                           
    unsigned int string_val();	      // string symbol id by value.                 
    unsigned int symbol_val();	      // symbol id by value.                        
    void* obj_val();		      // void* pointer to object by value.          
    unsigned int obj_type_val();      // classid of object by value.                
    unsigned int& class_symid();       // classid of object by value.                
    AttributeValueList* array_val();  // values in list by value.                   
    unsigned int array_type_val();    // type of values in list by value            
    AttributeValueList* list_val();   // values in list by value.                   
    unsigned int list_type_val();     // type of values in list by value            
    unsigned int keyid_val();	      // symbol id of keyword by value.             
    unsigned int keynarg_val();	      // number of arguments after keyword by value.

    const char* string_ptr();
    // lookup and return pointer to string associated with string.
    const char* symbol_ptr();
    boolean global_flag();
    // return true if a symbol and the global flag is set.
    void global_flag(boolean flag);
    // set global flag of a symbol
    int array_len();
    // length of list of values when ArrayType/ListType.
    int list_len();
    // length of list of values when ArrayType/ListType.

    int command_symid();
    // symbol id of associated command name, for use with ComTerp.
    void command_symid(int, boolean alias=false);
    // set symbol id of associated command name, for use with ComTerp.
    boolean command_alias();
    // returns true if command is an alias, not the first name.

    boolean object_compview() { return is_object() && _object_compview; }
    // true if object is wrapped with a ComponentView
    void object_compview(boolean flag) { _object_compview = flag; }
    // true if object is wrapped with a ComponentView

    int stream_mode();
    // 0 = disabled, negative = internal, positive = external
    void stream_mode(int mode) { if (is_stream()) _stream_mode = mode; }
    // 0 = disabled, negative = internal, positive = external
    void* stream_func() { return is_stream() ? _v.streamval.funcptr : nil; }
    // return function pointer associated with stream object
    void stream_func(void* func) { if (is_stream()) _v.streamval.funcptr = func; }
    // set function pointer associated with stream object
    AttributeValueList* stream_list() { return is_stream() ? _v.streamval.listptr : nil; }
    // return pointer to AttributeValueList associated with stream object
    void stream_list(AttributeValueList* list); 
    // set pointer to AttributeValueList associated with stream object

    int state();
    // get generic state value useful for any type other than CommandType, ObjectType, or StreamType
    void state(int val);
    // set generic state value useful for any type other than CommandType, ObjectType, or StreamType

    void negate();
    // negate numeric values.

    boolean is_true() { return type() != UnknownType && boolean_val(); }
    // returns true if !UnknownType && boolean_val()
    boolean is_false() { return !is_true(); }
    // returns true if !is_true()
    boolean is_type(ValueType t) { return type() == t; }
    // returns true if type() == 't'.

    boolean is_char() { return is_char(type()); }
    // returns true if CharType || UCharType.
    boolean is_short() { return is_short(type()); }
    // returns true if ShortType || UShortType.
    boolean is_int() { return is_int(type()); }
    // returns true if IntType || UIntType.
    boolean is_long() { return is_long(type()); }
    // returns true if LongType || ULongType.
    boolean is_float() { return is_float(type()); }
    // returns true if FloatType.
    boolean is_double() { return is_double(type()); }
    // returns true if DoubleType.

    boolean is_integer() { return is_integer(type()); }
    // returns true if is_char() || is_short() || is_int() || is_long().
    boolean is_floatingpoint() { return is_floatingpoint(type()); }
    // returns true if is_float() || is_double().
    boolean is_num() { return is_integer(type()) || is_floatingpoint(type()); }
    // returns true if is_integer() || is_floatingpoint().
    boolean is_numeric() { return is_num(); }
    // same as AttributeValue::is_num().

    boolean is_array() { return is_type(ArrayType); }
    // returns true if ArrayType/ListType.
    boolean is_list() { return is_type(ArrayType); }
    // returns true if ArrayType/ListType.
    boolean is_stream() { return is_type(StreamType); }
    // returns true if StreamType.
    boolean is_key() { return is_type(KeywordType); }
    // returns true if KeywordType.
    boolean is_unknown() { return is_type(UnknownType); }
    // returns true if UnknownType.
    boolean is_null() { return is_unknown(); }
    // returns true if UnknownType.
    boolean is_nil() { return is_unknown(); }
    // returns true if UnknownType.
    boolean is_known() { return !is_type(UnknownType); }
    // returns true if !UnknownType.
    boolean is_string() { return is_type(StringType) || is_type(SymbolType); }
    // returns true if StringType || SymbolType.
    boolean is_symbol() { return is_type(SymbolType); }
    // returns true if SymbolType.
    boolean is_command() { return is_type(CommandType); }
    // returns true if CommandType (for use of ComTerp).
    boolean is_object() { return is_type(ObjectType); }
    // returns true if ObjectType.
    boolean is_object(int class_symid);
    // returns true if ObjectType and class_symid matches or belongs to a parent class.

    static boolean is_char(ValueType t) 
      { return t==CharType || t==UCharType; }
    static boolean is_short(ValueType t)
      { return t==ShortType || t==UShortType; }
    static boolean is_int(ValueType t)
      { return t==IntType || t==UIntType; }
    static boolean is_long(ValueType t) 
      { return t==LongType || t==ULongType; }
    static boolean is_float(ValueType t)
      { return t==FloatType; }
    static boolean is_double(ValueType t)
      { return t==DoubleType; }

    static boolean is_integer(ValueType t)
      { return is_char(t) || is_short(t) || is_int(t) | is_long(t); }
    static boolean is_floatingpoint(ValueType t)
      { return is_float(t) || is_double(t); }
    static boolean is_num(ValueType t)
      { return is_integer(t) || is_floatingpoint(t); }

    boolean is_blank() { return is_type(BlankType); }
    // returns true if BlankType.
    static boolean is_blank(ValueType t)
      { return t==BlankType; };

    boolean is_attributelist();
    // returns true if ObjectType with an AttributeList object.
    boolean is_attribute();
    // returns true if ObjectType with an Attribute object.

    void* geta(int type); 
    // return a pointer if ObjectType matches or is a parent class

    friend ostream& operator << (ostream& s, const AttributeValue&);
    // output AttributeValue to ostream.

    void* value_ptr() { return &_v; }
    // returns void* pointer to value struct.

    void ref_as_needed();
    // increment ref counters as needed
    void unref_as_needed();
    // decrement ref counters as needed
    void dup_as_needed();
    // duplicate lists then increment ref counters as needed
    boolean same_list(const AttributeValue& av);
    // check if arrayval or streamval are the same

protected:

    ValueType _type;
    attr_value _v;
    union { 
      int _command_symid; // used for CommandType.
      boolean _object_compview; // used for ObjectType.
      int _stream_mode; // used for StreamType
      int _state; // useful for any type other than CommandType, ObjectType, or StreamType
    };
    static int* _type_syms;

};

//: for quick casting in debugger
typedef class AttributeValue _AV; 

#endif /* !defined(_attrvalue_h) */
