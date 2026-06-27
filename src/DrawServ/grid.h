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
#if !defined(__APPLE__) && !defined(IV_UUID_STRING_T_DEFINED)
#define IV_UUID_STRING_T_DEFINED
typedef char uuid_string_t[37];  /* Apple-only type; Linux libuuid lacks it */
#endif
#include <cstdint>

class DrawLink;
class OverlayComp;
class GraphicIdList;

//: object to encapsulate unique graphic id
class GraphicId {
public:
  GraphicId(uuid_t sessionid=NULL);
  virtual ~GraphicId();
  
  const uuid_t& id() { return _id; }
  // get associated unique id

  uuid_t& generate_id();
  // generate unique id and assign it

  void set_id(uuid_t id);
  // sete unique id

  const char* idstr() { return _id_str; }
  // get associated unique id in string form

  uuid_t& grid() { return _id; }
  // return graphic id portion of composite id
  // unique only to this process

  virtual int is_list() { return 0; }
  // return true if can be cast to GraphicIds

  virtual GraphicIdList* sublist() { return nil; }
  // returns pointer to sublist of GraphicId's, if any

  void grcomp(OverlayComp* comp);
  // set pointer to associated OverlayComp

  OverlayComp* grcomp() { return _comp; }
  // get pointer to associated OverlayComp

  void selector(uuid_t sid);
  // set session id of current selector

  uuid_t& selector() { return _selector; }
  // get session id of current selector

  uint32_t selectorkey();
  // get key of session id of current selector

  const char* selectorstr() { return _selector_str; }
  // get session id of current selector

  void selected(int state) { _selected = state; }
  // set selected state

  int selected() { return _selected; }
  // get selected state

  const char* compclass();
  // return name of comp's class

  void unlocked(boolean flag) { _unlocked = flag; }
  // set flag indicating remote lock temporarily suspended for local modification
  boolean unlocked() { return _unlocked; }
  // return true if remote lock temporarily suspended for local modification

protected:
  uuid_t _id;
  uuid_string_t _id_str;
  
  uuid_t _selector;
  uuid_string_t _selector_str;
  
  int _selected;
  
  OverlayComp* _comp;
  boolean _unlocked;
  // true when remote lock temporarily suspended for local modification

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
