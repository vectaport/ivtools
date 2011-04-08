/*
 * Copyright (c) 1996,1999 Vectaport Inc.
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

#include <Attribute/aliterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Attribute/lexscan.h>
#include <Attribute/paramlist.h>

#include <AttrGlyph/attredit.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/font.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <IV-look/dialogs.h>
#include <IV-look/kit.h>
#include <OS/string.h>

#include <IVGlyph/textedit.h>

#include <strstream.h>
#include <string.h>
#if LibStdCPlusPlus
#define STL_VECTOR
#include <vector.h>
#endif

declareActionCallback(AttributeListEditor)
implementActionCallback(AttributeListEditor)
declareFieldEditorCallback(AttributeListEditor)
implementFieldEditorCallback(AttributeListEditor)

AttributeListEditor::AttributeListEditor(AttributeList* al)
: Patch(nil) {
    _list = al;
    Resource::ref(_list);
    DialogKit& dk = *DialogKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    _namefe = dk.field_editor("", wk.style(), 
			      new FieldEditorCallback(AttributeListEditor)
			      (this, &AttributeListEditor::fe_add, 
			       &AttributeListEditor::fe_clr));
    _valfe = dk.field_editor("", wk.style(),
			     new FieldEditorCallback(AttributeListEditor)
			     (this, &AttributeListEditor::fe_add, 
			      &AttributeListEditor::fe_clr));
    Style* s = new Style(wk.style());
    s->attribute("rows", "10");
    s->attribute("columns", "30");
    _ete = new EivTextEditor(s, false);
    _ete->ref();
    build();
}

AttributeListEditor::~AttributeListEditor() {
//    _ete->unref();
  Resource::unref(_list);
}

AttributeList* AttributeListEditor::list() {
    return _list;
}

void AttributeListEditor::add() {
    const String* txt = _namefe->text();
    if (txt->length() > 0) {
	char* buf = new char[strlen(_valfe->text()->string())+2];
	sprintf(buf, "%s\n", _valfe->text()->string());
	_list->add_attr(txt->string(), ParamList::lexscan()->get_attr(buf, strlen(buf)));
	update_text(true);
    }
}

void AttributeListEditor::remove() {
    const String* txt = _namefe->text();
    if (txt->length() > 0) {
	Attribute* attr;
	if (attr = _list->GetAttr(txt->string())) {
	    _list->Remove(attr);
	    update_text(true);
	}
    }
}

void AttributeListEditor::update_text(boolean update) {
    ALIterator i;
#ifndef STL_VECTOR
    char buf[1024];
    memset(buf, 0, 1024);
#else
    vector <char> vbuf;
#endif
    for (_list->First(i); !_list->Done(i); _list->Next(i)) {
	Attribute* attr = _list->GetAttr(i);
	const char* name = attr->Name();
	int namelen = name ? strlen(name) : 0;
	if (name)
#ifndef STL_VECTOR
	    strcat(buf, name);
#else
	{
	    const char* namep = name;
	    while (*namep) { vbuf.push_back(*namep++); }
	}
#endif
	int n;
	for (n = 15; n > namelen-1; n--)
#ifndef STL_VECTOR
	    strcat(buf, " ");
#else
	    vbuf.push_back(' ');
#endif
#ifndef STL_VECTOR
	strcat(buf, " ");
#else
        vbuf.push_back(' ');
#endif
	strstream valstr;
	valstr << *attr->Value() << '\0';
	const char* val = valstr.str();
	int vallen = val ? strlen(val) : 0;
	if (val)
#ifndef STL_VECTOR
	    strcat(buf, val);
#else
	{
	    const char* valp = val;
	    while (*valp)  { vbuf.push_back(*valp++); }
	}
#endif
	for (n = 15; n > vallen; n--)
#ifndef STL_VECTOR
	    strcat(buf, " ");
	strcat(buf, "\n");
#else
            vbuf.push_back(' ');
	vbuf.push_back('\n');
#endif

    }
#ifndef STL_VECTOR
    _ete->text(buf, update);
#else
    _ete->text(&vbuf[0] ? &vbuf[0] : "", update);
#endif
}

void AttributeListEditor::build() {
    DialogKit& dk = *DialogKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    PolyGlyph* _mainglyph = lk.vbox();
//    _mainglyph->append(lk.hcenter(wk.label(_lab)));
    Glyph* glu = lk.vspace(5);
    PolyGlyph* _namebox = lk.vbox();
    PolyGlyph* _valbox = lk.vbox();
    InputHandler* ih = new InputHandler(nil, wk.style());
    Coord wid = wk.font()->width("MMMMMMMMMMMMMMM", 15);

    Action* addaction = new ActionCallback(AttributeListEditor)(
	this, &AttributeListEditor::add
    );
    Button* addbutton = wk.push_button("Add", addaction);
    Action* remaction = new ActionCallback(AttributeListEditor)(
	this, &AttributeListEditor::remove
    );
    Button* rembutton = wk.push_button("Remove", remaction);
    _mainglyph->append(
	lk.hcenter(
	    lk.hbox(
		lk.vcenter(addbutton),
		lk.hspace(10),
		lk.vcenter(rembutton)
	    )
	)
    );
    _mainglyph->append(lk.vspace(10));
    _mainglyph->append(
	lk.hcenter(
	    lk.hbox(
		lk.vcenter(lk.hfixed(_namefe, wid)),
		lk.hspace(10),
		lk.vcenter(lk.hfixed(_valfe, wid))
	    )
	)
    );
    _mainglyph->append(lk.vspace(15));
    update_text(false);
    _mainglyph->append(lk.hcenter(lk.hspace(300)));
    _mainglyph->append(lk.hcenter(_ete));
    ih->body(wk.outset_frame(lk.margin(_mainglyph, 10)));
    body(ih);
}

void AttributeListEditor::fe_clr(FieldEditor* fe) {
    fe->field("");
}
