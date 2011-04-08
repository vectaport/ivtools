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
 * Implementation of ParamList class.
 */

#include <Attribute/alist.h>
#include <Attribute/aliterator.h>
#include <Attribute/paramlist.h>
#include <Attribute/lexscan.h>

#include <IV-2_6/InterViews/textbuffer.h>

#include <IV-2_6/_enter.h>

#include <ctype.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <strstream>
#endif

/*****************************************************************************/

static const int BUFSIZE = 10000;
static char textbuf[BUFSIZE];

static void GetLine (
    const char* s, int size, int begin, int& end, int& lineSize, int& nextBegin
) {
    int i = begin;

    while (i < size) {
        if (s[i] == '\n') {
            break;
        } else {
            ++i;
        }
    }
    end = i - 1;
    nextBegin = i + 1;
    lineSize = i - begin;
}

/*****************************************************************************/

ParamStruct::ParamStruct(const char* name, ParamFormat format, param_callback ifunc, 
			 int offset1, int offset2, int offset3, int offset4, 
			 int indirection) {
    _name = name ? strdup(name) : nil;
    _format = format;
    _ifunc = ifunc;
    _offset1 = offset1;
    _offset2 = offset2;
    _offset3 = offset3;
    _offset4 = offset4;
    _indirection = indirection;
}

ParamStruct::ParamStruct(ParamStruct& ps) {
    *this = ps;
}

ParamStruct::~ParamStruct() {
    delete _name;
}

void * ParamStruct::addr1(void* base) {
    if (_offset1<0) return nil;
    if (_indirection<0)
	return (char *) base + _offset1;
    else 
	return *(char **)((char*)base+_indirection) + _offset1;
}

void * ParamStruct::addr2(void* base) {
    if (_offset2<0) return nil;
    if (_indirection<0)
	return (char *) base + _offset2;
    else 
	return *(char **)((char*)base+_indirection) + _offset2;
}

void * ParamStruct::addr3(void* base) {
    if (_offset3<0) return nil;
    if (_indirection<0)
	return (char *) base + _offset3;
    else 
	return *(char **)((char*)base+_indirection) + _offset3;
}

void * ParamStruct::addr4(void* base) {
    if (_offset4<0) return nil;
    if (_indirection<0)
	return (char *) base + _offset4;
    else 
	return *(char **)((char*)base+_indirection) + _offset4;
}

/*****************************************************************************/

LexScan* ParamList::_lexscan = nil;
ParamStruct* ParamList::_currstruct = nil;

ParamList::ParamList (ParamList* s) {
    _alist = new AList;
    _count = 0;
    _required_count = 0;
    _optional_count = 0;
    _keyword_count = 0;
    _other_count = 0;

    if (s != nil) {
        ALIterator i;

        for (s->First(i); !s->Done(i); s->Next(i)) {
	    insert(new ParamStruct(*GetStruct(i)));
	}
    }
}

ParamList::~ParamList () { 
    delete _lexscan;
    ALIterator i;
    for (First(i); !Done(i); Next(i)) 
	delete GetStruct(i);
    delete _alist; 
}

LexScan* ParamList::lexscan() {
    if (!_lexscan)
	_lexscan = new LexScan(nil, nil, nil, nil);
    return _lexscan;
}

void ParamList::add_param(const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
			  void* base, void* addr1, void* addr2, void* addr3, void* addr4) {
    ParamStruct* ps = new ParamStruct(
	name, format, ifunc, 
	addr1 ? (char *) addr1 - (char *) base : -1,
	addr2 ? (char *) addr2 - (char *) base : -1, 
	addr3 ? (char *) addr3 - (char *) base : -1, 
	addr4 ? (char *) addr4 - (char *) base : -1);
    insert(ps);
}

void ParamList::add_param_indirect(
    const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
    void* base, void* offset, void* addr1, void* addr2, void* addr3, void* addr4) 
{
    ParamStruct* ps = new ParamStruct(
	name, format, ifunc, 
	addr1 ? (char *) addr1 - *(char **) offset : -1,
	addr2 ? (char *) addr2 - *(char **) offset : -1, 
	addr3 ? (char *) addr3 - *(char **) offset : -1, 
	addr4 ? (char *) addr4 - *(char **) offset : -1,
	(char *) offset - (char *) base);
    insert(ps);
}

void ParamList::add_param_first(const char* name, ParamStruct::ParamFormat format, param_callback ifunc, 
			  void* base, void* addr1, void* addr2, void* addr3, void* addr4) {
    ParamStruct* ps = new ParamStruct(
	name, format, ifunc, 
	addr1 ? (char *) addr1 - (char *) base : -1,
	addr2 ? (char *) addr2 - (char *) base : -1, 
	addr3 ? (char *) addr3 - (char *) base : -1, 
	addr4 ? (char *) addr4 - (char *) base : -1);
    insert_first(ps);
}

void ParamList::insert(ParamStruct* ps) {
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	ParamStruct* ops = GetStruct(i);
	if (ps->format() == ParamStruct::other) break;
	if (ps->format() == ParamStruct::required &&
	    ops->format() != ParamStruct::required) break;
	if (ps->format() == ParamStruct::optional && 
	    ops->format() == ParamStruct::keyword) break;
    }
    InsertBefore(i, ps);
    _count++;
    if (ps->format() == ParamStruct::required) _required_count++;
    if (ps->format() == ParamStruct::optional) _optional_count++;
    if (ps->format() == ParamStruct::keyword) _keyword_count++;
    if (ps->format() == ParamStruct::other) _other_count++;
}

void ParamList::insert_first(ParamStruct* ps) {
    ALIterator i;
    for (First(i); !Done(i); Next(i)) {
	ParamStruct* ops = GetStruct(i);
	if (ps->format() == ParamStruct::other) break;
	if (ps->format() == ParamStruct::required) break;
	if (ps->format() == ParamStruct::keyword && 
	    ops->format() == ParamStruct::keyword) break;
    }
    InsertBefore(i, ps);
    _count++;
    if (ps->format() == ParamStruct::required) _required_count++;
    if (ps->format() == ParamStruct::optional) _optional_count++;
    if (ps->format() == ParamStruct::keyword) _keyword_count++;
    if (ps->format() == ParamStruct::other) _other_count++;
}

boolean ParamList::read_args(istream& in, void* base) {
    int curpar = 0;
    ALIterator i;
    First(i);
    if (_other_count > 0) 
	Next(i); //other always stored first in list
    const char* token;
    char ch;

    /* look for opening ( */
    skip_space(in);
    ch = in.get();
    if (ch != '(') {
	return false; 
    }
    
    /* required fixed format parameters */
    while (curpar < _required_count) {
	skip_space(in);
	ParamStruct* ps = GetStruct(i);
	if ((*ps->ifunc())(in, ps->addr1(base), ps->addr2(base), 
			   ps->addr3(base), ps->addr4(base)) == -1) {
	    cerr << "Error in required parameter " << ps->name() << "\n";
	    return false;
	}
	Next(i);
	curpar++;
    }

    /* optional fixed format parameters that have been supplied */
    while (skip_space(in), in.good() && (ch = in.peek()) != ':' 
	&& (ch = in.peek()) != ')' && curpar < _required_count+_optional_count) {
	ParamStruct* ps = GetStruct(i);
	if ((*ps->ifunc())(in, ps->addr1(base), ps->addr2(base), 
			   ps->addr3(base), ps->addr4(base)) == -1) {
	    cerr << "Error in optional parameter " << ps->name() << "\n";
	    return false;
	}
	Next(i);
	curpar++;
    }

    /* free format parameters, defaulted and otherwise */
    while (skip_space(in), in.good() && (ch = in.get()) != ')') {
	if (ch == ':') {
	    char keyword[BUFSIZE];
	    parse_token(in, keyword, BUFSIZE, ')');
	    ALIterator j(i);
	    boolean match = false;
	    while (!Done(j)) {
		if (strcmp(keyword, GetStruct(j)->name()) == 0) {
		    ParamStruct* ps = GetStruct(j);
		    skip_space(in);
		    if ((*ps->ifunc())(in, ps->addr1(base), ps->addr2(base), 
				       ps->addr3(base), ps->addr4(base)) == -1) {
			cerr << "Error in keyword parameter " << ps->name() << "\n";
			return false;
		    }
		    match = true;
		    break;
		} else
		    Next(j);
	    }
	    if (!match) {
		First(j);
	        ParamStruct* ps = GetStruct(j);
		skip_space(in);
	        if ((*ps->ifunc())(in, ps->addr1(base), ps->addr2(base), 
		   	           ps->addr3(base), keyword) == -1) {
		    cerr << "Error in other parameter " << ps->name() << "\n";
		    return false;
	        }
	    }
	}
    }
    // last '\n'
    if ((ch=in.get()) != '\n') in.putback(ch);	 
    return true;
}

ParamStruct* ParamList::Struct (AList* r) { return (ParamStruct*) (*r)(); }
AList* ParamList::Elem (ALIterator i) { return (AList*) i.GetValue(); }

void ParamList::Append (ParamStruct* v) {
    _alist->Append(new AList(v));
    ++_count;
}

void ParamList::Prepend (ParamStruct* v) {
    _alist->Prepend(new AList(v));
    ++_count;
}

void ParamList::InsertAfter (ALIterator i, ParamStruct* v) {
    Elem(i)->Prepend(new AList(v));
    ++_count;
}

void ParamList::InsertBefore (ALIterator i, ParamStruct* v) {
    Elem(i)->Append(new AList(v));
    ++_count;
}

void ParamList::Remove (ALIterator& i) {
    AList* doomed = Elem(i);

    Next(i);
    _alist->Remove(doomed);
    delete doomed;
    --_count;
}	
    
void ParamList::Remove (ParamStruct* p) {
    AList* temp;

    if ((temp = _alist->Find(p)) != nil) {
	_alist->Remove(temp);
        delete temp;
	--_count;
    }
}

ParamStruct* ParamList::GetStruct (ALIterator i) { _currstruct = Struct(Elem(i)); return _currstruct;}

void ParamList::SetStruct (ParamStruct* gv, ALIterator& i) {
    i.SetValue(_alist->Find(gv));
}

void ParamList::First (ALIterator& i) { i.SetValue(_alist->First()); }
void ParamList::Last (ALIterator& i) { i.SetValue(_alist->Last()); }
void ParamList::Next (ALIterator& i) { i.SetValue(Elem(i)->Next()); }
void ParamList::Prev (ALIterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean ParamList::Done (ALIterator i) { return Elem(i) == _alist->End(); }
int ParamList::Number () { return _count; }

boolean ParamList::Includes (ParamStruct* e) {
    return _alist->Find(e) != nil;
}

boolean ParamList::IsEmpty () { return _alist->IsEmpty(); }

int ParamList::read_int(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int f1, f2, f3, f4;
    char delim;
    if (addr1 && in.good()) {
	in >> f1;
	*(int*)addr1 = f1;
	if (addr2 && in.good()) {
	    in >> delim >> f2;
	    *(int*)addr2 = f2;
	    if (addr3 && in.good()) {
	        in >> delim >> f3;
	        *(int*)addr3 = f3;
	        if (addr4 && in.good()) {
		    in >> delim >> f4;
		    *(int*)addr4 = f4;
		}
	    }
	}
    }
    return in.good() ? 0 : -1;
}

int ParamList::read_float(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    float f1, f2, f3, f4;
    char delim;
    if (addr1 && in.good()) {
	in >> f1;
	*(float*)addr1 = f1;
	if (addr2 && in.good()) {
	    in >> delim >> f2;
	    *(float*)addr2 = f2;
	    if (addr3 && in.good()) {
	        in >> delim >> f3;
	        *(float*)addr3 = f3;
	        if (addr4 && in.good()) {
		    in >> delim >> f4;
		    *(float*)addr4 = f4;
		}
	    }
	}
    }
    return in.good() ? 0 : -1;
}
 
int ParamList::read_double(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    double f1, f2, f3, f4;
    char delim;
    if (addr1 && in.good()) {
	in >> f1;
	*(double*)addr1 = f1;
	if (addr2 && in.good()) {
	    in >> delim >> f2;
	    *(double*)addr2 = f2;
	    if (addr3 && in.good()) {
	        in >> delim >> f3;
	        *(double*)addr3 = f3;
	        if (addr4 && in.good()) {
		    in >> delim >> f4;
		    *(double*)addr4 = f4;
		}
	    }
	}
    }
    return in.good() ? 0 : -1;
}
 
int ParamList::read_string(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    char* s1, s2, s3, s4;
    char delim;
    char buffer[BUFSIZ];
    if (addr1 && in.good()) {
	if (parse_string(in, buffer, BUFSIZ) == 0)
	    *(char **)addr1 = strdup(buffer);
	if (addr2 && in.good()) {
	    if (parse_string(in, buffer, BUFSIZ) == 0)
		*(char **)addr2 = strdup(buffer);
	    if (addr3 && in.good()) {
		if (parse_string(in, buffer, BUFSIZ) == 0)
		    *(char **)addr3 = strdup(buffer);
	        if (addr4 && in.good()) {
		    if (parse_string(in, buffer, BUFSIZ) == 0)
			*(char **)addr4 = strdup(buffer);
		}
	    }
	}
    }
    return in.good() ? 0 : -1;
}
 
int ParamList::read_ints (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int bufsiz = 1024;
    int n = 0;
    int* nums = new int[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    int* newnums = new int[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) 
		newnums[i] = nums[i];
	    delete nums;
	    nums = newnums;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	in >> nums[n];
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    *(int**)addr1 = nums;
    *(int*)addr2 = n;
    return in.good() ? 0 : -1;
}

int ParamList::read_floats (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int bufsiz = 1024;
    int n = 0;
    float* nums = new float[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    float* newnums = new float[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) 
		newnums[i] = nums[i];
	    delete nums;
	    nums = newnums;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	in >> nums[n];
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    *(float**)addr1 = nums;
    *(int*)addr2 = n;
    return in.good() ? 0 : -1;
}

int ParamList::read_doubles (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int bufsiz = 1024;
    int n = 0;
    double* nums = new double[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    double* newnums = new double[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) 
		newnums[i] = nums[i];
	    delete nums;
	    nums = newnums;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	in >> nums[n];
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    *(double**)addr1 = nums;
    *(int*)addr2 = n;
    return in.good() ? 0 : -1;
}

int ParamList::read_strings (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int bufsiz = 32;
    int n = 0;
    char** strings = new char*[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    char** newstrings = new char*[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) 
		newstrings[i] = strings[i];
	    delete strings;
	    strings = newstrings;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	char buffer[BUFSIZ];
	if (parse_string(in, buffer, BUFSIZ) == 0)
	    strings[n] = strdup(buffer);
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    *(char***)addr1 = strings;
    *(int*)addr2 = n;
    return in.good() ? 0 : -1;
}

int ParamList::skip_space (istream& in) {
    char ch;
    while(isspace(ch=in.get()) && in.good());
    if (in.good())
	in.putback(ch);
    return in.good() ? 0 : -1;
}

int ParamList::parse_token (istream& in, char* buf, int buflen, char delim) {
    char ch;
    int cnt = 0;
    while(!isspace(ch=in.get()) && 
	  ch != delim && ch != ')' && in.good() && cnt<buflen-1) {
        buf[cnt] = ch;
        cnt++;
    }
    if (in.good() /* && ch != ')' */ )
	in.putback(ch);
    buf[cnt] = '\0';
    return in.good() && (ch==delim || ch==')') ? 0 : -1;
}

int ParamList::parse_token (istream& in, char* buf, int buflen, char* delim) {
    char ch;
    int cnt = 0;
    while(!isspace(ch=in.get()) && 
	  strchr(delim, ch)==NULL && ch != ')' && in.good() && cnt<buflen-1) {
        buf[cnt] = ch;
        cnt++;
    }
    if (in.good() /* && ch != ')' */ )
	in.putback(ch);
    buf[cnt] = '\0';
    return in.good() && (strchr(delim,ch) || ch==')') ? 0 : -1;
}

int ParamList::parse_string (istream& in, char* buf, int buflen) {
    int cnt = 0;
    char curr_ch = in.get();
    if (curr_ch == '"') {
	curr_ch = in.get();
	char prev_ch = '\0';
        while(in.good() && cnt<buflen-1 && 
	      (curr_ch != '"' || prev_ch == '\\')) {
	    if (curr_ch != '\\') 
		buf[cnt++] = curr_ch;
	    prev_ch = curr_ch;
	    curr_ch = in.get();
        }
        buf[cnt] = '\0';
    }
    return in.good() && curr_ch == '"' ? 0 : -1;
}

int ParamList::parse_points (istream& in, Coord*& x, Coord*& y, int& n) {
    char delim;

    int bufsiz = 1024;
    n = 0;
    x = new Coord[bufsiz];
    y = new Coord[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    Coord* newx = new Coord[bufsiz*2];
	    Coord* newy = new Coord[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) {
		newx[i] = x[i];
		newy[i] = y[i];
	    }
	    delete x;
	    x = newx;
	    delete y;
	    y = newy;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	if ((ch = in.get()) == '(')
	  in >> x[n] >> delim >> y[n] >> delim;
	else {
	  in.putback(ch);
	  in >> x[n] >> delim >> y[n];
	}
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    return in.good() ? 0 : -1;
}

int ParamList::parse_fltpts (istream& in, float*& x, float*& y, int& n) {
    char delim;

    int bufsiz = 1024;
    n = 0;
    x = new float[bufsiz];
    y = new float[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    float* newx = new float[bufsiz*2];
	    float* newy = new float[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) {
		newx[i] = x[i];
		newy[i] = y[i];
	    }
	    delete x;
	    x = newx;
	    delete y;
	    y = newy;
	    bufsiz *= 2;
	}
	
	skip_space(in);
	if ((ch = in.get()) == '(')
	  in >> x[n] >> delim >> y[n] >> delim;
	else {
	  in.putback(ch);
	  in >> x[n] >> delim >> y[n];
	}
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    return in.good() ? 0 : -1;
}

int ParamList::parse_dblpts (istream& in, double*& x, double*& y, int& n) {
    char delim;

    int bufsiz = 1024;
    n = 0;
    x = new double[bufsiz];
    y = new double[bufsiz];

    char ch;
    do {
	if (n >= bufsiz) {
	    double* newx = new double[bufsiz*2];
	    double* newy = new double[bufsiz*2];
	    for (int i=0; i<bufsiz; i++) {
		newx[i] = x[i];
		newy[i] = y[i];
	    }
	    delete x;
	    x = newx;
	    delete y;
	    y = newy;
	    bufsiz *= 2;
	}

	skip_space(in);
	if ((ch = in.get()) == '(')
	  in >> x[n] >> delim >> y[n] >> delim;
	else {
	  in.putback(ch);
	  in >> x[n] >> delim >> y[n];
	}
	skip_space(in);
	n++;
    } while ((ch = in.get()) == ',' && in.good());
    if (in.good()) in.putback(ch);
    
    return in.good() ? 0 : -1;
}

int ParamList::parse_text(istream& in, char* buffer, int buflen) {
    TextBuffer stext(buffer, 0, buflen);
    char null = '\0';
    char c = ',';
    int dot = 0;

    while (c == ',') {
        while (c != '"' && in.get(c));
   
        while (in.get(c) && c != '"') {
            if (c == '\\') {
	        in.get(c);

                if (isdigit(c)) {
		    char buf[4];
		    buf[0] = c;
		    in.get(buf[1]);
		    in.get(buf[2]);
		    buf[3] = '\0';
		    c = ParamList::octal(buf);
                }
                else if (c == 'n')
                    dot += stext.Insert(dot, "\\", 1);
	    }
	dot += stext.Insert(dot, &c, 1);
        }
        in.get(c);
        if (c == ',')
            dot += stext.Insert(dot, "\n", 1);
    }
    in.putback(c);
    stext.Insert(stext.Length(), &null, 1);
    return in.good() ? 0 : -1;
}

char* ParamList::parse_textbuf(istream& in) {
    int buflen = BUFSIZ;
    char* buffer = new char[buflen];

    char null = '\0';
    char c = ',';
    int dot = 0;

    while (c == ',') {
        while (c != '"' && in.get(c));
	
        while (in.get(c) && c != '"') {
            if (c == '\\') {
	        in.get(c);
		
                if (isdigit(c)) {
		    char buf[4];
		    buf[0] = c;
		    in.get(buf[1]);
		    in.get(buf[2]);
		    buf[3] = '\0';
		    c = ParamList::octal(buf);
                }
                else if (c == 'n') {
                    buffer[dot++] = '\\';
		    if (dot == buflen) {
			buflen *= 2;
			char* newbuf = new char[buflen];
			memcpy(newbuf, buffer, dot);
			delete buffer;
			buffer = newbuf;
		    }
		}
	    }
	    buffer[dot++] = c;
	    if (dot == buflen) {
		buflen *= 2;
		char* newbuf = new char[buflen];
		memcpy(newbuf, buffer, dot);
		delete buffer;
		buffer = newbuf;
	    }
        }
        in.get(c);
        if (c == ',') {
            buffer[dot++] = '\n';
	    
	    if (dot == buflen) {
		buflen *= 2;
		char* newbuf = new char[buflen];
		memcpy(newbuf, buffer, dot);
		delete buffer;
		buffer = newbuf;
	    }
	}
	    
    }
    in.putback(c);
    buffer[dot] = '\0';
    if (in.good()) 
	return buffer;
    else {
	delete buffer;
	return nil;
    }
}

int ParamList::output_text(ostream& out, const char* text, int indent) {
    if (!text) {
      out << "(null)";
      return out.good() ? 0 : -1;
    }

    int len = strlen(text);
    int beg, end, lineSize, nextBeg, ypos = 0;
    if (len == 0) 
	out << "\"\"";
    else {
	for (beg = 0; beg < len; ) {
	    GetLine(text, len, beg, end, lineSize, nextBeg);
	    const char* string = filter(&text[beg], end - beg + 1);
	    out << "\"" << string << "\"";
	    beg = nextBeg;
	    if (beg < len) {
		out << "," << "\n";
		for (int i = 0; i < indent; i++)
		    out << "    ";
	    }
	}
    }
    return out.good() ? 0 : -1;
}

int ParamList::parse_pathname (istream& in, char* buf, int buflen, const char* dir) {
    char buf2[buflen];
    if (parse_string(in, buf2, buflen) != 0)
	return -1;
    else if (buf2[0] != '/' && dir && !urltest(buf2)) {
	strncpy( buf, dir, buflen);
	strncat( buf+strlen(dir), buf2, buflen-strlen(dir));
    } else {
	strcpy( buf, buf2 );
    }
    return 0;
}

boolean ParamList::url_use_ok() {
  return bincheck("ivdl") || bincheck("w3c") || bincheck("curl") || bincheck("wget");
}

boolean ParamList::urltest(const char* buf) {
  if (!buf) return false;
  static boolean file_url_ok = bincheck("w3c") || bincheck("curl");
  return 
    strncasecmp("http://", buf, 7)==0 || 
    strncasecmp("ftp://", buf, 6)==0 || 
    file_url_ok && strncasecmp("file:/", buf, 6)==0 ;
}

int ParamList::bintest(const char* command) {
  char combuf[BUFSIZ];
  sprintf( combuf, "sh -c \"wr=`which %s`; echo $wr\"", command );
  FILE* fptr = popen(combuf, "r");
  char testbuf[BUFSIZ];	
  fgets(testbuf, BUFSIZ, fptr);  
  pclose(fptr);
  if (strncmp(testbuf+strlen(testbuf)-strlen(command)-1, 
	      command, strlen(command)) != 0) {
    return -1;
  }
  return 0;
}

boolean ParamList::bincheck(const char* command) {
  int status = bintest(command);
  return !status;
}

// octal converts a character to the string \ddd where d is an octal digit.

char* ParamList::octal(unsigned char c, register char* p) {
    *p-- = '\0';		// backwards from terminating null...
    *p-- = (char)('0' + c%8);
    *p-- = (char)('0' + (c >>= 3)%8);
    *p-- = (char)('0' + (c >>= 3)%8);
    *p = '\\';			// ...to beginning backslash
    return p;
}

// octal converts a string of three octal digits to a character.

char ParamList::octal(const char* p) {
    char c = *p - '0';
    c = c*8 + *++p - '0';
    c = c*8 + *++p - '0';
    return c;
}

// filter escapes embedded special characters

const char* ParamList::filter (const char* string, int len) {
    TextBuffer text(textbuf, 0, BUFSIZE);
    int dot;
    for (dot = 0; len--; string++) {
	char c = *string;

	if (!isascii(c) || iscntrl(c)) {
	    char buf[5];
	    octal(c, &buf[sizeof(buf) - 1]);
	    dot += text.Insert(dot, buf, sizeof(buf) - 1);

	} else {
	    if (c == '\\' || c == '"') 
	      dot += text.Insert(dot, "\\", 1);
	    dot += text.Insert(dot, string, 1);
	}
    }
    text.Insert(dot, "", 1);

    return text.Text();
}
