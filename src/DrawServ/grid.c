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

GraphicId::GraphicId (unsigned int sessionid) 
{
  _comp = nil;
  if (sessionid != 0) {
    _id = DrawServ::unique_grid();
    _sid = sessionid&DrawServ::SessionIdMask;
    GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
    table->insert(_id|_sid, this);
  } else {
    _id = _sid = 0;
  }
}

GraphicId::~GraphicId () 
{
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  table->remove(_id&_sid);
}

void GraphicId::id(unsigned int id) {
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  if (_id!=0 && _sid!=0) 
    table->remove(_id&_sid);
  _id = id & DrawServ::GraphicIdMask;
  _sid = id & DrawServ::SessionIdMask;
  table->insert(id, this);
}

/*****************************************************************************/

GraphicIds::GraphicIds(unsigned int sessionid, GraphicId* subids, int nsubs) : GraphicId (sessionid)
{
  if (nsubs>0) {
    _sublist = new GraphicIdList();
     for(int i=0; i<nsubs; i++)
      _sublist->Append(&subids[i]);
  }
  Resource::ref(_sublist);
}

GraphicIds::GraphicIds(unsigned int sessionid, GraphicIdList* sublist) : GraphicId (sessionid)
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

void GraphicId::grcomp(GraphicComp* comp) {
  if (comp==_comp) return;
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();
  if (_comp) {
    void* ptr = nil;
    table->find_and_remove(ptr, comp);
  } 
  _comp = comp;
  table->insert((void*)comp, (void*)this);
}
