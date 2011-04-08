/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

typedef int (*param_callback)(istream&, void*, void*, void*, void*);

class ParamList;

class ParamStruct {
public:
    enum ParamFormat { required, optional, keyword, other };

    ParamStruct(const char* name, ParamFormat format, param_callback ifunc,
		int offset1, int offset2, int offset3, int offset4,
		int indirection = -1);
    ~ParamStruct();

    const char* name()          {return _name;}
    ParamFormat format()        {return _format;}
    param_callback ifunc()      {return _ifunc;}

    int offset1()               {return _offset1;}
    int offset2()               {return _offset2;}
    int offset3()               {return _offset3;}
    int offset4()               {return _offset4;}
    int indirection()           {return _indirection;}

    void* addr1(void*);
    void* addr2(void*);
    void* addr3(void*);
    void* addr4(void*);

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
    
class ParamList {
public:
    ParamList(ParamList* = nil);
    virtual ~ParamList();

    static LexScan* lexscan();

    void add_param(
	const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
	void* base, void* addr1, 
	void* addr2 = nil, void* addr3 = nil, void* addr4 = nil);
    void add_param_indirect(
	const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
	void* base, void* offset, void* addr1, 
	void* addr2 = nil, void* addr3 = nil, void* addr4 = nil);
    boolean read_args(istream& in, void* base);
    

    /* static callback functions */
    static int read_int(istream&, void*, void*, void*, void*);
    static int read_float(istream&, void*, void*, void*, void*);
    static int read_double(istream&, void*, void*, void*, void*);
    static int read_string(istream&, void*, void*, void*, void*);
    static int read_ints(istream&, void*, void*, void*, void*);
    static int read_floats(istream&, void*, void*, void*, void*);
    static int read_doubles(istream&, void*, void*, void*, void*);
    static int read_strings(istream&, void*, void*, void*, void*);

    /* static functions for use in callbacks */
    static int skip_space(istream& in);
    static int parse_token(istream& in, char* buffer, int buflen, char delim = '(');
    static int parse_token(istream& in, char* buffer, int buflen, char* delim);
    static int parse_string(istream& in, char* buffer, int buflen);
    static int parse_points(istream&, Coord*&, Coord*&, int&);
    static int parse_fltpts(istream&, float*&, float*&, int&);
    static int parse_dblpts(istream&, double*&, double*&, int&);
    static int parse_text(istream& in, char* buffer, int buflen); 
    static char* parse_textbuf(istream& in);
    static int output_text(ostream& out, const char* text, int indent=0);
    static int parse_pathname(istream& in, char* buffer, int buflen, const char* dir);

    static const char* filter(const char* string, int len);
    static char* octal(unsigned char c, register char* p);
    static char octal(const char* p);

protected:
    void insert(ParamStruct*);

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
};

#include <IV-2_6/_leave.h>

#endif /* !defined(_param_list_h) */
