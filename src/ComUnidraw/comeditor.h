/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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

#ifndef comeditor_h
#define comeditor_h

#include <OverlayUnidraw/oved.h>

class ComTerpIOHandler;
class ComTerp;
class ComTerpServ;

//: editor that integrates ComTerp into the drawing editor framework.
class ComEditor : public OverlayEditor {
public:
    ComEditor(OverlayComp*, OverlayKit* = OverlayKit::Instance());
    // constructor for using existing component.
    ComEditor(const char* file, OverlayKit* = OverlayKit::Instance());
    // constructor for building top-level component from a file.
    ComEditor(boolean initflag, OverlayKit* = OverlayKit::Instance());
    // constructor for use of derived classes.
    void Init(OverlayComp* = nil, const char* name = "ComEditor");
    virtual void InitCommands();
    // method for running Unidraw Command objects after OverlayEditor
    // is constructed.
    virtual void AddCommands(ComTerp*);
    // method for adding ComFunc objects to the ComTerp associated with
    // this ComEditor.

    ComTerpServ* GetComTerp() { return _terp;}
    // return pointer to associated ComTerp (always a ComTerpServ).
    void SetComTerp(ComTerpServ* terp) { _terp = terp;}
    // set pointer to associated ComTerp (always a ComTerpServ).

    virtual void ExecuteCmd(Command* cmd);
    // indirect command execution for distributed whiteboard mechanism.

    boolean whiteboard();
    // test for distributed whiteboard mode, which only exists when
    // used with a ComEditor.

protected:

    ComTerpServ* _terp;
    ComTerpIOHandler* _terp_iohandler;
    int _whiteboard; // -1 == unitialized, 0 = false, 1 = true
};

#endif
