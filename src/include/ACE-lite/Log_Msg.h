/*
 * ACE-lite (issue #147).  src/include/ACE-lite/Log_Msg.h
 * The ACE_DEBUG / ACE_ERROR / ACE_ERROR_RETURN / ACE_ASSERT logging macros the
 * consumer code uses, routed to a small stderr logger that understands the few
 * ACE format directives ivtools actually passes (%P pid, %t thread, %p errno).
 *
 * Usage matches ACE: ACE_DEBUG((LM_DEBUG, "fmt", args...)); the doubled parens
 * make the (severity, fmt, ...) list a single macro argument.
 */

#ifndef _acelite_Log_Msg_h
#define _acelite_Log_Msg_h

#include <assert.h>

// Severity tokens (values unused; only their presence as the first arg matters).
enum {
    LM_DEBUG   = 1,
    LM_INFO    = 2,
    LM_WARNING = 4,
    LM_ERROR   = 8
};

// Formatted log to stderr; handles ACE's %P/%t/%p plus standard printf escapes.
void ace_lite_log(int severity, const char* format, ...);

#define ACE_DEBUG(X)            ::ace_lite_log X
#define ACE_ERROR(X)            ::ace_lite_log X
#define ACE_ERROR_RETURN(X, R)  do { ::ace_lite_log X; return R; } while (0)
#define ACE_ASSERT(C)           assert(C)

#endif /* _acelite_Log_Msg_h */
