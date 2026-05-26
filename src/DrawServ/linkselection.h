/*
 * Copyright (c) 2004 Scott E. Johnston
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
 * LinkSelection - manages the selection of graphics between remotely linked drawing editors.
 */

#ifndef link_selection_h
#define link_selection_h

#ifdef HAVE_ACE
#include <OverlayUnidraw/ovselection.h>

class DrawEditor;
class DrawLinkList;
class GraphicIdList;
class OverlayComp;

//: distributed Selection class
// specialized Selection class with support for coordinating the selection of graphics
// between two remote drawing editors that are linked together.
class LinkSelection: public OverlaySelection {
public:
  LinkSelection(DrawEditor* ed, LinkSelection* = nil);
  LinkSelection(DrawEditor* ed, Selection*);
  
  void Init(DrawEditor*);
  
  virtual void Update(Viewer* = nil); 
  virtual void Clear(Viewer* = nil);
  
  virtual void Reserve();
  // reserve newly created graphics in selection across the network
  
  virtual void CopyFlags(OverlaySelection* from);

  void AddComp(OverlayComp*);
  // add this graphic to the Selection.
  
  enum { NotSelected, LocallySelected, RemotelySelected, WaitingToBeSelected };

  static const char* selected_string(int state) 
    { return state>=0 && state<=WaitingToBeSelected ?  _selected_strings[state] : nil; }

  int& waiting_count() {return _waiting_count; }
  // return waiting_count by reference

  int& granted_count() {return _granted_count; }
  // return granted_count by reference

  boolean& wtbs_flag() {return _wtbs_flag; }
  // return true if a new graphic just got selected.

  boolean& remote_flag() {return _remote_flag; }
  // return true if a remotely selected graphic is detected

  boolean& paste_in_progress_flag() {return _paste_in_progress_flag; }
  // return true if a programmatic paste is in progress.

  void Beep(const char* debugstr = NULL);
  // make alert sound

  void Ding(const char* debugstr = NULL);
  // make confirmation sound

  int all_requests_resolved(boolean granted);
  // check if all requests have been denied.
  // return values:
  //  0 = still waiting (more requests in flight)
  //  1 = all resolved, at least one granted (ding)
  // -1 = all resolved, all denied (beep)
  
  boolean request_resolved_check(boolean granted, const char* fileline);
  // make beep/ding sound when request resolved and response is known

  virtual void unlock_key(const char* keystr);
  // unlock graphics owned by keystr for local modification without grid messages
  virtual void lock_key(const char* keystr);
  // re-lock graphics previously unlocked by keystr

protected:
  DrawEditor* _editor;
  static GraphicIdList* _locally_selected;
  static GraphicIdList* _waiting_to_be_selected;

  static const char* _selected_strings[];

  int _waiting_count;
  int _granted_count;
  boolean _wtbs_flag;
  boolean _remote_flag;
  boolean _paste_in_progress_flag;

};
#endif /* HAVE_ACE */
#endif
