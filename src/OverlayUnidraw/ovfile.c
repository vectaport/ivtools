/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/Graphic/picture.h>

#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

#include <Attribute/attrlist.h>

#include <stdio.h>
#include <stream.h>
#include <string.h>

/*****************************************************************************/

ParamList* OverlayFileComp::_overlay_file_params = nil;

OverlayFileComp::OverlayFileComp(OverlayComp* parent) : OverlaysComp(parent) {
    _pathname = nil;
}

OverlayFileComp::OverlayFileComp(Graphic* g, OverlayComp* parent) : OverlaysComp(g, parent) {
    _pathname = nil;
}

OverlayFileComp::OverlayFileComp (istream& in, OverlayComp* parent) : OverlaysComp(parent) {
    _pathname = nil;
    _valid = GetParamList()->read_args(in, this);
}

OverlayFileComp::~OverlayFileComp() {
    delete _pathname;
}

ParamList* OverlayFileComp::GetParamList() {
    if (!_overlay_file_params) 
	GrowParamList(_overlay_file_params = new ParamList());
    return _overlay_file_params;
}

void OverlayFileComp::GrowParamList(ParamList* pl) {
    pl->add_param("pathname", ParamStruct::required, &OverlayFileScript::ReadPathName,
		  this, this);
    OverlaysComp::GrowParamList(pl); 
}

Component* OverlayFileComp::Copy () {
    OverlayFileComp* ovfile = new OverlayFileComp(new Picture(GetGraphic()));
    if (attrlist()) ovfile->SetAttributeList(new AttributeList(attrlist()));
    ovfile->SetPathName(GetPathName());
    Iterator i;
    First(i);
    if (!Done(i)) ovfile->Append((GraphicComp*)GetComp(i)->Copy());
    return ovfile;
}

ClassId OverlayFileComp::GetClassId() { return OVFILE_COMP; }

boolean OverlayFileComp::IsA(ClassId id) {
    return id == OVFILE_COMP || OverlaysComp::IsA(id);
}

void OverlayFileComp::Interpret (Command* cmd) {
    if (cmd->IsA(UNGROUP_CMD)) {
	/* don\'t allow it */
    } else 
	OverlaysComp::Interpret(cmd);
}

void OverlayFileComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(UNGROUP_CMD)) {
	/* don\'t allow it */
    } else 
	OverlaysComp::Uninterpret(cmd);
}

void OverlayFileComp::SetPathName(const char* pathname) {
    _pathname = strdup(pathname);
    if (GetIdrawComp()) GetIdrawComp()->SetPathName(pathname);
}

const char* OverlayFileComp::GetPathName() { 
    return _pathname;
}

OverlayIdrawComp* OverlayFileComp::GetIdrawComp() {
    Iterator i;
    First(i);
    return ((OverlayIdrawComp*) GetComp(i));
}

boolean OverlayFileComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    return
	strcmp(GetPathName(), ((OverlayFileComp&)comp).GetPathName()) &&
	OverlayComp::operator==(comp);
}

void OverlayFileComp::Append(GraphicComp* comp) {
  Iterator i;
  First(i);
  while (!Done(i)) {
    GraphicComp* ocomp = GetComp(i);
    Remove(i); 
    delete ocomp;
  }
  OverlaysComp::Append(comp);
  SetAttributeList(((OverlayComp*)comp)->GetAttributeList());
}

/*****************************************************************************/

OverlayFileView::OverlayFileView() : OverlaysView() {}

ClassId OverlayFileView::GetClassId() { return OVFILE_VIEW; }

boolean OverlayFileView::IsA(ClassId id) {
    return id == OVFILE_VIEW || OverlaysView::IsA(id);
}

/*****************************************************************************/

OverlayFileScript::OverlayFileScript (OverlayFileComp* subj) : OverlaysScript(subj) {}

ClassId OverlayFileScript::GetClassId () { return OVFILE_SCRIPT; }

boolean OverlayFileScript::IsA (ClassId id) { 
    return OVFILE_SCRIPT == id || OverlaysScript::IsA(id);
}

boolean OverlayFileScript::Definition (ostream& out) {
    OverlayFileComp* comp = (OverlayFileComp*) GetSubject();

    out << "drawtool(\"" << comp->GetPathName() << "\"";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return true;
}    

int OverlayFileScript::ReadPathName (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    OverlayFileComp* filecomp = (OverlayFileComp*)addr1;

    char pathname[BUFSIZ];
    if (ParamList::parse_pathname(in, pathname, BUFSIZ, filecomp->GetBaseDir()) != 0)
	return -1;

    /* check pathname for recursion */
    OverlayComp* parent = (OverlayComp*) filecomp->GetParent();
    while (parent != nil) {
	if (parent->GetPathName() && strcmp(parent->GetPathName(), pathname) == 0) {
	    cerr << "pathname recursion not allowed (" << pathname << ")\n";
	    return -1;
	}
	parent = (OverlayComp*) parent->GetParent();
    }

    filecomp->SetPathName(pathname);
    OverlayIdrawComp* child = nil;
    OverlayCatalog* catalog = (OverlayCatalog*) unidraw->GetCatalog();
    catalog->SetParent(filecomp);
    if( catalog->OverlayCatalog::Retrieve(pathname, (Component*&)child)) {
	catalog->SetParent(nil);
	catalog->Forget(child);
	filecomp->Append(child);
	return 0;
    } else {
	catalog->SetParent(nil);
	return -1;
    }
}

boolean OverlayFileScript::EmitGS(ostream& out, Clipboard* cb, boolean prevout) {
    return OverlayScript::EmitGS(out, cb, prevout);
}

