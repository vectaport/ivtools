/*
 * Copyright (c) 1994,1999 Vectaport Inc.
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
 * OverlayUnidraw - Unidraw derived for OverlayUnidraw library
 */
#ifndef ovunidraw_h
#define ovunidraw_h

#include <Unidraw/unidraw.h>

class AttributeList;
class Command;
class ComTerpServ;
class Event;
class MacroCmd;
class OverlayViewer;

//: derived Unidraw object with extra mechanisms.
// derived Unidraw object with extra mechanisms for logging and deferred
// execution of commands.
class OverlayUnidraw : public Unidraw {
public:
    OverlayUnidraw(
        Catalog*, int& argc, char** argv, 
        OptionDesc* = nil, PropertyData* = nil
    );
    OverlayUnidraw(Catalog*, World*);
    virtual ~OverlayUnidraw();

    virtual void Update(boolean immediate = false);
    virtual void Run();
    virtual void Log(Command*, boolean dirty);

    void Append(Command*);

    virtual boolean PrintAttributeList(ostream& out, AttributeList* list) 
      { return false; }
    // alternate method for serializing an AttributeList
    // returns false if really not there.

    static boolean unidraw_updated();
    static boolean npause_lessened();
    static boolean unidraw_updated_or_command_pushed();
    static boolean unidraw_updated_or_command_pushed_or_npause_lessened();
    static void pointer_tracker_func(Event&);

    void CurrentViewer(OverlayViewer* viewer) { _ovviewer = viewer; }
    OverlayViewer* CurrentViewer() { return _ovviewer; }

    ComTerpServ* comterp() { return _comterp; }
    void comterp(ComTerpServ* comterp) { _comterp = comterp; }

    void DeferredNotify();
    // do all deferred notifications

    static boolean deferred_notifications() { return _deferred_notifications; }
    // return flag that indicates deferred notifications

    static void deferred_notifications(boolean flag) { _deferred_notifications = flag; }
    // set flag that indicates deferred notifications
    
protected:
    static MacroCmd* _cmdq;
    static boolean* _updated_ptr;
    OverlayViewer* _ovviewer;
    static ComTerpServ* _comterp;
    static int _npause;
    static boolean _deferred_notifications;
};

#endif
