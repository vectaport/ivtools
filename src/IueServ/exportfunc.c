/*
 * Copyright (c) 1998 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

// from the IUE
#include <ImageClasses/Image.h>

#include <IueServ/exportfunc.h>
#include <IueServ/iuecomps.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#endif

/*-----------------------------------------------------------------------*/

IueExportFunc::IueExportFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueExportFunc::execute() {
  ComValue iueobj(stack_arg(0));
  ComValue host(stack_arg(1));	
  ComValue port(stack_arg(2));	
  reset_stack();

  IueImageComp* comp = image_comp(iueobj);
  if (comp) {
    Image* image = comp->image();
    if (image) {

#ifdef HAVE_ACE
      const char* hoststr = nil;
      const char* portstr = nil;
      hoststr = host.type()==ComValue::StringType ? host.string_ptr() : nil;
      portstr = port.type()==ComValue::StringType ? port.string_ptr() : nil;
      u_short portnum = portstr ? atoi(portstr) : port.ushort_val();
      
      ACE_SOCK_Stream* socket = nil;
      filebuf fbuf;
      if (portnum) {
	socket = new ACE_SOCK_Stream;
	ACE_SOCK_Connector conn;
	ACE_INET_Addr addr (portnum, hoststr);
	
	if (conn.connect (*socket, addr) == -1)
	  ACE_ERROR ((LM_ERROR, "%p\n", "open"));
	fbuf.attach(socket->get_handle());
      } else
	fbuf.attach(fileno(stderr));

      ostream out(&fbuf);

      unsigned ncols = image->GetSizeX();
      unsigned nrows = image->GetSizeY();
      Image::PixelType iuetype = image->GetPixelType();
      switch (iuetype) {
	case Image::BYTE:   
	case Image::INT8:   
	  out << "P5\n" << ncols << " " << nrows << "\n255\n";
	  for (int y=0; y<nrows; y++) {
	    unsigned char bytes[ncols];
	    image->GetSection(&bytes, 0, y, ncols, 1);
	    out.write(bytes, ncols);
	  }
	  break;

	case Image::INT16:  
	case Image::UINT16: 
	  out << "P5\n" << ncols << " " << nrows << "\n65535\n";
	  for (int y=0; y<nrows; y++) {
	    unsigned short words[ncols];
	    image->GetSection(&words, 0, y, ncols, 1);
	    out.write(words, ncols*2);
	  }
	  break;

	case Image::INT32:  /* truncation might occur */
	case Image::UINT32: 
	  out << "P5\n" << ncols << " " << nrows << "\n65535\n";
	  for (int y=0; y<nrows; y++) {
	    unsigned int ints[ncols];
	    image->GetSection(&ints, 0, y, ncols, 1);
	    for (int x=0; x<ncols; x++) {
	      unsigned short word = ints[x];
	      out.write(&word, 1);
	    }
	  }
	  break;

	case Image::RGB_BYTE:
	  out << "P6\n" << ncols << " " << nrows << "\n255\n";
	  for (int y=0; y<nrows; y++) {
	    unsigned char bytes[ncols*3];
	    image->GetSection(&bytes, 0, y, ncols, 1);
	    out.write(bytes, ncols*3);
	  }
	  break;

	case Image::RGB_16:
	  out << "P6\n" << ncols << " " << nrows << "\n65535\n";
	  for (int y=0; y<nrows; y++) {
	    unsigned short words[ncols];
	    image->GetSection(&words, 0, y, ncols, 1);
	    out.write(words, ncols*2*3);
	  }
	  break;

	  break;

	case Image::RGB_32:
	case Image::FLOAT:  
	case Image::COMPLEX_FLOAT:
	case Image::DOUBLE: 
	case Image::COMPLEX_DOUBLE:
	case Image::UINT64: 
	case Image::INT64:  
	case Image::RGBA:
	case Image::PYRAMID:
	  break;
      }

      out.flush();
      if (socket) {
	if (socket->close () == -1)
	  ACE_ERROR ((LM_ERROR, "%p\n", "close"));
	delete(socket);
      }
#else
      push_stack(ComValue::nullval());
      return;
#endif
    }
  }
}

