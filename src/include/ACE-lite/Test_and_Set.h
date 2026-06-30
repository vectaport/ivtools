/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Test_and_Set.h
 * ACE_Test_and_Set<LOCK, TYPE> -- holds a flag toggled by a signal and tested
 * by the event loop.  ivtools uses it (via ACE_Singleton) as the SIGINT quit
 * flag: COMTERP_QUIT_HANDLER::instance()->is_set().
 */

#ifndef _acelite_Test_and_Set_h
#define _acelite_Test_and_Set_h

#include <ACE-lite/Event_Handler.h>

// An event handler whose handle_signal sets a flag the event loop polls.
// Registered for SIGINT via ACE_Reactor::register_handler(signum, handler),
// so ^C makes COMTERP_QUIT_HANDLER::instance()->is_set() return true.
template <class ACE_LOCK, class TYPE>
class ACE_Test_and_Set : public ACE_Event_Handler {
public:
    ACE_Test_and_Set() : is_set_(0) {}

    TYPE is_set() const { return is_set_; }
    void is_set(TYPE value) { is_set_ = value; }

    virtual int handle_signal(int, void* = 0, void* = 0) { is_set_ = 1; return 0; }

private:
    TYPE is_set_;
};

#endif /* _acelite_Test_and_Set_h */
