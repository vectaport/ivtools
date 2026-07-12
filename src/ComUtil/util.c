/*
 * Copyright (c) 1993-1995 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software without 
 * specific, written prior permission.  The copyright holders make no 
 * representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* low-level utility functions */

int   Kaput_On = 1;
int   TITLE = 0;

#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/time.h>

void print_stack_trace() {
    void *buffer[10];
    int nptrs = backtrace(buffer, 10);
    char **symbols = backtrace_symbols(buffer, nptrs);

    if (symbols == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Stack trace:\n");
    for (int i = 0; i < nptrs; i++) {
        fprintf(stderr, "%s\n", symbols[i]);
    }
    fflush(stderr);

    free(symbols);
}

const char* shell_string(const char* cmd) {
    static char buffer[BUFSIZ];
    FILE* fp = popen(cmd, "r");
    if (fp) {
        if (!fgets(buffer, BUFSIZ, fp))
            buffer[0] = '\0';
        buffer[strcspn(buffer, "\n")] = 0;  // strip newline
        pclose(fp);
    } else
        buffer[0] = '\0';
    return buffer;
}

const char* local_hostname() {
    static char buffer[MAXHOSTNAMELEN];
#ifdef __APPLE__
    static const char* name = shell_string("scutil --get LocalHostName");
    static bool ready = false;
    if (!ready) {
        strncpy(buffer, name, MAXHOSTNAMELEN);
	buffer[MAXHOSTNAMELEN-1] = '\0'; // guarding against strncpy overwrite
        strncat(buffer, ".local", MAXHOSTNAMELEN - strlen(buffer) - 1);
	ready = true;
    }
    if (ready) return buffer;
#endif
    gethostname(buffer, MAXHOSTNAMELEN);
    return buffer;
}

void log_with_timestamp(const char* msg) {
  int n = strlen(msg);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  fprintf(stderr, "[%ld.%06ld] %s", tv.tv_sec%100, (long)tv.tv_usec, n==0 ? "NULL MESSAGE" : msg);
  if (n==0 || msg[n-1]!='\n') {
    fprintf(stderr, "\n");
  }
  fflush(stderr);
}

/* ca_tz: California build-timezone hint derived from the build month.  __DATE__
   carries no zone, so this is a hint, correct for a California build clock:
   Pacific Daylight (PDT) ~ mid-Mar..early-Nov, Pacific Standard (PST) otherwise.
   The two switch months (Mar, Nov) can't be exact from the month alone; Apr..Oct
   are unambiguously PDT, Dec..Feb unambiguously PST, and Mar/Nov default to PST
   (the standard side).  Emitted as the trailing element of the timestamp
   colon-chain -- month opens the date, timezone closes the time.  See the ":"
   operator timezone extension. */
static const char* ca_tz(const char* mon) {
  /* PDT for Apr,May,Jun,Jul,Aug,Sep,Oct; PST otherwise (incl. Mar,Nov defaults) */
  static const char* pdt[] = {"Apr","May","Jun","Jul","Aug","Sep","Oct"};
  for (int i=0; i<7; i++)
    if (mon[0]==pdt[i][0] && mon[1]==pdt[i][1] && mon[2]==pdt[i][2]) return "PDT";
  return "PST";
}

/* build_stamp: build identity as a string in comterp attrlist-literal notation --
   (:built YYYY:Mon:DD:HH:MM:SS:TZ :commit "hash").  The colon-chain is the readable
   date/time/zone literal the post-eval ":" operator consumes (TZ a trailing
   timezone symbol, like the month, closing the timestamp); nothing here evaluates
   it -- the banner just prints a string comterp *could* parse once ":" knows the
   timezone vocabulary.  date/time are a caller's __DATE__/__TIME__ ("Mon DD YYYY"
   day space-padded, "HH:MM:SS") and commit_id its COMMIT_ID -- those stay in each
   main.c (via #include "gitcommitid.h", regenerated at build time -- see
   config/gitcommitid.sh) so a fresh COMMIT_ID recompiles that main.c and
   refreshes its __DATE__/__TIME__ too; only this formatter lives here. */
const char* build_stamp(const char* date, const char* time, const char* commit_id) {
  static char buf[160];
  char mon[4]; mon[0]=date[0]; mon[1]=date[1]; mon[2]=date[2]; mon[3]=0;
  int day = atoi(date+4);
  int year = atoi(date+7);
  snprintf(buf, sizeof(buf),
           "(:built %d:%s:%d:%s:%s :commit \"%s\")",
           year, mon, day, time, ca_tz(mon), commit_id);
  return buf;
}

char* restore_escapes(const char* str, int& bufsize) {
    bufsize = strlen(str)*2+2;
    char* dst = new char[bufsize];
    char* dptr = dst;
    const char* src = str;
    while (*src) {
        if (*src == '\n') {
            *dptr++ = '\\';
            *dptr++ = 'n';
        } else if (*src == '\t') {
            *dptr++ = '\\';
            *dptr++ = 't';
        } else if (*src == '\r') {
            *dptr++ = '\\';
            *dptr++ = 'r';
        } else {
            *dptr++ = *src;
        }
        src++;
    }
    *dptr = '\0';
    return dst;
}

int stdout_puts(const char* s, void* ignored) { 
    int ret = fputs(s, stdout);
    fflush(stdout);
    return ret;
}
