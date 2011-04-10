/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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
 * ComTerpServ is a ComTerp with added ability to load its input from a buffer.
 */

#ifndef comterpserv_h
#define comterpserv_h

/*
 * Server-oriented interpreter deals with strings
 */

#include <ComTerp/comterp.h>

class ComTerpServ : public ComTerp {
public:
    ComTerpServ(int bufsize = 1024, int fd = -1);
    ~ComTerpServ();

    void load_string(const char*);
    void read_string(const char*);
    postfix_token* gen_code(const char*, int& codelen);

    virtual int run();
    virtual ComValue& run(const char*, boolean nested=false);
    virtual ComValue& run(postfix_token*, int);
    
    virtual int runfile(const char*);

    void add_defaults();

protected:

    static char* s_fgets(char* s, int n, void* serv);
    static int s_feof(void* serv);
    static int s_ferror(void* serv);
    static int s_fputs(const char* s, void* serv);
    static char* fd_fgets(char* s, int n, void* serv);
    static int fd_fputs(const char* s, void* serv);

protected:
    char* _instr;
    int _inpos;
    char* _outstr;
    int _outpos;
    int _fd;
    FILE* _fptr;
    int _instat;

    friend class ComterpHandler;
    friend class ComTerpIOHandler;
};

#endif
