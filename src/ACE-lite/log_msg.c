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
 * Standard conversions (%s %d %u %x %c %f %% and their l/ll/z/j/t length
 * modifiers) pass through to printf one piece at a time -- and CRUCIALLY each
 * is read with the va_arg type matching its length modifier.  Reading a %ld/%lu
 * arg as a plain int would, on LP64, consume 4 bytes where the caller pushed 8
 * and desync the va_list for every following argument.
 */

#include <ACE-lite/Log_Msg.h>

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
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
        // Collect a full conversion spec "%[flags/width/.prec/length]<conv>".
        char spec[32];
        int si = 0;
        spec[si++] = *p++;            // the '%'
        while (*p && !strchr("PtpsdiuxXcfgeEgGo%", *p) && si < (int)sizeof(spec) - 2) {
            spec[si++] = *p++;
        }
        if (*p == '\0') { spec[si] = '\0'; fputs(spec, stderr); break; }
        char conv = *p;
        spec[si++] = conv;
        spec[si] = '\0';

        // Length modifier sits in the 1-2 chars just before the conversion.
        char m1 = (si >= 3) ? spec[si - 2] : 0;
        char m0 = (si >= 4) ? spec[si - 3] : 0;
        bool is_ll = (m1 == 'l' && m0 == 'l');
        bool is_l  = (m1 == 'l' && !is_ll);
        bool is_z  = (m1 == 'z');
        bool is_j  = (m1 == 'j');
        bool is_t  = (m1 == 't');
        bool is_L  = (m1 == 'L');
        bool is_unsigned = (conv == 'u' || conv == 'x' || conv == 'X' || conv == 'o');

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
        case 'f': case 'g': case 'e': case 'E': case 'G':
            if (is_L) fprintf(stderr, spec, va_arg(ap, long double));
            else      fprintf(stderr, spec, va_arg(ap, double));
            break;
        default:                      // d i u x X c o  -- match the length modifier
            if (is_ll) {
                if (is_unsigned) fprintf(stderr, spec, va_arg(ap, unsigned long long));
                else             fprintf(stderr, spec, va_arg(ap, long long));
            } else if (is_l) {
                if (is_unsigned) fprintf(stderr, spec, va_arg(ap, unsigned long));
                else             fprintf(stderr, spec, va_arg(ap, long));
            } else if (is_z) {
                fprintf(stderr, spec, va_arg(ap, size_t));
            } else if (is_j) {
                fprintf(stderr, spec, va_arg(ap, intmax_t));
            } else if (is_t) {
                fprintf(stderr, spec, va_arg(ap, ptrdiff_t));
            } else {                  // default argument promotions: int width
                if (is_unsigned) fprintf(stderr, spec, va_arg(ap, unsigned int));
                else             fprintf(stderr, spec, va_arg(ap, int));
            }
            break;
        }
    }

    va_end(ap);
}
