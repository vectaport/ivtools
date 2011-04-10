/*
  downloader.cc

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

#include <stdlib.h>
#include "downloader.h"
#include "http.h"
#include "ftp.h"

CDownLoader::CDownLoader(char *a_url, char *a_file) {

  url = a_url;
  localfile = a_file;
  
  Parse();

  if( http ) { 
    CHttp Http(hostname, port, remotefile, localfile);
    Http.DoGet();
    }

  if( ftp ) { 
    CFtp Ftp(hostname, port, remotefile, localfile);
    Ftp.DoRetrieve();
  }
 
};

void CDownLoader::Parse() {
  
  http = false;
  ftp = false;

  // Determine protocol.

  if( url.contains("http://", 0) ){ 

#ifdef LIBGPLUSPLUS
    url.at("http://") = "";     // Trim protocol from URL
#else
    url.set_to_right(7);        // Trim protocol from URL
#endif
    http = true;

  } else { 

    if( url.contains("ftp://", 0) ){ 

#ifdef LIBGPLUSPLUS      
      url.at("ftp://") = "";     // Trim protocol from URL 
#else
      url.set_to_right(6);       // Trim protocol from URL 
#endif
      ftp = true;

    } else { // Specified protocol is bad
      Thrower("Unimplemented protocol in URL");
    }
  }

  if( url.contains(":") ) { // Port number has been specified

#ifdef LIBGPLUSPLUS    
    hostname = url.before(":"); // Determine Host name
    url.before(":") = "";
    url.at(":") = "";
    s_port = url.before("/"); // Determine Port number
    port = atoi((const char*)s_port);
#else
    hostname = url.before(":"); // Determine Host name
    url.set_to_right(hostname.length()+1);
    s_port = url.before("/"); // Determine Port number
    s_port.convert(port);
#endif

  } else {
    
    hostname = url.before("/"); // Determine Host name

    if( hostname == ""){ hostname = url; } // No trailing slash in URL.

    if(http){ port = 80; }
    if(ftp) { port = 21; }

  }
  
  if ( (hostname == "") || (hostname.contains("/")) ){
    
    Thrower("Bad URL. Extra Slash or No Server Name?");
    
  } else { 
    // Remove hostname from URL to get filename
    remotefile = url.from("/");
    
    if( remotefile == "" ){ remotefile = "/"; } // No trailing slash in URL.

  }
  
  // At this point we assume that the hostname and 
  // filename are correct.  We'll let the sockets (before connection)
  // or the server (after connection) tell us if these are specified
  // incorrectly.

#ifdef DEBUG
  cerr << endl << "In Parse()" << endl;
  cerr << "hostname is: " << hostname << endl;
  cerr << "remotefile is: " << remotefile << endl;
  cerr << "port no. is: " << port << endl;
  cerr << endl;
#endif
  
};
