/*
  sockets.cc

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


#include <errno.h>
#include "sockets.h"

CSocket::CSocket(String hstnm, int prt) {

  hostname = hstnm;
  port = prt;

};

void CSocket::PConnect(){

  // We first get out hostname so we're not stuck with the loopback addr
  // as our only known address.
  char thishostname[64];
  struct hostent *hp;
  gethostname(thishostname, sizeof(thishostname));
  if((hp = gethostbyname(thishostname)) == NULL)
    Thrower("Host Unknown.  Modem down?");

  // Setup for  a passive connection to port ListenPort.
  
  int sockoptval = 1;
  
  // get a socket into TCP/IP
  //   AF_INET is the Internet address (protocol) family
  //   with SOCK_STREAM we ask for a sequenced, reliable, two-way
  //   conenction based on byte streams.  With IP, this means that
  //   TCP will be used 
  
  if ((Psocket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    Thrower("cannot create socket");
  }
  
  setsockopt(Psocket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&sockoptval,
	     sizeof(int));
  
  // set up our address
  // htons converts a short integer into the network representation
  // htonl converts a long integer into the network representation
  // INADDR_ANY is the special IP address 0.0.0.0 which binds the
  // transport endpoint to all IP addresses on the machine.
  
  memset((char*)&my_addr, 0, sizeof(my_addr));	// 0 out the structure
  my_addr.sin_family = AF_INET;	// address family
  my_addr.sin_port = htons(ListenPort);  // =0; lets system pick one?
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Populate the my_addr structure with our hostname info.
  // Now we wont be stuck with the loopback as our only known address.
  bcopy(hp->h_addr, (char *)&my_addr.sin_addr, hp->h_length);

  // bind to the address to which the service will be offered
  
  if (bind(Psocket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    Thrower("bind failed");

  // set up the socket for listening with a queue length of 2
  
  if (listen(Psocket_fd, 2) < 0) {
    Thrower("listen failed");
  }

};

int CSocket::Listen(){
  
   Palen = sizeof(Pclient_addr); // length of address
  
  // Wait for connection.
  // If we don't Place some sort of timing on this..... may we hang forever?
  
  int data_fd;
  if ((data_fd = accept(Psocket_fd,
			(struct sockaddr *)&Pclient_addr, 
			(unsigned int *)&Palen)) < 0) {
    // we can break out of accept if the system call was interrupted
#ifndef ERESTART	/* ERESTART not defined on some systems */
    if (errno != EINTR) {
#else
    if ((errno != ERESTART) && (errno != EINTR)) {
#endif /* ERESTART */
      Thrower("accept failed");
    }
  }
  return data_fd;
};

int CSocket::AConnect(){

  struct hostent *hp;           //
  struct sockaddr_in myaddr;    // Stuff for sockets.
  struct sockaddr_in servaddr;  //

  int CS_fd;

  // get a tcp/ip socket
  // request the Internet address protocol and a reliable 2-way byte stream
  if ((CS_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    Thrower("Cannot create socket");
  }

  // bind to an arbitrary return address
  // because this is the client side, we don't care about the address
  // no application will connect here  ---
  // INADDR_ANY is the IP address and 0 is the socket
  // htonl converts a long integer (e.g. address) to a network representation

  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(0);

  if (bind(CS_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    Thrower("Bind to socket failed");
  }

  // fill in the server's address and data
  // htons() converts a short integer to a network representation

  memset((char*)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  // look up the address of the server given its name
#ifdef LIBGPLUSPLUS
  hp = gethostbyname((const char*) hostname);
#else
  hp = gethostbyname((const char*) hostname.string());
#endif
  if (!hp) {
    Thrower("Could not obtain address of specified host.  Bad hostname?");
  }
  
  // put the host's address into the server address structure
  memcpy((caddr_t)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* connect to server */
  if (connect(CS_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    Thrower("Connect to server failed.");
  }
  
  return CS_fd; // return socket file descriptor.
  
};

int CSocket::ReadWrite(fstream& in, String header, char* localfile) {
  
  char buffer[BUFSIZ]; // Buffer for I/O.
  
  if( *localfile == '-' ) {

    // If header was passed in, server is 0.9, and all info 
    // from server is for file, including what we called the "header".
#ifdef LIBGPLUSPLUS
    if(header) {
      int length = strlen(header);
      cout.write(header, length);
    }
#else
    if(header.length()) {
      cout.write(header.string(), header.length());
    }
#endif
    
    if(!in.is_open()) {
      Thrower("Network connection was broken or connection timed out before retrieving file.");
      return 0;
    }
    
    while(in.read(buffer, DownLoadAmount)){    
      cout.write(buffer, DownLoadAmount);

      if(!in.is_open()) {
	Thrower("Network connection was broken or connection timed out before retrieving file.");
	return 0;
      }
    }
    
    // Only put that which was last read onto output stream.
    cout.write(buffer, in.gcount());
    cout.flush();
    
  } else {
    
    // TODO:
    // HOW DOES THE USER WANT TO OPEN THE FILE ??
    // Overwrite??
    // Delete file if not completely downloaded.
    
    ofstream fout(localfile);

    // Check the output file.
    if(!fout.is_open()) {
      Thrower("Problem opening file for saving download.");
      return 0;
    }

    // Check the input file.
    if(!in.is_open()) {
      Thrower("Network connection was broken or connection timed out before retrieving file.");
      return 0;
    }

    // If header was passed in, server is 0.9, and all info 
    // from server is for file, including what we called the "header".
#ifdef LIBGPLUSPLUS
    if(header) {
      int length = strlen(header);
      fout.write(header, length);
    }
#else
    if(header.length()) 
      fout.write(header.string(), header.length());
#endif
    
    while(in.read(buffer, DownLoadAmount)) {
      if(!fout.write(buffer, DownLoadAmount)) {
	// DELETION OF DOWNLODED AMOUNT SHOULD OCCUR HERE.
	Thrower("Problem saving download to file.");
	return 0;
      }
      if(!in.is_open()) {
	Thrower("Network connection was broken or connection timed out while trying to retrieve file.");
	// DELETION OF DOWNLODED AMOUNT SHOULD OCCUR HERE.
	return 0;
      }
    }
    
    // Only write out amount of last read.
    if(!fout.write(buffer, in.gcount())) {
      Thrower("Problem saving download to file.");
      // DELETION OF DOWNLODED AMOUNT SHOULD OCCUR HERE.
      return 0;
    }
    
    // Close the output file.
    fout.flush();
    fout.close();
    
  } // end else clause.
  
  // Everything went just fine....
  return 1;
  
};


void CSocket::Shutdown(int filedescriptor) {
  
  if((shutdown(filedescriptor, 2)) <0)
    ;
  
};

