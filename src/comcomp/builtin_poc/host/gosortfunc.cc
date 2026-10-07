#include "gosortfunc.h"
#include "gosort/libgosort.h"

#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>

#include <sstream>
#include <cstdlib>
#include <cstring>

GoSortFunc::GoSortFunc(ComTerp* comterp) : ComFunc(comterp) {}

// execute -- serialize the list argument to one CSV string, call the
// compiled Go sort through the c-archive boundary, parse the one CSV
// string back: the string-at-the-boundary design Scott asked for, so a
// timing comparison isn't paying for per-element ComValue marshaling on
// either side.
void GoSortFunc::execute() {
    ComValue listv(stack_arg(0));
    reset_stack();

    if (!listv.is_type(ComValue::ArrayType) || !listv.array_val()) {
        push_stack(ComValue::nullval());
        return;
    }

    AttributeValueList* in = listv.array_val();
    std::ostringstream csv;
    ALIterator it;
    boolean first = true;
    for (in->First(it); !in->Done(it); in->Next(it)) {
        AttributeValue* elt = in->GetAttrVal(it);
        if (!first) csv << ",";
        first = false;
        csv << (elt ? elt->int_val() : 0);
    }

    std::string csvstr = csv.str();
    char* result = SortIntsCSV(const_cast<char*>(csvstr.c_str()));

    AttributeValueList* out = new AttributeValueList();
    char* tok = strtok(result, ",");
    while (tok) {
        out->Append(new AttributeValue(atol(tok)));
        tok = strtok(nil, ",");
    }
    free(result);

    ComValue retval(out);
    push_stack(retval);
}
