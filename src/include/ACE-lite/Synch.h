/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Synch.h
 * ACE_Null_Mutex / ACE_NULL_SYNCH -- the no-op synchronization used as template
 * parameters in ivtools' single-threaded reactor world.
 */

#ifndef _acelite_Synch_h
#define _acelite_Synch_h

// A mutex that does nothing (single-threaded build).
class ACE_Null_Mutex {
public:
    ACE_Null_Mutex() {}
    int acquire() { return 0; }
    int release() { return 0; }
    int tryacquire() { return 0; }
    int remove() { return 0; }
};

// The null synchronization-strategy tag used by ACE_Svc_Handler<PEER, SYNCH>.
class ACE_Null_Synch {};

#ifndef ACE_NULL_SYNCH
#define ACE_NULL_SYNCH ACE_Null_Synch
#endif

#endif /* _acelite_Synch_h */
