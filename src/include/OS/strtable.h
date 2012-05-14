/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 * Copyright (c) 2010 Wave Semiconductor Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * String to object association table.
 */

#ifndef os_strtable_h
#define os_strtable_h

#include <OS/enter-scope.h>

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __StrTableEntry(StrTable) StrTable##_Entry
#define StrTableEntry(StrTable) __StrTableEntry(StrTable)
#define __StrTableIterator(StrTable) StrTable##_Iterator
#define StrTableIterator(StrTable) __StrTableIterator(StrTable)
#else
#define __StrTableEntry(StrTable) StrTable/**/_Entry
#define StrTableEntry(StrTable) __StrTableEntry(StrTable)
#define __StrTableIterator(StrTable) StrTable/**/_Iterator
#define StrTableIterator(StrTable) __StrTableIterator(StrTable)
#endif

#define declareStrTable(StrTable,Value) \
struct StrTableEntry(StrTable); \
\
class StrTable { \
public: \
    StrTable(int); \
    ~StrTable(); \
\
    void insert(const char*, Value); \
    boolean find(Value&, const char*); \
    boolean find_and_remove(Value&, const char*); \
    void remove(const char*); \
private: \
    friend class StrTableIterator(StrTable); \
\
    int size_; \
    StrTableEntry(StrTable)** first_; \
    StrTableEntry(StrTable)** last_; \
\
    StrTableEntry(StrTable)*& probe(const char*); \
}; \
\
struct StrTableEntry(StrTable) { \
private: \
    friend class StrTable; \
    friend class StrTableIterator(StrTable); \
\
    const char* key_; \
    Value value_; \
    StrTableEntry(StrTable)* chain_; \
}; \
\
class StrTableIterator(StrTable) { \
public: \
    StrTableIterator(StrTable)(StrTable&); \
\
    StrTableEntry(StrTable)* cur_entry(); \
    const char*& cur_key(); \
    Value& cur_value(); \
    boolean more(); \
    boolean next(); \
private: \
    StrTableEntry(StrTable)* cur_; \
    StrTableEntry(StrTable)** entry_; \
    StrTableEntry(StrTable)** last_; \
}; \
\
inline StrTableEntry(StrTable)* StrTableIterator(StrTable)::cur_entry() { return cur_; } \
inline const char*& StrTableIterator(StrTable)::cur_key() { return cur_->key_; } \
inline Value& StrTableIterator(StrTable)::cur_value() { return cur_->value_; } \
inline boolean StrTableIterator(StrTable)::more() { return entry_ <= last_; }

/*
 * Predefined hash functions
 */

inline unsigned long skey_to_hash(const char* str) { 
  int len = strlen(str);
  int numbytes = sizeof(unsigned long);
  int maxbytes = numbytes > len ? len : numbytes;
  unsigned char retval[numbytes];
  unsigned long *retvalptr = (unsigned long*)retval;
  *retvalptr = 0;
  for(int i=0; i<maxbytes; i++) retval[i] = str[i];
  for(int i=0; i<maxbytes; i++) retval[(i+1)%numbytes] ^= str[len-i-1];
  return *retvalptr;
}

/*
 * StrTable implementation
 */

#define implementStrTable(StrTable,Value) \
StrTable::StrTable(int n) { \
    for (size_ = 32; size_ < n; size_ <<= 1); \
    first_ = new StrTableEntry(StrTable)*[size_]; \
    --size_; \
    last_ = &first_[size_]; \
    for (register StrTableEntry(StrTable)** e = first_; e <= last_; e++) { \
	*e = nil; \
    } \
} \
\
StrTable::~StrTable() { \
    for (register StrTableEntry(StrTable)** e = first_; e <= last_; e++) { \
	StrTableEntry(StrTable)* t = *e; \
	delete t; \
    } \
    delete[] first_; \
} \
\
inline StrTableEntry(StrTable)*& StrTable::probe(const char* i) { \
    return first_[skey_to_hash(i) & size_]; \
} \
\
void StrTable::insert(const char* k, Value v) { \
    register StrTableEntry(StrTable)* e = new StrTableEntry(StrTable); \
    e->key_ = k; \
    e->value_ = v; \
    register StrTableEntry(StrTable)** a = &probe(k); \
    e->chain_ = *a; \
    *a = e; \
} \
\
boolean StrTable::find(Value& v, const char* k) { \
    for (register StrTableEntry(StrTable)* e = probe(k); e != nil; e = e->chain_) { \
        if (strcmp(e->key_,k)==0) {						\
	    v = e->value_; \
	    return true; \
	} \
    } \
    return false; \
} \
\
boolean StrTable::find_and_remove(Value& v, const char* k) { \
    StrTableEntry(StrTable)** a = &probe(k); \
    register StrTableEntry(StrTable)* e = *a; \
    if (e != nil) { \
        if (strcmp(e->key_, k)==0) {			\
	    v = e->value_; \
	    *a = e->chain_; \
	    delete e; \
	    return true; \
	} else { \
	    register StrTableEntry(StrTable)* prev; \
	    do { \
		prev = e; \
		e = e->chain_; \
	    } while (e != nil && strcmp(e->key_,k)!=0 );	\
	    if (e != nil) { \
		v = e->value_; \
		prev->chain_ = e->chain_; \
		delete e; \
		return true; \
	    } \
	} \
    } \
    return false; \
} \
\
void StrTable::remove(const char* k) { \
    StrTableEntry(StrTable)** a = &probe(k); \
    register StrTableEntry(StrTable)* e = *a; \
    if (e != nil) { \
        if (strcmp(e->key_,k)==0) {			\
	    *a = e->chain_; \
	    delete e; \
	} else { \
	    register StrTableEntry(StrTable)* prev; \
	    do { \
		prev = e; \
		e = e->chain_; \
	    } while (e != nil && strcmp(e->key_,k)!=0 );	\
	    if (e != nil) { \
		prev->chain_ = e->chain_; \
		delete e; \
	    } \
	} \
    } \
} \
\
StrTableIterator(StrTable)::StrTableIterator(StrTable)(StrTable& t) { \
    last_ = t.last_; \
    for (entry_ = t.first_; entry_ <= last_; entry_++) { \
	cur_ = *entry_; \
	if (cur_ != nil) { \
	    break; \
	} \
    } \
} \
\
boolean StrTableIterator(StrTable)::next() { \
    cur_ = cur_->chain_; \
    if (cur_ != nil) { \
	return true; \
    } \
    for (++entry_; entry_ <= last_; entry_++) { \
	cur_ = *entry_; \
	if (cur_ != nil) { \
	    return true; \
	} \
    } \
    return false; \
}

#endif
