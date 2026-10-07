#include "shim.h"

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>

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
