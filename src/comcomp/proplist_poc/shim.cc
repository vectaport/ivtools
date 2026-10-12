#include "shim.h"

#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>
#include <Attribute/aliterator.h>

#include <cstring>
#include <sstream>

struct proplist_box {
    AttributeList* al;
    char* buf;
};

static void set_buf(proplist_box* h, const std::string& s) {
    delete[] h->buf;
    h->buf = new char[s.size() + 1];
    strcpy(h->buf, s.c_str());
}

proplist_handle proplist_new(void) {
    proplist_box* h = new proplist_box;
    h->al = new AttributeList;
    h->al->ref();
    h->buf = nil;
    return h;
}

void proplist_free(proplist_handle handle) {
    proplist_box* h = (proplist_box*)handle;
    h->al->unref();
    delete[] h->buf;
    delete h;
}

void proplist_set(proplist_handle handle, const char* key, const char* val) {
    proplist_box* h = (proplist_box*)handle;
    Attribute* attr = h->al->GetAttr(key);
    if (attr) {
        attr->Value(new AttributeValue(val));
    } else {
        h->al->add_attr(key, new AttributeValue(val));
    }
}

const char* proplist_get(proplist_handle handle, const char* key, int* found) {
    proplist_box* h = (proplist_box*)handle;
    Attribute* attr = h->al->GetAttr(key);
    if (!attr) {
        *found = 0;
        return nil;
    }
    *found = 1;
    AttributeValue* v = attr->Value();
    set_buf(h, v->is_string() ? v->string_ptr() : "");
    return h->buf;
}

int proplist_has(proplist_handle handle, const char* key) {
    proplist_box* h = (proplist_box*)handle;
    return h->al->GetAttr(key) != nil;
}

int proplist_delete(proplist_handle handle, const char* key) {
    proplist_box* h = (proplist_box*)handle;
    Attribute* attr = h->al->GetAttr(key);
    if (!attr) return 0;
    h->al->Remove(attr);  // unrefs and (at refcount 0) deletes attr itself
    return 1;
}

int proplist_len(proplist_handle handle) {
    proplist_box* h = (proplist_box*)handle;
    return h->al->Number();
}

const char* proplist_key_at(proplist_handle handle, int i) {
    proplist_box* h = (proplist_box*)handle;
    ALIterator it;
    h->al->First(it);
    int n = 0;
    while (!h->al->Done(it)) {
        if (n == i) {
            Attribute* attr = h->al->GetAttr(it);
            set_buf(h, attr->Name());
            return h->buf;
        }
        h->al->Next(it);
        n++;
    }
    return nil;
}

int proplist_touch(proplist_handle handle, const char* key) {
    proplist_box* h = (proplist_box*)handle;
    Attribute* attr = h->al->GetAttr(key);
    if (!attr) return 0;
    // Remove() unrefs attr, which deletes it (its refcount is 1, owned
    // solely by the list) -- copy name/value out first, then re-add as
    // a fresh Attribute to move it to the end.
    std::string name = attr->Name();
    AttributeValue* val = new AttributeValue(*attr->Value());
    h->al->Remove(attr);
    h->al->add_attr(name.c_str(), val);
    return 1;
}

const char* proplist_print(proplist_handle handle) {
    proplist_box* h = (proplist_box*)handle;
    std::ostringstream out;
    h->al->serialize(out, true);
    set_buf(h, out.str());
    return h->buf;
}
