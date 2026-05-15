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
#include <uuid/uuid.h>
#include <cstdint>

class DrawLink;
class GraphicComp;
class GraphicIdList;

//: object to encapsulate unique graphic id
class GraphicId {
public:
  GraphicId(uuid_t sessionid=NULL);
  virtual ~GraphicId();
  
  const uuid_t& id() { return _id; }
  // get associated unique id

  void id(uuid_t id);
  // set associated unique id

  const char* idstr() { return _id_str; }
  // get associated unique id in string form

  uuid_t& grid() { return _id; }
  // return graphic id portion of composite id
  // unique only to this process

  const uuid_t& sessionid() {return _sid;};
  // get associated universally unique session

  void sessionid(uuid_t id);
  // set associated universally unique session id

  const char* sessionidstr() { return _sid_str; }
  // get associated universally unique session id in string form

  virtual int is_list() { return 0; }
  // return true if can be cast to GraphicIds

  virtual GraphicIdList* sublist() { return nil; }
  // returns pointer to sublist of GraphicId's, if any

  void grcomp(GraphicComp* comp);
  // set pointer to associated GraphicComp

  GraphicComp* grcomp() { return _comp; }
  // get pointer to associated GraphicComp

  void selector(uuid_t sid);
  // set session id of current selector

  uuid_t& selector() { return _selector; }
  // get session id of current selector

  uint32_t selectorkey() { uint32_t key; memcpy(&key, _selector, sizeof(key)); return key; }
  // get session id of current selector

  const char* selectorstr() { return _selector_str; }
  // get session id of current selector

  void selected(int state) { _selected = state; }
  // set selected state

  int selected() { return _selected; }
  // get selected state

protected:
  uuid_t _id;
  uuid_t _sid;
  uuid_string_t _id_str;
  uuid_string_t _sid_str;
  
  uuid_t _selector;
  uuid_string_t _selector_str;
  int _selected;
  
  GraphicComp* _comp;

};

//: object to encapsulate a set of graphic ids
class GraphicIds : public GraphicId {
public:
  GraphicIds(uuid_t sessionid, GraphicId* subids, int nsubids); 
  GraphicIds(uuid_t sessionid, GraphicIdList* sublist=nil);
  virtual ~GraphicIds();

  virtual int is_list() { return 1; }
  // return true if can be cast to GraphicIds
  virtual GraphicIdList* sublist() { return _sublist; }
  // returns pointer to sublist of GraphicId's, if any

protected:
  GraphicIdList* _sublist;
};

#endif
