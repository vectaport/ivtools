/*
  sockets.h

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

#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#ifdef LIBGPLUSPLUS
#include <String.h>
#else
#include <OS/string.h>
#endif
#include <fstream.h>
#include <iostream.h>
#include <string.h>    // memset()
#include <stdio.h>     // BUFSIZ
#include "thrower.h"
const int DownLoadAmount = 1024; // Amount per socket read.

#if !defined(SOCKLEN_T_DEFINED) || !SOCKLEN_T_DEFINED
typedef int socklen_t;
#endif

// Perhaps.. a value of 0 allows the system to choose a port.
const int ListenPort = 20003; // Port for receiving FTP data.


class CSocket {

 protected:

  // PConnect() and Listen() share the following 4 data members.
  int Psocket_fd;  // socket file descriptor (transport)
  int Palen;	   // length of address structure
  struct sockaddr_in Pclient_addr; // client's address
  struct sockaddr_in my_addr;      // address of this service 

#ifdef LIBGPLUSPLUS
  String hostname;
#else
  CopyString hostname;
#endif
  int port;

  CSocket(String, int); // initializes above "hostname" an "port" members.

  int AConnect();  // Creates an active socket connection.
  void PConnect();  // Sets up a passive socket connection.
  int Listen(); // Blocks until incoming connection is made.

  int ReadWrite(fstream&, String, char*); // Socket reading and file writing. 
  
  // Closes a socket connection, given a file descriptor.
  void Shutdown(int);             
  
};

