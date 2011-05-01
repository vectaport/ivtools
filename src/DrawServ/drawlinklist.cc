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
 * Implementation of DrawLinkList class.
 */

#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/sid.h>

#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
/*****************************************************************************/

DrawLinkList::DrawLinkList() 
{
  _ulist = new UList;
  _count = 0;
}

DrawLinkList::~DrawLinkList () 
{
  if (_ulist) {
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
      delete GetDrawLink(i);
    }
    delete _ulist; 
  }
}


void DrawLinkList::add_drawlink(DrawLink* new_link) {
  Iterator i;
  First(i);
  InsertBefore(i, new_link);
}

DrawLink* DrawLinkList::find_drawlink(GraphicId* grid) {
  void* ptr = nil;
  ((DrawServ*)unidraw)->sessionidtable()->find(ptr, grid->selector());
  SessionId* sessionid = (SessionId*)ptr;
  return (DrawLink*) (sessionid ? sessionid->drawlink() : nil);
}

DrawLink* DrawLinkList::Link (UList* r) {
    return (DrawLink*) (*r)();
}

UList* DrawLinkList::Elem (Iterator i) { return (UList*) i.GetValue(); }

void DrawLinkList::Append (DrawLink* v) {
    _ulist->Append(new UList(v));
    ++_count;
    v->attach(this);
    notify();
}

void DrawLinkList::Prepend (DrawLink* v) {
    _ulist->Prepend(new UList(v));
    ++_count;
    v->attach(this);
    notify();
}

void DrawLinkList::InsertAfter (Iterator i, DrawLink* v) {
    Elem(i)->Prepend(new UList(v));
    ++_count;
    v->attach(this);
    notify();
}

void DrawLinkList::InsertBefore (Iterator i, DrawLink* v) {
    Elem(i)->Append(new UList(v));
    ++_count;
    v->attach(this);
    notify();
}

void DrawLinkList::Remove (Iterator& i) {
    GetDrawLink(i)->detach(this);

    UList* doomed = Elem(i);

    Next(i);
    _ulist->Remove(doomed);
    delete doomed;
    --_count;
    notify();
}	
    
void DrawLinkList::Remove (DrawLink* p) {
    p->detach(this);

    UList* temp;

    if ((temp = _ulist->Find(p)) != nil) {
	_ulist->Remove(temp);
        delete temp;
	--_count;
    }
    notify();
}

DrawLink* DrawLinkList::GetDrawLink (Iterator i) { return Link(Elem(i)); }

void DrawLinkList::SetDrawLink (DrawLink* gv, Iterator& i) {
    i.SetValue(_ulist->Find(gv));
}

void DrawLinkList::First (Iterator& i) { i.SetValue(_ulist->First()); }
void DrawLinkList::Last (Iterator& i) { i.SetValue(_ulist->Last()); }
void DrawLinkList::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void DrawLinkList::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean DrawLinkList::Done (Iterator i) { return Elem(i) == _ulist->End(); }
int DrawLinkList::Number () { return _count; }

boolean DrawLinkList::Includes (DrawLink* e) {
    return _ulist->Find(e) != nil;
}

boolean DrawLinkList::IsEmpty () { return _ulist->IsEmpty(); }
