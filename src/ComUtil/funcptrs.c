/*
 * Copyright (c) 1993-1995 Vectaport Inc.
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
funcptrs.c      Routines to convert feof and ferror into function pointers

Externals:      int ffeof(), int fferror()

History:        Written by Scott E. Johnston, March 1989
*/

#include <stdio.h>

#include "comutil.ci"

/*!

ffeof    Function version of feof macro


Summary:

#include <ComUtil/comutil.h>
*/

int ffeof(FILE * stream)


/*!
Return Value:  0 if not at end of file, otherwise non-zero


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *          stream    ;/* I   Pointer to `FILE` structure */
#endif


/*!
Description:

`ffeof` determines whether the end of `stream` has been recognized.   It is
functionally similar to `feof`, except it is implemented as a function 
instead of a macro.  This allows it to be passed as a function pointer
to other functions.

!*/

{
   return feof( stream );
}



/*!

fferror	   Function version of ferror macro


Summary:

#include <ComUtil/comutil.h>
*/

int fferror(FILE * stream)


/*!
Return Value:  0 if not at end of file, otherwise non-zero


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *          stream    ;/* I   Pointer to `FILE` structure */
#endif


/*!
Description:

`fferror` tests for an error in reading or writing `stream`.   It is
functionally similar to `ferror`, except it is implemented as a function 
instead of a macro.  This allows it to be passed as a function pointer
to other functions.

!*/

{
   return ferror( stream );
}



