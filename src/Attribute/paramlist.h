/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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
 * ParamList - a list of parameter definition structures.
 */

#if !defined(param_list_h)
#define param_list_h

#include <IV-2_6/InterViews/defs.h>
#include <Unidraw/enter-scope.h>
#include <IV-2_6/_enter.h>

class ALIterator;
class LexScan;
class AList;
class istream;
class ostream;

//: signature of static methods for reading parameter data from an istream.
typedef int (*param_callback)(istream&, void*, void*, void*, void*);

class ParamList;

//: parameter definition structure for ParamList.
// structure for defining an argument to be read out of a serialized view
// of an object written out for persistent storage or interprocess communication.
class ParamStruct {
public:
    enum ParamFormat { required, optional, keyword, other };
    // type of parameter formats supported.  Similar to Common Lisp.

    ParamStruct(const char* name, ParamFormat format, param_callback ifunc,
		int offset1, int offset2, int offset3, int offset4,
		int indirection = -1);
    // construct with 'name' and 'format', a pointer to a static method
    // with 'param_callback' signature, and up to 4 offsets relative to a 
    // base pointer that will be used in constructing the contents of another
    // object during a de-serialization process (an istream constructor).
    // Also possible to specify an 'indirection' amount used when adding
    // offsets to the base.
    ParamStruct(ParamStruct&);
    // copy constructor.
    ~ParamStruct();

    const char* name()          {return _name;}
    // the name of the parameter -- keyword parameters do not need the ":".
    ParamFormat format()        {return _format;}
    // parameter format type: required, optional, keyword, or other.
    param_callback ifunc()      {return _ifunc;}
    // pointer to static method (or function) to invoke when parameter found
    // during de-serialization process.

    int offset1()               {return _offset1;}
    // first offset from base.  This one is always supplied, and might
    // be the only true offset if pointing to an array.
    int offset2()               {return _offset2;}
    // second offset from base.  Might really be an array length in a
    // certain usage.  The 'ifunc' is free to interpret these as it wants.
    int offset3()               {return _offset3;}
    // third offset from base.
    int offset4()               {return _offset4;}
    // fourth offset from base.  If this is not enough either an array can
    // can be passed by using the first two, or these can be ignored
    // all together and you can use fixed locations within the object being 
    // deserialized.
    int indirection()           {return _indirection;}
    // offset from base address used to retrieve another address (by indirection). 
    // disabled if less than zero.

    void* addr1(void* base);
    // compute address by adding offset1() to 'base' or an indirect base
    // determined by the contents of 'base' + indirection().
    void* addr2(void* base);
    // compute address by adding offset2() to 'base' or an indirect base
    // determined by the contents of 'base' + indirection().
    void* addr3(void* base);
    // compute address by adding offset3() to 'base' or an indirect base
    // determined by the contents of 'base' + indirection().
    void* addr4(void* base);
    // compute address by adding offset4() to 'base' or an indirect base
    // determined by the contents of 'base' + indirection().

protected:
    char* _name;
    ParamFormat _format;
    param_callback _ifunc;
    int _offset1;
    int _offset2;
    int _offset3;
    int _offset4;
    int _indirection;
};

//: list of ParamStruct objects for de-serialization of a previously serialized object.
// list of ParamStruct objects that defines the possible signature
// of a serialized object, for use in de-serialization (istream constructors). 
// <p>
// See OverlayScript and OverlayComp for an example of how this is used.  
// OverlayScript is a serializing view of an OverlayComp, and generates a variable
// length, ASCII record (with arguments enclosed in parenthesis following the object
// name) that describes the internal state of an OverlayComp.  Then later, the 
// istream constructor of an OverlayComp uses the static ParamList object of its 
// class to reverse the process (called de-serialization), to set the internal state 
// of a newly constructed OverlayComp to mirror the internal state of the original 
// object.  This mechanism can be used for any struct or C++ class that needs to be 
// saved out to disk for persistent storage, or transmitted over a socket. 
// <p>
// Keep in mind that only relative addresses are used to construct the ParamStruct
// objects that make up the ParamList, and the actual base address used to
// resolve where to place data is different when the final object is constructed
// (except for the very first time an object of a given class gets constructed --
// at that time the addresses used to build up the ParamList are the same ones
// used to populate the object).
class ParamList {
public:
    ParamList(ParamList* = nil);
    // construct with optional ParamList to copy.
    virtual ~ParamList();

    static LexScan* lexscan();
    // construct default LexScan for use.

    void add_param(
	const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
	void* base, void* addr1, 
	void* addr2 = nil, void* addr3 = nil, void* addr4 = nil);
    // compose and insert ParamStruct, computing offsets 1 thru 4 by differing
    // against 'base'.
    void add_param_indirect(
	const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
	void* base, void* offset, void* addr1, 
	void* addr2 = nil, void* addr3 = nil, void* addr4 = nil);
    // compose and insert ParamStruct, computing offsets 1 thru 4 by differing
    // against the content of 'offset', and subtracting 'base' from 'offset'
    // to arrive at indirection offset to set in the ParamStruct (the offset from
    // the base address of the object being constructed where you will find
    // an address to add the other offsets to, to arrive a final address).
    void add_param_first(
	const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
	void* base = (void*)0x1, void* addr1 = (void*)0x1, 
	void* addr2 = nil, void* addr3 = nil, void* addr4 = nil);
    // compose and insert ParamStruct as far forward in the ParamList as possible
    // given its desired format type.  In this manner a parameter handler can be 
    // easily overriden.

    boolean read_args(istream& in, void* base);
    // work-horse method that assumes istream is positioned right before
    // the open-paren of the serialized object (the object name precedes
    // the open-paren).  Handles the logic to parse first required, then
    // optional, then keyword arguments (or 'other' arugments, which means
    // pass the rest of the arguments on as is).  Invokes all the function
    // pointers of type param_callback.

    /* static callback functions */
    static int read_int(istream&, void*, void*, void*, void*);
    // param_callback to read up to four comma-separated integers from istream.
    static int read_float(istream&, void*, void*, void*, void*);
    // param_callback to read up to four comma-separated floats from istream.
    static int read_double(istream&, void*, void*, void*, void*);
    // param_callback to read up to four comma-separated doubles from istream.
    static int read_string(istream&, void*, void*, void*, void*);
    // param_callback to read up to four comma-separated character strings from istream.
    static int read_ints(istream&, void*, void*, void*, void*);
    // param_callback to read any number of comma-separated integers from istream.
    static int read_floats(istream&, void*, void*, void*, void*);
    // param_callback to read any number of comma-separated floats from istream.
    static int read_doubles(istream&, void*, void*, void*, void*);
    // param_callback to read any number of comma-separated doubles from istream.
    static int read_strings(istream&, void*, void*, void*, void*);
    // param_callback to read any number of comma-separated strings from istream.

    /* static functions for use in callbacks */
    static int skip_space(istream& in);
    // skip whitespace in istream, for use of a param_callback.
    static int parse_token(istream& in, char* buffer, int buflen, char delim = '(');
    // parse token from istream, for use of a param_callback. 
    static int parse_token(istream& in, char* buffer, int buflen, char* delim);
    // parse token from istream ended by one of a string of delimters, 
    // for use of a param_callback.
    static int parse_string(istream& in, char* buffer, int buflen);
    // parse string from istream, correctly handling embedded quotes prefixed
    // by back-slashes, for use of a param_callback.
    static int parse_points(istream&, Coord*&, Coord*&, int&);
    // parse parenthesized point list of integers from istream, for use of a param_callback.
    static int parse_fltpts(istream&, float*&, float*&, int&);
    // parse parenthesized point list of floats from istream, for use of a param_callback.
    static int parse_dblpts(istream&, double*&, double*&, int&);
    // parse parenthesized point list of doubles from istream, for use of a param_callback.
    static int parse_text(istream& in, char* buffer, int buflen); 
    // parse text string, converting embedded octal constants, and handling 
    // embedded quotes preceded by back-slashes, for use of a param_callback.
    static char* parse_textbuf(istream& in);
    // parse multi-line comma-separated set of character strings into a single buffer,
    // for use of a param_callback.
    static int output_text(ostream& out, const char* text, int indent=0);
    // output text in a format readable by parse_text().
    static int parse_pathname(istream& in, char* buffer, int buflen, const char* dir);
    // parse pathname, expanding relative pathnames from 'dir', for use of a
    // param_callback.

    static boolean url_use_ok();
    // test if url use ok for pathnames.  Same as OpenFileChooser method.
    static boolean urltest(const char*);
    // test if pathname looks like a URL.  Same as OpenFileChooser method.

    static int bintest(const char* name);
    // return 0 if executable can be found, otherwise -1.
    static boolean bincheck(const char* name);
    // return true if executable can be found.

    static const char* filter(const char* string, int len);
    // filter text buffer for octal constants.
    static char* octal(unsigned char c, register char* p);
    // convert a character to an octal string.
    static char octal(const char* p);
    // convert string of three octal digits to a character.

    static ParamStruct* CurrParamStruct() { return _currstruct; }
    // last ParamStruct from ::GetStruct

protected:
    void insert(ParamStruct*);
    void insert_first(ParamStruct*);

    void First(ALIterator&);
    void Last(ALIterator&);
    void Next(ALIterator&);
    void Prev(ALIterator&);
    boolean Done(ALIterator);
    boolean IsEmpty();
    int Number();

    void Append(ParamStruct*);
    void Prepend(ParamStruct*);
    void InsertAfter(ALIterator, ParamStruct*);
    void InsertBefore(ALIterator, ParamStruct*);
    void Remove(ParamStruct*);
    void Remove(ALIterator&);

    ParamStruct* GetStruct(ALIterator);
    void SetStruct(ParamStruct*, ALIterator&);
    boolean Includes(ParamStruct*);

    ParamStruct* Struct(AList*);
    AList* Elem(ALIterator);

protected:
    AList* _alist;
    int _count;
    int _required_count;
    int _optional_count;
    int _keyword_count;
    int _other_count;

    static LexScan* _lexscan;
    static ParamStruct* _currstruct;
};

#include <IV-2_6/_leave.h>

#endif /* !defined(_param_list_h) */
