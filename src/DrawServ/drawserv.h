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
 * DrawServ - Unidraw derived from OverlayUnidraw for DrawServ library
 */
#ifndef drawserv_h
#define drawserv_h

#include <OverlayUnidraw/ovunidraw.h>
#include <stdio.h>
#include <strstream>
#include <uuid/uuid.h>

// utility function for grabbing key from uuid_t.
extern uint32_t uuid_key(const uuid_t u);

#include <OS/table.h>
declareTable(GraphicIdTable,uint32_t,void*)
declareTable(SessionIdTable,uint32_t,void*)
declareTable(CompIdTable,void*,void*);
     

//: Unidraw specialized for DrawServ
// Unidraw (OverlayUnidraw) specialized for DrawServ application.
// Networked application of the Unidraw framework.

class AttributeValueList;
class ComTerp;
class DrawEditor;
class DrawLink;
class DrawLinkList;
class GraphicId;
class GraphicIdList;
class OverlayComp;

#if !defined (HOST_NAME_MAX)
#  define HOST_NAME_MAX 256
#endif /* !HOST_NAME_MAX */

class DrawServ : public OverlayUnidraw {
public:
  DrawServ(
	   Catalog*, int& argc, char** argv, 
	   OptionDesc* = nil, PropertyData* = nil
	   );
  DrawServ(Catalog*, World*);
  virtual ~DrawServ();
  
  void Init();
  
#ifdef HAVE_ACE
  DrawLink* linkup(const char* hostname, int portnum, 
		   int state, uuid_t link_id=NULL, ComTerp* comterp=nil);
  // Create new link to remote drawserv, return -1 if error
  // state: 0==new_link, 1==one_way, 2==two_way.
  // Let DrawLink assign local_id by passing -1 for local_id.
  // The local_id argument is for verification purposes once
  // two-way link is established.
  
  int linkdown(DrawLink* link);
  // shut down existing link to remote drawserv
  
  DrawLink* linkget(const char* hostname, int portnum);
  // return pointer to existing DrawLink
  
  DrawLink* linkget(uuid_t sessionid);
  // return pointer to existing DrawLink given a sessionid.
  
  void linkdump(FILE*);
  // dump text table of DrawLink's
  
  virtual void ExecuteCmd(Command*);
  // execute Command's locally, and on remote linked DrawServ's.
  
  virtual void DistributeCmdString(const char* cmdstring, DrawLink* orglink=nil);
  // send command string to every remote DrawServ (except where it came from).
  
  virtual void SendCmdString(DrawLink* link, const char* cmdstring);
  // execute command string on one remote DrawServ

  virtual void SendAllToBackgroundEditor(DrawLink* link, DrawEditor* fged);
  // send copies of everything to remote DrawServ to paste on front

  virtual void SendAllToForegroundEditor(DrawLink* link, DrawEditor* bged);
  // send copies of everything to remote DrawServ to paste in back

  DrawLinkList* linklist() { return _linklist; }
  // return pointer to list of DrawLink's
  
  GraphicIdTable* gridtable() { return _gridtable; }
  // return pointer to table of GraphicId's.
  
  SessionIdTable* sessionidtable() { return _sessionidtable; }
  // return pointer to table of session id's that map to SessionId's
  
  CompIdTable* compidtable() { return _compidtable; }
  // return pointer to table that map from GraphicComp* to GraphicId*
  
  void sessionid_register(DrawLink* link);
  // register all sessionid's used by this DrawServ with remote DrawServ
  
  void sessionid_register_handle(DrawLink* link, uuid_t sid,
				 int pid, const char* user, 
				 const char* host, int hostid);
  // handle request to register unique session id
  
  void sessionid_register_propagate(DrawLink* link, uuid_t sid, int pid, 
				    const char* user, const char *host, int hostid);
  // propagate a newly registered session id to all other DrawLink's
  
  uuid_t& sessionid() { return _sessionid; }
  // get universally unique session id.

  const char* sessionidstr() { return (const char*) _sessionid_str; }
  // get universally unique session id.

  uint32_t sessionidkey() { return uuid_key(_sessionid); }
  // get universally unique session id.

  void remove_sids(DrawLink*);
  // remove all SessionId's associated with this DrawLink

  void grid_message(GraphicId* grid);
  // generate graphic id selection message

  void grid_message_handle(DrawLink* link, uuid_t id, uuid_t selector, 
			   int state, uuid_t newselector=NULL);
  // handle graphic id selection message

  void grid_message_callback(DrawLink* link, uuid_t id, uuid_t selector, 
			     int state, uuid_t oldselector);
  // callback for graphic id selection message 

  void unique_grid(uuid_t grid);
  // generate unique graphic id.
  
  static int test_grid(uuid_t gid);
  // test candidate graphic id for local uniqueness
  static int test_sessionid(uuid_t sid);
  // test candidate graphic id for local uniqueness
  
  void  create_unique_sessionid();
  // generate and assign unique session id.
  
  static unsigned int GraphicIdMask;
  static unsigned int SessionIdMask;

  void print_gridtable();
  // print contents of table of GraphicId's
  
  void print_sidtable();
  // print contents of table of SessionId's
 
  int comdraw_port() { return _comdraw_port; }
  // return port used for comdraw command interpreter

  boolean cycletest(uuid_t sid, const char* host, const char* user, int pid);
  // test for new incoming link that would establish a cycle

  boolean selftest(const char* host, unsigned int portnum);
  // test if a new outgoing link is really to yourself

  virtual boolean PrintAttributeList(ostream& out, AttributeList* list);
  // alternate method for serializing an AttributeList
  // returns false if really not there.

protected:
  boolean add_grid(OverlayComp* comp, uuid_t grid, uuid_t sid);
  
  DrawLinkList* _linklist;
  // DrawLink list
  GraphicIdTable* _gridtable;
  // table of all GraphicId's.
  // maps from id to GraphicId*
  SessionIdTable* _sessionidtable;
  // table of all session id's.
  // maps from session id to SessionId*
  CompIdTable* _compidtable;
  // table of all GraphicComp's associated with a GraphicId.
  // maps from GraphicComp* to GraphicId*
  
  uuid_t _sessionid;
  // universally unique session id.
  
  uuid_string_t _sessionid_str;
  // universally unique session id in string form.
  
  int _comdraw_port;
  // port used for comdraw command interpreter
  
#endif /* HAVE_ACE */
};

#endif
