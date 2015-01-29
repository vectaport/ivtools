/*
  thrower.cc

  Copyright (c) 1998 Eric F. Kahler
        
  Permission to use, copy, modify, distribute, and sell this software and
  its documentation for any purpose is hereby granted without fee, provided
  that the below copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation, and that the names of the copyright holders not be used in
  advertising or publicity pertaining to distribution of the software
  without specific, written prior permission.  The copyright holders make
  no representations about the suitability of this software for any purpose.
  It is provided "as is" without express or implied warranty.
  
  THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
  SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
  INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.             
*/

#include "thrower.h"
#include <stdlib.h>

// Yes this is utterly simple, but can be revised for more sophisticated
// error handling without editing all of the other files involved in 
// the downloader program.        

void Thrower(char *s) {

  cerr << endl << s << endl << endl;
  exit(8);
}


