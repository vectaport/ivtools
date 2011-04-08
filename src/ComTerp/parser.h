/*
 * Copyright (c) 1994, 1995, 1998, 1999 Vectaport Inc.
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
 * Parser creates and returns a postfix buffer of ComTerp tokens.
 */

#if !defined(_parser_h)
#define _parser_h

#include <ComTerp/commodule.h>
#include <ComTerp/_comterp.h>

class istream;

//: C++ wrapper for ComUtil parser capability.
class Parser : public ComTerpModule {
public:
    Parser();
    Parser(const char* path);
    Parser(void*, char*(*)(char*,int,void*), int(*)(void*), int(*)(void*));
    // see descriptions in ComTerp or ComTerpModule.
    Parser(istream&);
    ~Parser();

    int print_next_expr();
    postfix_token* copy_postfix_tokens(int& ntokens);
    // make a copy of current buffer of postfix tokens.

    boolean skip_matched_parens();
    // support for '()', '{}', and '[]'.

protected:
    void init();

    static char* istream_fgets(char* s, int n, void* istreamptr);
    // signature like fgets for reading from an istream.
    static int istream_feof(void* istreamptr);
    // signature like feof for passing on istream end-of-file.
    static int istream_ferror(void* istreamptr);
    // signature like feof for passing on istream error info.

protected:
    postfix_token* _pfbuf;
    unsigned int _pfsiz;
    unsigned int _pfnum;
};

#endif /* !defined(_parser_h) */
