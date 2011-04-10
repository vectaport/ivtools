/*
 * Copyright (c) 1993-1995 Vectaport Inc.
 * Copyright (c) 1989 Triple Vision, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
atox.c          Routines to convert hex and octal strings to internal numbers

Externals:      unsigned long atox(), unsigned long atoo()

History:        Written by Scott E. Johnston, March 1989
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "comutil.ci"


/*!

atox    Convert character string of hexadecimal digits to an unsigned long


Summary:

#include <comterp/comutil.h>
*/

unsigned long atox(char * string )


/*!
Return Value:  returns the unsigned long value


Parameters:

Type            Name          IO  Description 
------------    -----------   --  -----------                  */
#ifdef DOC
char *          string    ;/* I   String to be converted. */
#endif

/*!
Description:

`atox` converts a character string of hexadecimal digits to an unsigned long
value.  Conversion is limited to the number of hexadecimal digits supported
by an unsigned long, and is terminated if a non-hexadecimal digit is 
encountered.  No indication of either condition is given, because `atox` is
designed for use in situations where the extent and content of the
hexadecimal string has already been verified.

!*/

{
int string_length;
unsigned long value = 0;
int index;

   string_length = min( sizeof(unsigned long) * 2, strlen( string ));

   for( index=0; index<string_length; index++ ) {
      if( !isxdigit( string[index] ))
	 return value;
      value <<= 4;
      value |= HEXVAL( string[index] );
      }

   return value;
}




/*!

atoo    Convert character string of octal digits to an unsigned long


Summary:

#include <ComUtil/comutil.h>
*/

unsigned long atoo( char * string )


/*!
Return Value:  returns the unsigned long value



Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char *          string    ;/* I   String to be converted. */
#endif

/*!
Description:

`atoo` converts a character string of octal digits to an unsigned long
value.  Conversion is limited to the number of octal digits supported
by an unsigned long, and is terminated if a non-octal digit is 
encountered.  No indication of either condition is given, because `atoo` is
designed for use in situations where the extent and content of the
octal string has already been verified.

!*/

{
int string_length;
unsigned long value = 0;
int index;

   string_length = min( sizeof(unsigned long) * 8 / 3, strlen( string ));

   for( index=0; index<string_length; index++ ) {
      if( !isodigit( string[index] ))
	 return value;
      value <<= 3;
      value |= OCTVAL( string[index] );
      }
  
   return value;
}
