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
 * Implementation of GraphicIdList class.
 */

#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
/*****************************************************************************/

GraphicIdList::GraphicIdList() 
{
  _ulist = new UList;
  _count = 0;
}

GraphicIdList::~GraphicIdList () 
{
  if (_ulist) {
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
      delete GetGraphicId(i);
    }
    delete _ulist; 
  }
}


void GraphicIdList::add_grid(GraphicId* new_id) {
  Iterator i;
  First(i);
  InsertBefore(i, new_id);
}

GraphicId* GraphicIdList::Id (UList* r) {
    return (GraphicId*) (*r)();
}

UList* GraphicIdList::Elem (Iterator i) { return (UList*) i.GetValue(); }

void GraphicIdList::Append (GraphicId* v) {
    _ulist->Append(new UList(v));
    ++_count;
}

void GraphicIdList::Prepend (GraphicId* v) {
    _ulist->Prepend(new UList(v));
    ++_count;
}

void GraphicIdList::InsertAfter (Iterator i, GraphicId* v) {
    Elem(i)->Prepend(new UList(v));
    ++_count;
}

void GraphicIdList::InsertBefore (Iterator i, GraphicId* v) {
    Elem(i)->Append(new UList(v));
    ++_count;
}

void GraphicIdList::Remove (Iterator& i) {
    UList* doomed = Elem(i);

    Next(i);
    _ulist->Remove(doomed);
    delete doomed;
    --_count;
}	
    
void GraphicIdList::Remove (GraphicId* p) {
    UList* temp;

    if ((temp = _ulist->Find(p)) != nil) {
	_ulist->Remove(temp);
        delete temp;
	--_count;
    }
}

GraphicId* GraphicIdList::GetGraphicId (Iterator i) { return Id(Elem(i)); }

void GraphicIdList::SetGraphicId (GraphicId* gv, Iterator& i) {
    i.SetValue(_ulist->Find(gv));
}

void GraphicIdList::First (Iterator& i) { i.SetValue(_ulist->First()); }
void GraphicIdList::Last (Iterator& i) { i.SetValue(_ulist->Last()); }
void GraphicIdList::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void GraphicIdList::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean GraphicIdList::Done (Iterator i) { return Elem(i) == _ulist->End(); }
int GraphicIdList::Number () { return _count; }

boolean GraphicIdList::Includes (GraphicId* e) {
    return _ulist->Find(e) != nil;
}

boolean GraphicIdList::IsEmpty () { return _ulist->IsEmpty(); }
