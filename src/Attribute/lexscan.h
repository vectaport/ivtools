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

#if !defined(_lexscan_h)
#define _lexscan_h

#include <Attribute/attrvalue.h>
#include <Attribute/commodule.h>

//: Lexical scanner for C tokens, with C-style comment skipping.
class LexScan : public ComTerpModule {
public:
    LexScan();
    // construct on stdin.
    LexScan(const char* path);
    // construct on 'path'.
    LexScan(void*, char*(*)(char*,int,void*), int(*)(void*), int(*)(void*));
    // construct on arbitrary function pointers, with arbitrary void* 
    // pointer to pass to them.
    ~LexScan();

    void init();
    // intialize comment delimeters and internal token buffer.

    attr_value get_next_token(unsigned int& toktype);         
    // return token in structure ready for constructing an AttributeValue.
    const char* get_next_token_string(unsigned int& toktype);
    // return pointer to next token in internal buffer.

    AttributeValue* get_attr(char* buf, unsigned int bufsiz);
    // return token in newly allocated AttributeValue.

protected:
    char* _begcmt;
    char* _endcmt;
    char* _tokbuf;
};

#endif /* !defined(_lexscan_h) */
