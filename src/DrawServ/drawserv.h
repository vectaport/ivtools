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
#include <stdint.h>   /* uint32_t -- pulled in transitively on macOS, not on Linux */
#include <strstream>
#include <uuid/uuid.h>
#if !defined(__APPLE__) && !defined(IV_UUID_STRING_T_DEFINED)
#define IV_UUID_STRING_T_DEFINED
typedef char uuid_string_t[37];  /* Apple-only type; Linux libuuid lacks it */
#endif

// utility function for grabbing key from uuid_t.
extern uint32_t uuid_key(const uuid_t u);

typedef uint32_t freezeid_t;
// id of a linkfreeze hold: the wire protocol only needs enough entropy to
// tell apart the handful of freezes ever concurrently in flight, not a
// full uuid_t

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
  
  DrawLink* linkup(const char* hostname, int portnum, 
		   int state, uuid_t link_id=NULL, ComTerp* comterp=nil,
		   int interactive=false);
  // Create new link to remote drawserv (state: 0==new_link, 1==one_way,
  // 2==two_way), return -1 if error.
  
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
  // register a session id learned from link, or, when that sid is
  // already recorded via a different link, bench whichever of the two
  // links loses the tie-break

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

  void bench(DrawLink* link);
  // set a link aside as a redundant path: keep it open and in the link
  // list, but stop routing broadcast traffic across it, and tell the
  // peer at its other end to do likewise

  boolean sole_active_link(DrawLink* link);
  // whether link is the only remaining non-benched link, so benching it
  // (locally, or on a peer's say-so) would cut this node off entirely

  boolean freeze_fragment(freezeid_t& qid_out, const uuid_t linkid);
  // originate a fragment freeze for the link formation identified by
  // linkid: flood a request (its first 4 bytes as the wire id) across
  // every two_way link and block, pumping the reactor, until every
  // neighbor has echoed an ack or the attempt times out; on success
  // qid_out identifies the hold, to release later via unfreeze_fragment().
  // Deriving the id from linkid rather than generating an unrelated one
  // ties a freeze's req/ack/thaw log lines to the connection they protect.

  void unfreeze_fragment(freezeid_t qid);
  // release a fragment freeze this node originated, flooding the release
  // to the same links the request went to

  void freeze_request_handle(DrawLink* fromlink, freezeid_t qid);
  // handle a freeze request, whether self-originated (fromlink==nil) or
  // relayed from a peer: relay it to every other two_way link and echo an
  // ack once every relay target has echoed back; a request for a qid
  // other than one already held here is dropped rather than queued, so
  // its sender simply times out and can retry once the hold is released

  void freeze_ack_handle(DrawLink* fromlink, freezeid_t qid);
  // record an echoed ack for the freeze this node is currently relaying
  // or originating; once every relay target has acked, echo onward (or,
  // for the originator, unblock the waiting freeze_fragment() call)

  void freeze_release_handle(DrawLink* fromlink, freezeid_t qid);
  // relay a freeze release to the same links its request went to and
  // clear the local hold

  void grid_message(GraphicId* grid);
  // generate graphic id selection message

  void grid_message_handle(DrawLink* link, uuid_t id, uuid_t selector, 
			   int state, uuid_t newselector=NULL, int gen = 0);
  // handle graphic id selection message

  void grid_deny(DrawLink* link, uuid_t id, uuid_t requester, uuid_t denier,
		 int gen = 0);
  // handle a refusal, relaying it on when we are not the one who asked

  void grid_notaken(DrawLink* link, uuid_t id, uuid_t responder, uuid_t granter,
		    int gen = 0);
  // handle a grant of ours that the responder could not take

  void grid_message_callback(DrawLink* link, uuid_t id, uuid_t selector, 
			     int state, uuid_t oldselector, int gen = 0);
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
  // alternate method for serializing an AttributeList; returns false
  // if really not there.

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

  boolean _freeze_active;
  // whether this node currently holds a fragment freeze, as originator
  // or as a relay
  freezeid_t _freeze_qid;
  // id of the freeze held in _freeze_active
  DrawLink* _freeze_parent;
  // link the held freeze's request arrived from; nil when self-originated
  DrawLinkList* _freeze_sent_to;
  // links the held freeze's request was relayed to, reused to fan out
  // its eventual release
  int _freeze_acks_pending;
  // entries of _freeze_sent_to not yet acked back
  boolean _freeze_done;
  // for the originator: whether every relay target has acked, unblocking
  // the freeze_fragment() wait loop

  void freeze_clear();
  // drop the local hold unconditionally, without flooding a release

};

#endif
