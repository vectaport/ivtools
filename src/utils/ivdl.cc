/*
  main.cc

  Copyright (c) 1999 Vectaport Inc.
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


#include "downloader.h"
#include <iostream.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
  
  if(argc != 3){
    cerr << "Wrong number of args.\n";
    cerr << "Use:  a.out URL FilenameToSave\n";
    exit(0);
  }

  CDownLoader CDL(argv[1], argv[2]); 


}
