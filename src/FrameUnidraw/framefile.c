/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

#include <FrameUnidraw/framecatalog.h>
#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framefile.h>

#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/Graphic/picture.h>

#include <Unidraw/catalog.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

#include <stdio.h>
#include <stream.h>
#include <string.h>

/*****************************************************************************/

ParamList* FrameFileComp::_frame_file_params = nil;

FrameFileComp::FrameFileComp(OverlayComp* parent) : FramesComp(parent) {
    _pathname = nil;
}

FrameFileComp::FrameFileComp(Graphic* g, OverlayComp* parent) : FramesComp(g, parent) {
    _pathname = nil;
}

FrameFileComp::FrameFileComp (istream& in, OverlayComp* parent) : FramesComp(parent) {
    _pathname = nil;
    _valid = GetParamList()->read_args(in, this);
}

FrameFileComp::~FrameFileComp() {
    delete _pathname;
}

ParamList* FrameFileComp::GetParamList() {
    if (!_frame_file_params) 
	GrowParamList(_frame_file_params = new ParamList());
    return _frame_file_params;
}

void FrameFileComp::GrowParamList(ParamList* pl) {
    pl->add_param("pathname", ParamStruct::required, &FrameFileScript::ReadPathName,
		  this, this);
    FramesComp::GrowParamList(pl); 
}

Component* FrameFileComp::Copy () {
    FrameFileComp* ovfile = new FrameFileComp(new Picture(GetGraphic()));
    Iterator i;
    First(i);
    if (!Done(i)) ovfile->Append((GraphicComp*)GetComp(i)->Copy());
    return ovfile;
}

ClassId FrameFileComp::GetClassId() { return FRAME_FILE_COMP; }

boolean FrameFileComp::IsA(ClassId id) {
    return id == FRAME_FILE_COMP || FramesComp::IsA(id);
}

void FrameFileComp::Interpret (Command* cmd) {
    if (cmd->IsA(UNGROUP_CMD)) {
    } else 
	FramesComp::Interpret(cmd);
}

void FrameFileComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(UNGROUP_CMD)) {
    } else 
	FramesComp::Uninterpret(cmd);
}

void FrameFileComp::SetPathName(const char* pathname) {
    _pathname = strdup(pathname);
    if (GetIdrawComp()) GetIdrawComp()->SetPathName(pathname);
}

const char* FrameFileComp::GetPathName() { 
    return _pathname;
}

FrameIdrawComp* FrameFileComp::GetIdrawComp() {
    Iterator i;
    First(i);
    return ((FrameIdrawComp*) GetComp(i));
}

/*****************************************************************************/

FrameFileView::FrameFileView() : FramesView() {}

ClassId FrameFileView::GetClassId() { return FRAME_FILE_VIEW; }

boolean FrameFileView::IsA(ClassId id) {
    return id == FRAME_FILE_VIEW || FramesView::IsA(id);
}

/*****************************************************************************/

FrameFileScript::FrameFileScript (FrameFileComp* subj) : FramesScript(subj) {}

ClassId FrameFileScript::GetClassId () { return FRAME_FILE_SCRIPT; }

boolean FrameFileScript::IsA (ClassId id) { 
    return FRAME_FILE_SCRIPT == id || FramesScript::IsA(id);
}

boolean FrameFileScript::Definition (ostream& out) {
    FrameFileComp* comp = (FrameFileComp*) GetSubject();

    out << "framefile(\"" << comp->GetPathName() << "\"";
    FullGS(out);
    Annotation(out);
    out << ")";

    return true;
}    

int FrameFileScript::ReadPathName (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    FrameFileComp* filecomp = (FrameFileComp*)addr1;

    char pathname[BUFSIZ];
    if (ParamList::parse_pathname(in, pathname, BUFSIZ, filecomp->GetBaseDir()) != 0)
	return -1;

    /* check pathname for recursion */
    OverlayComp* parent = (OverlayComp*) filecomp->GetParent();
    while (parent != nil) {
	if (parent->GetPathName() && strcmp(parent->GetPathName(), pathname) == 0) {
	    cerr << "framefile recursion not allowed (" << pathname << ")\n";
	    return -1;
	}
	parent = (OverlayComp*) parent->GetParent();
    }

    filecomp->SetPathName(pathname);
    FrameIdrawComp* child = nil;
    FrameCatalog* catalog = (FrameCatalog*)unidraw->GetCatalog();
    catalog->SetParent(filecomp);
    if( catalog->FrameCatalog::Retrieve(pathname, (Component*&)child)) {
	catalog->SetParent(nil);
	catalog->Forget(child);
	filecomp->Append(child);
	return 0;
    } else {
	catalog->SetParent(nil);
	return -1;
    }
}

boolean FrameFileScript::EmitGS(ostream& out, Clipboard* cb, boolean prevout) {
    return OverlayScript::EmitGS(out, cb, prevout);
}

boolean FrameFileScript::EmitPts(ostream& out, Clipboard* cb, boolean prevout) {
    return OverlayScript::EmitPts(out, cb, prevout);
}

