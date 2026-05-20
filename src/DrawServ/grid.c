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
 * Implementation of GraphicId class.
 */

#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>

#include <Unidraw/iterator.h>

#include <stdlib.h>
#include <time.h>

/*****************************************************************************/

GraphicId::GraphicId (uuid_t sid) 
{
#ifdef HAVE_ACE
  _comp = nil;
  uuid_clear(_selector);
  memset(_selector_str, 0, sizeof(_selector_str));
  _selected = 0;
  uuid_clear(_id);
  memset(_id_str, 0, sizeof(_id_str));
  
  if (sid!=NULL && !uuid_is_null(sid)) {
    selector(sid);
  }
  _beep_on_deny = false;

#endif
}

GraphicId::~GraphicId () 
{
#ifdef HAVE_ACE
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  table->remove(uuid_key(_id));
#endif
}

void GraphicId::set_id(uuid_t id) {
#ifdef HAVE_ACE
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();

  if (!uuid_is_null(id)) {
    
    void *ptr = NULL;
    table->find(ptr, uuid_key(id));
    if (ptr != NULL) {
      fprintf(stderr, "UNEXPECTED COLLISION ON UUID8, NEED TO REWRITE TABLES FOR FULL UUID\n");
      abort();
    }
    // table->remove(uuid_key(id));
    
  }

  uuid_copy(_id, id);
  uuid_unparse(_id, _id_str);
  table->insert(uuid_key(id), this);
#endif
}

uuid_t& GraphicId::generate_id() {
#ifdef HAVE_ACE
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();

  uuid_generate(_id);
  
  void *ptr = NULL;
  table->find(ptr, uuid_key(_id));
  if (ptr != NULL) {
    // table->remove(uuid_key(id));
    fprintf(stderr, "UNEXPECTED COLLISION ON UUID8, NEED TO REWRITE TABLES FOR FULL UUID\n");
    abort();
  }
  uuid_unparse(_id, _id_str);
  table->insert(uuid_key(_id), this);
#endif
  return _id;
}

void GraphicId::selector(uuid_t selector) {
  uuid_copy(_selector, selector);
  uuid_unparse(_selector, _selector_str);
}

uint32_t GraphicId::selectorkey() {
  return uuid_key(_selector);
}



/*****************************************************************************/

GraphicIds::GraphicIds(uuid_t sessionid, GraphicId* subids, int nsubs) : GraphicId (sessionid)
{
  if (nsubs>0) {
    _sublist = new GraphicIdList();
     for(int i=0; i<nsubs; i++)
      _sublist->Append(&subids[i]);
  }
  Resource::ref(_sublist);
}

GraphicIds::GraphicIds(uuid_t sessionid, GraphicIdList* sublist) : GraphicId (sessionid)
{
  if (!sublist) sublist = new GraphicIdList;
  _sublist = sublist;
  _sublist->Reference();
}

GraphicIds::~GraphicIds () 
{
  Unref(_sublist);
  _sublist = nil;
}

void GraphicId::grcomp(OverlayComp* comp) {
#ifdef HAVE_ACE
  if (comp==_comp) return;
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();
  if (_comp) {
    void* ptr = nil;
    table->find_and_remove(ptr, comp);
  } 
  _comp = comp;
  table->insert((void*)comp, (void*)this);
#endif
}
