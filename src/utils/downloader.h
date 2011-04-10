/*
  downloader.h

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

#ifdef LIBGPLUSPLUS
#include <String.h>
#else
#include <OS/string.h>
#endif

class CDownLoader {

public:
  CDownLoader(char*, char*);


private:

  bool http, ftp;    // Determines which protocol to use.

  String url;        // URL to retrieve, given on command line.
  char *localfile;   // Name to "save file as", given on command line.
 
  int port;                  // Port for remote socket connection.
  String s_port;             // Temporary string for port to convert to int.
#ifdef LIBGPLUSPLUS
  String hostname, remotefile; // Actual server name and file to retrieve.
#else
  CopyString hostname;
  CopyString remotefile; // Actual server name and file to retrieve.
#endif
  
  void Parse(); // Parses URL contained in this String url.

};

