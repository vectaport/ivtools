/*
 * Copyright (c) 1998 Vectaport, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

main() {
  int r, g, b;
  printf("P3\n# CREATOR: stdcmapppm\n6 36\n255\n");
  for (r=0; r<=255; r+=51) {
    for (g=0; g<=255; g+=51) {
      for (b=0; b<=255; b+=51) {
	printf("%d %d %d ", r, g, b);
      }
      printf("\n");
    }
  }
}

  
