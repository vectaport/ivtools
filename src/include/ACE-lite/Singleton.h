/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Singleton.h
 * ACE_Singleton<TYPE, LOCK> -- one shared instance of TYPE, reached via
 * instance().  The LOCK parameter is accepted for signature compatibility and
 * ignored (single-threaded build).
 */

#ifndef _acelite_Singleton_h
#define _acelite_Singleton_h

template <class TYPE, class ACE_LOCK>
class ACE_Singleton {
public:
    static TYPE* instance() {
        static TYPE instance_;
        return &instance_;
    }
};

#endif /* _acelite_Singleton_h */
