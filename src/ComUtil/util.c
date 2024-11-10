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

