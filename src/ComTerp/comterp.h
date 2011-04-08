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
 * ComTerp is a command interpreter derived from the Parser
 */

#if !defined(_comterp_h)
#define _comterp_h

#include <ComTerp/parser.h>

#include <OS/table.h>
declareTable(ComValueTable,int,void*)


class AttributeList;
class ComFunc;
class ComValue;
class ostream;

class ComterpHandler;

class ComTerp : public Parser {
public:
    ComTerp();
    ComTerp(const char* path);
    ComTerp(void*, char*(*)(char*,int,void*), int(*)(void*), int(*)(void*));
    ~ComTerp();

    void init();

    boolean read_expr();
    boolean eof();

    virtual int eval_expr(boolean nested=false);

    int print_stack_top() const;
    int print_stack_top(ostream& out) const;
    int print_stack() const;
    int stack_height() { return _stack_top+1; }
    boolean brief() const;

    int add_command(const char* name, ComFunc*);
    void list_commands(ostream& out);

    ComValue& pop_stack();
    ComValue& lookup_symval(ComValue&);
    ComValue& stack_top(int n=0);
    ComValue& pop_symbol();
    void push_stack(ComValue&);
    void incr_stack();
    void incr_stack(int n);
    void decr_stack(int n=1);
    boolean stack_empty() { return _stack_top<0; }

    static ComTerp& instance();

    void quit(boolean quitflag=true);
    boolean quitflag();
    virtual void exit(int status=0);

    virtual int run();
    virtual int runfile(const char* filename);
    void add_defaults();

    ComValueTable* localtable() const { return _localtable; }
    ComValueTable* globaltable() const { return _globaltable; }
    ComValue* localvalue(int symid);
    ComValue* globalvalue(int symid);
    ComValue* eithervalue(int symid, boolean globalfirst=false);

    const char* errmsg() { return _errbuf; }

    void set_attributes(AttributeList*);
    AttributeList* get_attributes();

    void handler(ComterpHandler* h );
    ComterpHandler* handler();

protected:
    void push_stack(postfix_token*);
    const ComValue* stack(unsigned int &top) const;
    int load_sub_expr();
    void load_postfix(postfix_token*, int toklen, int tokoff);

protected:
    ComValue* _stack;
    int _stack_top;
    unsigned int _stack_siz;
    boolean _quitflag;
    char* _errbuf;
    int _pfoff;
    boolean _brief;
    boolean _just_reset;

    ComValueTable* _localtable;
    static ComValueTable* _globaltable;
    AttributeList* _alist;

    static ComTerp* _instance;

    ComterpHandler* _handler;

    friend ComFunc;
};

#endif /* !defined(_comterp_h) */
