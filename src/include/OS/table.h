/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 * Generic object association table.
 */

#ifndef os_table_h
#define os_table_h

#include <OS/enter-scope.h>

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __TableEntry(Table) Table##_Entry
#define TableEntry(Table) __TableEntry(Table)
#define __TableGen(Table) Table##_Gen
#define TableGen(Table) __TableGen(Table)
#define __TableIterator(Table) Table##_Iterator
#define TableIterator(Table) __TableIterator(Table)
#else
#define __TableEntry(Table) Table/**/_Entry
#define TableEntry(Table) __TableEntry(Table)
#define __TableGen(Table) Table/**/_Gen
#define TableGen(Table) __TableGen(Table)
#define __TableIterator(Table) Table/**/_Iterator
#define TableIterator(Table) __TableIterator(Table)
#endif

/*
 * A table grows by adding a new, larger bucket array and directing all
 * future inserts to it; a full entry never moves once inserted, so no
 * live iterator or lookup into an older generation is ever invalidated.
 * find()/remove() fall back through older generations, oldest last, when
 * a key isn't in the current one. TableGen holds one such retired
 * generation's array.
 */
#define declareTable(Table,Key,Value) \
struct TableEntry(Table); \
struct TableGen(Table); \
\
class Table { \
public: \
    Table(int); \
    ~Table(); \
\
    void insert(Key, Value); \
    boolean find(Value&, Key); \
    boolean find_and_remove(Value&, Key); \
    void remove(Key); \
private: \
    friend class TableIterator(Table); \
\
    int size_; \
    TableEntry(Table)** first_; \
    TableEntry(Table)** last_; \
    TableGen(Table)* older_; \
\
    TableEntry(Table)*& probe(Key); \
    void grow(); \
}; \
\
struct TableEntry(Table) { \
private: \
    friend class Table; \
    friend class TableIterator(Table); \
\
    Key key_; \
    Value value_; \
    TableEntry(Table)* chain_; \
}; \
\
struct TableGen(Table) { \
    TableEntry(Table)** first_; \
    TableEntry(Table)** last_; \
    int size_; \
    TableGen(Table)* older_; \
}; \
\
class TableIterator(Table) { \
public: \
    TableIterator(Table)(Table&); \
\
    TableEntry(Table)* cur_entry(); \
    Key& cur_key(); \
    Value& cur_value(); \
    boolean more(); \
    boolean next(); \
private: \
    TableEntry(Table)* cur_; \
    TableEntry(Table)** entry_; \
    TableEntry(Table)** last_; \
    TableGen(Table)* remaining_; \
}; \
\
inline TableEntry(Table)* TableIterator(Table)::cur_entry() { return cur_; } \
inline Key& TableIterator(Table)::cur_key() { return cur_->key_; } \
inline Value& TableIterator(Table)::cur_value() { return cur_->value_; } \
inline boolean TableIterator(Table)::more() { return entry_ <= last_; }

/*
 * Predefined hash functions
 */

#ifndef os_table2_h
inline unsigned long key_to_hash(long k) { return (unsigned long)k; }
inline unsigned long key_to_hash(const void* k) { return (unsigned long)k; }
#endif

/*
 * Table implementation
 */

#define implementTable(Table,Key,Value) \
Table::Table(int n) { \
    for (size_ = 32; size_ < n; size_ <<= 1); \
    first_ = new TableEntry(Table)*[size_]; \
    --size_; \
    last_ = &first_[size_]; \
    for (TableEntry(Table)** e = first_; e <= last_; e++) { \
	*e = nil; \
    } \
    older_ = nil; \
} \
\
Table::~Table() { \
    for (TableEntry(Table)** e = first_; e <= last_; e++) { \
	TableEntry(Table)* t = *e; \
	while (t) { \
	    TableEntry(Table)* next = t->chain_; \
	    delete t; \
	    t = next; \
	} \
    } \
    delete[] first_; \
    for (TableGen(Table)* g = older_; g != nil; ) { \
	for (TableEntry(Table)** e = g->first_; e <= g->last_; e++) { \
	    TableEntry(Table)* t = *e; \
	    while (t) { \
		TableEntry(Table)* next = t->chain_; \
		delete t; \
		t = next; \
	    } \
	} \
	delete[] g->first_; \
	TableGen(Table)* older = g->older_; \
	delete g; \
	g = older; \
    } \
} \
\
inline TableEntry(Table)*& Table::probe(Key i) { \
    return first_[key_to_hash(i) & size_]; \
} \
\
void Table::grow() { \
    TableGen(Table)* g = new TableGen(Table); \
    g->first_ = first_; \
    g->last_ = last_; \
    g->size_ = size_; \
    g->older_ = older_; \
    older_ = g; \
    size_ = ((size_ + 1) << 1) - 1; \
    first_ = new TableEntry(Table)*[size_ + 1]; \
    last_ = &first_[size_]; \
    for (TableEntry(Table)** e = first_; e <= last_; e++) { \
	*e = nil; \
    } \
} \
\
void Table::insert(Key k, Value v) { \
    TableEntry(Table)** a = &probe(k); \
    int chainlen = 0; \
    for (TableEntry(Table)* e = *a; e != nil; e = e->chain_) { \
	chainlen++; \
    } \
    if (chainlen >= (size_ + 1) >> 3) { \
	grow(); \
	a = &probe(k); \
    } \
    TableEntry(Table)* e = new TableEntry(Table); \
    e->key_ = k; \
    e->value_ = v; \
    e->chain_ = *a; \
    *a = e; \
} \
\
boolean Table::find(Value& v, Key k) { \
    for (TableEntry(Table)* e = probe(k); e != nil; e = e->chain_) { \
	if (e->key_ == k) { \
	    v = e->value_; \
	    return true; \
	} \
    } \
    for (TableGen(Table)* g = older_; g != nil; g = g->older_) { \
	for (TableEntry(Table)* e = g->first_[key_to_hash(k) & g->size_]; \
	     e != nil; e = e->chain_) { \
	    if (e->key_ == k) { \
		v = e->value_; \
		return true; \
	    } \
	} \
    } \
    return false; \
} \
\
boolean Table::find_and_remove(Value& v, Key k) { \
    TableEntry(Table)** a = &probe(k); \
    TableEntry(Table)* e = *a; \
    if (e != nil) { \
	if (e->key_ == k) { \
	    v = e->value_; \
	    *a = e->chain_; \
	    delete e; \
	    return true; \
	} else { \
	    TableEntry(Table)* prev; \
	    do { \
		prev = e; \
		e = e->chain_; \
	    } while (e != nil && e->key_ != k); \
	    if (e != nil) { \
		v = e->value_; \
		prev->chain_ = e->chain_; \
		delete e; \
		return true; \
	    } \
	} \
    } \
    for (TableGen(Table)* g = older_; g != nil; g = g->older_) { \
	a = &g->first_[key_to_hash(k) & g->size_]; \
	e = *a; \
	if (e != nil) { \
	    if (e->key_ == k) { \
		v = e->value_; \
		*a = e->chain_; \
		delete e; \
		return true; \
	    } else { \
		TableEntry(Table)* prev; \
		do { \
		    prev = e; \
		    e = e->chain_; \
		} while (e != nil && e->key_ != k); \
		if (e != nil) { \
		    v = e->value_; \
		    prev->chain_ = e->chain_; \
		    delete e; \
		    return true; \
		} \
	    } \
	} \
    } \
    return false; \
} \
\
void Table::remove(Key k) { \
    TableEntry(Table)** a = &probe(k); \
    TableEntry(Table)* e = *a; \
    if (e != nil) { \
	if (e->key_ == k) { \
	    *a = e->chain_; \
	    delete e; \
	    return; \
	} else { \
	    TableEntry(Table)* prev; \
	    do { \
		prev = e; \
		e = e->chain_; \
	    } while (e != nil && e->key_ != k); \
	    if (e != nil) { \
		prev->chain_ = e->chain_; \
		delete e; \
		return; \
	    } \
	} \
    } \
    for (TableGen(Table)* g = older_; g != nil; g = g->older_) { \
	a = &g->first_[key_to_hash(k) & g->size_]; \
	e = *a; \
	if (e != nil) { \
	    if (e->key_ == k) { \
		*a = e->chain_; \
		delete e; \
		return; \
	    } else { \
		TableEntry(Table)* prev; \
		do { \
		    prev = e; \
		    e = e->chain_; \
		} while (e != nil && e->key_ != k); \
		if (e != nil) { \
		    prev->chain_ = e->chain_; \
		    delete e; \
		    return; \
		} \
	    } \
	} \
    } \
} \
\
TableIterator(Table)::TableIterator(Table)(Table& t) { \
    last_ = t.last_; \
    remaining_ = t.older_; \
    for (entry_ = t.first_; entry_ <= last_; entry_++) { \
	cur_ = *entry_; \
	if (cur_ != nil) { \
	    break; \
	} \
    } \
    while (cur_ == nil && entry_ > last_ && remaining_ != nil) { \
	entry_ = remaining_->first_; \
	last_ = remaining_->last_; \
	remaining_ = remaining_->older_; \
	for (; entry_ <= last_; entry_++) { \
	    cur_ = *entry_; \
	    if (cur_ != nil) { \
		break; \
	    } \
	} \
    } \
} \
\
boolean TableIterator(Table)::next() { \
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
    while (remaining_ != nil) { \
	entry_ = remaining_->first_; \
	last_ = remaining_->last_; \
	remaining_ = remaining_->older_; \
	for (; entry_ <= last_; entry_++) { \
	    cur_ = *entry_; \
	    if (cur_ != nil) { \
		return true; \
	    } \
	} \
    } \
    return false; \
}

#endif
