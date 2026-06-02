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
