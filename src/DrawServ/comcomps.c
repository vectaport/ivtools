/*
 * Copyright (c) 1994-1996 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * ComIdrawComp implementation.
 */

#include <DrawServ/comcomps.h>
#include <DrawServ/comclasses.h>
#include <OverlayUnidraw/paramlist.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <iostream.h>

/*****************************************************************************/

ComIdrawComp::ComIdrawComp (const char* pathname, OverlayComp* parent)
: OverlayIdrawComp(pathname, parent) {
}

ComIdrawComp::ComIdrawComp (istream& in, const char* pathname, OverlayComp* parent) : 
OverlayIdrawComp(in, pathname, parent) {
}

ComIdrawComp::~ComIdrawComp () {
}

ClassId ComIdrawComp::GetClassId () { return COM_IDRAW_COMP; }

Component* ComIdrawComp::Copy () {
    ComIdrawComp* comps = new ComIdrawComp(GetPathName());
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

boolean ComIdrawComp::IsA (ClassId id) {
    return COM_IDRAW_COMP == id || OverlayIdrawComp::IsA(id);
}


/*****************************************************************************/

ComIdrawScript::ComIdrawScript (ComIdrawComp* subj) : OverlayIdrawScript(subj) {
}

ComIdrawScript::~ComIdrawScript() {

}

ClassId ComIdrawScript::GetClassId () { return COM_IDRAW_SCRIPT; }

boolean ComIdrawScript::IsA (ClassId id) { 
    return COM_IDRAW_SCRIPT == id || OverlayIdrawScript::IsA(id);
}

boolean ComIdrawScript::Emit (ostream& out) {
    out << "drawserv(";

    /* make list and output unique point lists */
    boolean prevout = false;
    if (_pts_compacted) {
	_ptslist = new Clipboard();
	prevout = EmitPts(out, _ptslist, prevout);
    }

    /* make list and output unique graphic states */
    if (_gs_compacted) {
	_gslist = new Clipboard();
	prevout = EmitGS(out, _gslist, prevout);
    }

    /* make list and output unique picture graphics */
    if (_pic_compacted) {
	_piclist1 = new Clipboard();
	_piclist2 = new Clipboard();
	prevout = EmitPic(out, _piclist1, _piclist2, prevout);
    }

    /* output graphic components */
    boolean status = true;
    Iterator i;
    First(i);
    if (!Done(i) ) {
	if (prevout) out << ",";
	out << "\n";
    }
    for (; status && !Done(i); ) {
	ExternView* ev = GetView(i);
	Indent(out);
        status = ev->Definition(out);
	Next(i);
	if (!Done(i)) out << ",\n";
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")\n";
    return status;
}


