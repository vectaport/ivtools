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
 * GraphicId - object to encapsulate 2-way link with remote drawserv
 */
#ifndef grid_h
#define grid_h

#include <Unidraw/globals.h>

class DrawLink;
class GraphicComp;
class GraphicIdList;

//: object to encapsulate unique graphic id
class GraphicId {
public:
  GraphicId(unsigned int sessionid=0);
  virtual ~GraphicId();
  
  unsigned int id() { return _id|_sid; }
  // get associated unique composite integer id

  void id(unsigned int id);
  // set associated unique composite integer id

  unsigned int grid() { return _id; }
  // return graphic id portion of composite id
  // unique only to this process

  unsigned int sessionid() { return _sid; }
  // return session id portion of composite id

  virtual int is_list() { return 0; }
  // return true if can be cast to GraphicIds

  virtual GraphicIdList* sublist() { return nil; }
  // returns pointer to sublist of GraphicId's, if any

  void grcomp(GraphicComp* comp);
  // set pointer to associated GraphicComp

  GraphicComp* grcomp() { return _comp; }
  // get pointer to associated GraphicComp

  void selector(unsigned int sid) { _selector = sid; }
  // set session id of current selector

  unsigned int selector() { return _selector; }
  // get session id of current selector

  void selected(int state) { _selected = state; }
  // set selected state

  int selected() { return _selected; }
  // get selected state

protected:
  unsigned int _id;
  unsigned int _sid;
  unsigned int _selector;
  int _selected;
  GraphicComp* _comp;

};

//: object to encapsulate a set of graphic ids
class GraphicIds : public GraphicId {
public:
  GraphicIds(unsigned int sessionid, GraphicId* subids, int nsubids); 
  GraphicIds(unsigned int sessionid, GraphicIdList* sublist=nil);
  virtual ~GraphicIds();

  virtual int is_list() { return 1; }
  // return true if can be cast to GraphicIds
  virtual GraphicIdList* sublist() { return _sublist; }
  // returns pointer to sublist of GraphicId's, if any

protected:
  GraphicIdList* _sublist;
};

#endif
