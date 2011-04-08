/*
  http.cc

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

#include "http.h"
#include "thrower.h"

CHttp::CHttp(String hst, int prt, String rmtfile, char* lclfile) 
  : CSocket(hst, prt) {

  hostname = hst;
  port = prt;
  remotefile = rmtfile;
  localfile = lclfile;
  
};

void CHttp::DoGet() {

  String request;  // Request to send to HTTP server.
  char reqbuf[BUFSIZ];

  int fd = AConnect(); // hostname, port);     // Connect to host hostname at port.

  // HTTP 0.9 Request format.  For debugging only.
#ifdef OLD  
#ifdef LIBGPLUSPLUS
  request = "GET " + remotefile + " \r\n";
#else
  sprintf(reqbuf, "GET %s \r\n", remotefile.string());
  request = reqbuf;
#endif
#endif

  // HTTP 1.0 Request format.
#ifndef OLD
#ifdef LIBGPLUSPLUS
  request = "GET " + remotefile + " HTTP/1.0\r\n\n";
#else
  sprintf(reqbuf, "GET %s HTTP/1.0\r\n\n", remotefile.string());
  request = reqbuf;
#endif
#endif
  
#ifdef DEBUG
  cerr << "The request is: " << request; 
  cerr.flush();
#endif
 
  // Write out request on socket.
  
  fstream socket_stream(fd);

  if (!socket_stream.is_open()) {
    Thrower("Socket connection closed before processing.");
    Shutdown(fd);
  }

  // Send request to server.
  if(! ( socket_stream << request) ){
    Shutdown(fd);
    Thrower("HTTP request could not be written to network.");
  }
  
  char header[BUFSIZ]; // Buffer for HTTP 1.0 header.

  // Retrieve reply from server.
  socket_stream.getline(header, BUFSIZ, '\n'); //First line from socket.
  if(socket_stream.fail()){
    socket_stream.close();
    Shutdown(fd);
    Thrower("HTTP server reply could not be read from network.");
  }

  String reply(header);
  
  if(reply.contains("HTTP/",0)) { // HTTP >= 1.0 server

    
#ifdef DEBUG
    cerr << endl << "HTTP 1.0 (or greater) header:  " 
	 << reply 
	 << endl << endl;
#endif

    
    if(reply.contains("200")) { // 200 OK Document will follow.

      int i = 1;
      while( i ) { 

	// Skip over headers
	if(!(socket_stream.getline(header, BUFSIZ, '\n'))){ 
	  Thrower("Socket connection closed while reading header.");
	  socket_stream.close();
	  Shutdown(fd);
	}

	if (strlen(header) <= 1) break;  //Single NewLine marks end of header. 
	
#ifdef DEBUG
	cerr << header << endl;
#endif
	
	++i;
	if(i > 200) {
	  Thrower("Server Error: HTTP Header contains over 200 lines.");
	  socket_stream.close();
	  Shutdown(fd);
	}
      }
      

#ifdef DEBUG
      cerr << endl; // NewLine between header and file.
#endif

      // Read from socket and write to file.
      if(!(ReadWrite(socket_stream, (const char *)NULL, localfile))) {
	socket_stream.close();
	Shutdown(fd);
      }
      
    } else { // Handle HTTP 1.0 or greater Errors.

      Shutdown(fd); // Shutdown socket and delete descriptor

      if(reply.contains("301"))
	Thrower("Document has been moved permanently");

      if(reply.contains("302"))
	Thrower("Document has been moved temporarily.");

      if(reply.contains("401"))
	Thrower("Unauthorized: the document is restricted.");

      if(reply.contains("402"))
	Thrower("The information requires paying a fee.");

      if(reply.contains("403"))
	Thrower("Access is forbidden.");

      if(reply.contains("404"))
	Thrower("The document could not be found at the specified URL.");

      if(reply.contains("500"))
	Thrower("The server experienced an error.");

      // else.
      Thrower("An unspecified HTTP error has occurred.");

    }

  } else { // HTTP 0.9 server
    
    if(reply.contains("<HTML>")) { // HTTP 0.9 Error
      
      Thrower("Either you downloaded HTML, or the server reported an error.");
      
    } else { // HTTP 0.9 Success
      
      // We have to send header or it's lost. 
      if(!(ReadWrite(socket_stream, header, localfile))) {
	socket_stream.close();
	Shutdown(fd);
      }
    }
  }
  
  Shutdown(fd);  // Shut down socket connection.
  
};

