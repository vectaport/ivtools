/*
 * ACE-lite (issue #147).  src/ACE-lite/log_msg.c
 * (ACE-lite reimplements a subset of the ACE interface; ACE is (c) the DOC
 * group -- see src/ACE-lite/NOTICE.)
 *
 * ace_lite_log -- the stderr logger behind ACE_DEBUG/ACE_ERROR.  ivtools'
 * format strings use a few ACE-specific directives that printf does not
 * understand (and would mis-handle), so we walk the format and translate:
 *   %P -> process id     %t -> thread id (0; single-threaded)
 *   %p -> "<arg>: strerror(errno)" (ACE's perror form; arg is a const char*)
 * Standard %s %d %u %x %c %% (and %ld etc.) pass through to printf one piece at
 * a time, so they stay type-safe with the va_list.
 */

#include <ACE-lite/Log_Msg.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void ace_lite_log(int /*severity*/, const char* format, ...) {
    if (format == 0) return;
    va_list ap;
    va_start(ap, format);

    for (const char* p = format; *p; ++p) {
        if (*p != '%') {
            fputc(*p, stderr);
            continue;
        }
        // Collect a full conversion spec "%[flags/width/length]<conv>".
        char spec[32];
        int si = 0;
        spec[si++] = *p++;            // the '%'
        while (*p && !strchr("PtpsdiuxXcfgeo%", *p) && si < (int)sizeof(spec) - 2) {
            spec[si++] = *p++;
        }
        if (*p == '\0') { spec[si] = '\0'; fputs(spec, stderr); break; }
        char conv = *p;
        spec[si++] = conv;
        spec[si] = '\0';

        switch (conv) {
        case 'P':
            fprintf(stderr, "%ld", (long)getpid());
            break;
        case 't':
            fputc('0', stderr);       // thread id; ACE-lite is single-threaded
            break;
        case 'p': {                   // ACE perror form: "<arg>: <errno msg>"
            const char* msg = va_arg(ap, const char*);
            fprintf(stderr, "%s: %s", msg ? msg : "", strerror(errno));
            break;
        }
        case '%':
            fputc('%', stderr);
            break;
        case 's':
            fprintf(stderr, spec, va_arg(ap, const char*));
            break;
        case 'f': case 'g': case 'e':
            fprintf(stderr, spec, va_arg(ap, double));
            break;
        default:                      // d i u x X c o -> int-width
            fprintf(stderr, spec, va_arg(ap, int));
            break;
        }
    }

    va_end(ap);
}
