/*
  ftp.cc

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
#include "sockets.h"
#include "ftp.h"

CFtp::CFtp(String hst, int prt, String rmtfile, char* lclfile) 
  : CSocket(hst, prt) {
  
  hostname = hst;
  port = prt;
  remotefile = rmtfile;
  localfile = lclfile;
  
};

void CFtp::DoRetrieve() {

  String ftpsend;      // String of commands to send to FTP server.
  char reply[BUFSIZ];  // Buffer for FTP server replies.

  // Connect to host "hostname" at port "port".
  int control_fd = AConnect(); 

  fstream control_stream(control_fd);
  
  // Check for steam's existence
  if (!control_stream.is_open()) {
    Thrower("Socket connection closed before processing.");
    Shutdown(control_fd);
  }

  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("Initial FTP server reply could not be read from network.");
  }
  String sreply(reply);
  if( !(sreply.contains("220",0)) ) { // Couldn't log in to FTP server.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
// WHY CAN'T WE LOG INTO SERVER????
    Thrower("Initial connect to FTP server failed.");
  }
  
  // Send User Name to server.
  ftpsend = "User anonymous\r\n";
  if(! (control_stream << ftpsend) ){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP request could not be written to network.");
  }
  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP server reply could not be read from network.");
  }
  sreply = reply;
  if( !(sreply.contains("331",0)) ) { // Couldn't login to FTP server.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
// WHY CAN'T WE LOG INTO SERVER????
    Thrower("Sent User Name anonymous, but couldn't login to FTP server.");
  }

  // Send Password to server.
  ftpsend = "PASS anonymous@anonymous.com\r\n";
  if(! (control_stream << ftpsend) ){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP request could not be written to network.");
  }
  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP server reply could not be read from network.");
  }
  sreply = reply;
  if( !(sreply.contains("230",0)) ) { // Couldn't login to FTP server.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
// WHY CAN'T WE LOG INTO SERVER (Get Error Code!)????
    Thrower("Sent name and password, but couldn't login to FTP server.");
  }

  PConnect();

  // Send FTP Server our IP Address and Listen Port Number,
  // using the following form on one line:
  // IPAddress(comma separated segments),
  // Portnumber/256,
  // remainder of previous divide.
  register unsigned char *p, *a;
  a = (unsigned char *)&my_addr.sin_addr;
  p = (unsigned char *)&my_addr.sin_port;

#define	UC(b)	(((unsigned int)b)&0xff)

  if(! (control_stream << "Port "
	<< UC(a[0]) << "," 
	<< UC(a[1]) << ","
	<< UC(a[2]) << ","
	<< UC(a[3]) << ","
	<< UC(p[0]) << ","
	<< UC(p[1]) << "\r\n" )){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP port command could not be written to network.");
  }
  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP server reply could not be read from network.");
  }

  sreply = reply;
  while (sreply.contains("230",0)) {
    control_stream.getline(reply, BUFSIZ, '\n'); 
    sreply = reply;
  }
  if( !(sreply.contains("200",0)) ) { // PORT command unsuccessful.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
    // GET Error Code!
     Thrower("PORT command was unsuccessful.");
  }

  // Set TYPE of file to retrieve
  ftpsend = "TYPE I\r\n";
  if(! (control_stream << ftpsend) ){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP TYPE command could not be written to network.");
  }
  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP server reply could not be read from network.");
  }
  sreply = reply;
  if( !(sreply.contains("200",0)) ) { // Couldn't set type.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
// WHY CAN'T WE GET FILE FROM SERVER (Get Error Code!)????
    Thrower("Could not set data retieval type.");
  }

  // Send command to Retrieve file.
  if(! (control_stream << "RETR "
	<< remotefile
	<< "\r\n") ){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("RETR request could not be written to network.");
  }
  // Retrieve reply from server.
  control_stream.getline(reply, BUFSIZ, '\n'); //First line from socket.
  if(control_stream.fail()){
    control_stream.close();
    Shutdown(control_fd);
    Thrower("FTP server reply could not be read from network.");
  }
  sreply = reply;
  if( !(sreply.contains("150",0)) ) { // Couldn't login to FTP server.
    control_stream.close();
    Shutdown(control_fd);
// MAKE THIS REPORTING MORE ROBUST
// WHY CAN'T WE GET FILE FROM SERVER (Get Error Code!)????
    Thrower("Could not retreive file from FTP server.");
  }

  int data_fd;    // file descriptor accepting the request
  data_fd = Listen();
  
  // Pass Data Connection Stream to ReadWrite().
  fstream data_stream(data_fd);
  if(!(ReadWrite(data_stream, NULL, localfile))) {
    control_stream.close();
    Shutdown(control_fd);
    data_stream.close();
    Shutdown(data_fd);
    Thrower("Reading and/or Writing of file failed.");
  }

  // Send command QUIT.
  ftpsend = "QUIT\r\n";
  if(! (control_stream << ftpsend) ){
    control_stream.close();
    Shutdown(control_fd);
    data_stream.close();
    Shutdown(data_fd);
    Thrower("FTP QUIT command could not be written to network.");
  }

  // Shutdown all connections.
  control_stream.close();
  Shutdown(control_fd);
  data_stream.close();
  Shutdown(data_fd);
};
