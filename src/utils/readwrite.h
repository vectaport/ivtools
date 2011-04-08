/*
  readwrite.h

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

int CDownLoader::ReadWrite(fstream& in, String header) {
  
  char buffer[BUFSIZ]; // Buffer for I/O.
  
  if( *SaveFileName == '-' ) {

    // If header was passed in, server is 0.9, and all info 
    // from server is for file, including what we called the "header".
    if(header) {
      int length = strlen(header);
      cout.write(header, length);
    }
    
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
    
    ofstream fout(SaveFileName);

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
    if(header) {
      int length = strlen(header);
      fout.write(header, length);
    }
    
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
