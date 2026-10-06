#include "shim.h"

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>

#include <cstring>
#include <sstream>

struct comterp_bridge {
    ComTerpServ* terp;
    char* result;
};

comterp_handle comterp_bridge_new(void) {
    comterp_bridge* h = new comterp_bridge;
    h->terp = new ComTerpServ(BUFSIZ * BUFSIZ);
    h->terp->add_defaults();
    h->result = nil;
    return h;
}

void comterp_bridge_free(comterp_handle handle) {
    comterp_bridge* h = (comterp_bridge*)handle;
    delete[] h->result;
    delete h->terp;
    delete h;
}

const char* comterp_bridge_eval(comterp_handle handle, const char* expr) {
    comterp_bridge* h = (comterp_bridge*)handle;
    delete[] h->result;
    h->result = nil;

    ComValue val(h->terp->run(expr));
    if (*h->terp->errmsg()) return nil;

    /* ComValue::String() prints the object's address, not its value
       (comvalue.c:526 streams `this` instead of `*this`) -- stream the
       value directly through operator<< instead until that's fixed. */
    ComValue::comterp(h->terp);
    std::ostringstream out;
    out << val;
    std::string str = out.str();
    h->result = new char[str.size() + 1];
    strcpy(h->result, str.c_str());
    return h->result;
}

const char* comterp_bridge_errmsg(comterp_handle handle) {
    comterp_bridge* h = (comterp_bridge*)handle;
    return h->terp->errmsg();
}

int comterp_bridge_ring_info(comterp_handle handle, const char* name, comterp_ring_info* out) {
    comterp_bridge* h = (comterp_bridge*)handle;
    int symid = symbol_find(name);
    if (symid < 0) return 0;
    ComValue* val = h->terp->localvalue(symid);
    if (!val || !val->is_stream()) return 0;

    AttributeValueList* avl = val->stream_list();
    if (!avl || avl->Number() < 5) return 0;

    AttributeValue* bufav = (AttributeValue*)avl->Get(0);
    AttributeValue* headav = (AttributeValue*)avl->Get(1);
    AttributeValue* tailav = (AttributeValue*)avl->Get(2);
    AttributeValue* countav = (AttributeValue*)avl->Get(3);
    AttributeValue* wrapav = (AttributeValue*)avl->Get(4);

    /* same layout strmfunc.c's ring_buf_base/ring_elemsz/ring_buf_cap
       (static to that file) read -- reconstructed here from the public
       ComValue accessors they're themselves built on. */
    ComValue bufv(*bufav);
    int elemsz = bufv.blocksz() > 0 ? bufv.blocksz() : 1;
    int bytecap = bufv.sliced() ? bufv.slicelen() : symbol_len(bufv.string_val());

    out->buf = (char*)bufv.string_ptr() + (bufv.sliced() ? bufv.sliceoff() : 0);
    out->elemsz = elemsz;
    out->cap = bytecap / elemsz;
    out->head = &headav->int_ref();
    out->tail = &tailav->int_ref();
    out->count = &countav->int_ref();
    out->wrap = &wrapav->int_ref();
    return 1;
}

void comterp_bridge_encode_int(long val, char* chunk) {
    ComValue v((int)val);
    ComValue::comval_encode(chunk, v, AttributeValue::AnyType);
}

int comterp_bridge_decode_int(const char* chunk, long* out) {
    ComValue v = ComValue::comval_decode(chunk, AttributeValue::AnyType);
    if (!v.is_type(ComValue::IntType)) return 0;
    *out = v.int_val();
    return 1;
}
