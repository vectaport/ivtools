/*
 * Copyright (c) 1994 Vectaport
 * Copyright (c) 1989 Triple Vision, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software without 
 * specific, written prior permission.  The copyright holders make no 
 * representations about the suitability of this software for any purpose.  
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
types.c		Data type support

Externals:      int print_type()

Summary:

History:        Written by Scott E. Johnston, September 1988
*/

#include <stdio.h>

#include <ComUtil/util.h>


/*!

print_type  Print value of given type


Summary:

#include <fsconfig.h>
#include <util.h>
*/

int print_type(FILE * fptr,unsigned dtype,char * dptr,int offset)


/*!
Return Value:  Number of characters written


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *          fptr      ;/* I   Output file pointer.         */
unsigned        dtype     ;/* I   Type of datum.               */
char *          dptr      ;/* I   Pointer to datum to print.   */
int		offset    ;/* I   Offset from `dptr`.          */
#endif


/*!
Description:

`print_type` prints the contents of a data structure pointed to by `dptr`, 
using a format indicated by `dtype`.  `offset` increments `dptr` by the 
necessary amount, according to `type`. The supported types are defined by
the following macros in `util.h`:

	Macro Definition  C Equivalent	  Description
        ----------------  ------------    -----------
	DFINTPAR	  int             default signed integer
	DFUNSPAR	  unsigned        default unsigned integer
	SHINTPAR   	  short           16-bit signed integer
        SHUNSPAR          unsigned short  16-bit unsigned integer
        LNINTPAR          long            32-bit signed integer
        LNUNSPAR          unsigned long   32-bit unsigned integer
        FLOATPAR          float           floating point number
	DOUBLPAR          double          double-precision floating point no.
        CHAR_PAR          char            character
	STRNGPAR          char *          character string
        BOOL_PAR          unsigned        boolean


!*/

{

   switch (dtype) {
      case DFINTPAR:
         return fprintf( fptr, DFINT_PRFORM, *((int *)dptr+offset) );
      case DFUNSPAR:
         return fprintf( fptr, DFUNS_PRFORM, *((unsigned int *)dptr+offset) );
      case SHINTPAR:
         return fprintf( fptr, SHINT_PRFORM, *((short *)dptr+offset) );
      case SHUNSPAR:
         return fprintf( fptr, SHUNS_PRFORM, *((unsigned short *)dptr+offset) );
      case LNINTPAR:
         return fprintf( fptr, LNINT_PRFORM, *((long *)dptr+offset) );
      case LNUNSPAR:
         return fprintf( fptr, LNUNS_PRFORM, *((unsigned long *)dptr+offset) );
      case FLOATPAR: 
         return fprintf( fptr, FLOAT_PRFORM, *((float *)dptr+offset) );
      case DOUBLPAR: 
         return fprintf( fptr, DOUBL_PRFORM, *((double *)dptr+offset) );
      case CHAR_PAR: 
         return fprintf( fptr, "%c", *(dptr+offset) );
      case STRNGPAR: 
         return fprintf( fptr, "%s", *((char **)dptr+offset) );
      case BOOL_PAR:  
         return fprintf( fptr, "%s", (*((BOOLEAN *)dptr+offset) ? "TRUE" : "FALSE" ));
      }

   return 0;

}

