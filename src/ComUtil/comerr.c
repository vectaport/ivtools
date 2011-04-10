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
comerr.c        COMTERP specific error routines

Externals:      char * comerr_read(), void comerr_set(), 
                int comerr_get(), BOOLEAN comerr_chk()

History:        Written by Scott E. Johnston, March 1989
*/

#include <stdio.h>

#include "comutil.ci"

/* Local Statics */
static int ErrorId = -1;




/*! 

comerr_read	Read error format string from COMTERP error file


Summary:

#include <ComUtil/comutil.h>
*/

char * comerr_read( unsigned errnum )


/*!
Return Value:   Pointer to format string


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        errnum    ;/* I   Number of error message to search for. */
#endif

/*!
Description:

`comerr_read` reads a format string from the COMTERP error file, which can be 
used to build an error message using `sprintf`.


See Also:  comerr_set, comerr_get, comerr_chk, err_read
!*/

#undef TITLE
#define TITLE "comerr_read"

{
/* Initialize if necessary */
   if( ErrorId == -1 )
      ErrorId = err_open( "comterp.err" );

/* Retrieve string */
   return err_read( ErrorId, errnum );
 
}





/*!

comerr_set      Submit COMTERP error to error system


Summary:

#include <ComUtil/comutil.h>
*/

void comerr_set( unsigned errnum, unsigned errlen )


/*!
Return Value:   none


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        errnum    ;/* I   Number of error message being submitted. */
unsigned        errlen    ;/* I   Length of error message string just
				  printed via `fprintf` to file pointer
				  returned from `err_fileio`. */
#endif

/*!
Description:

`comerr_set` submits an COMTERP error to the error system.  `errnum` is the
error number associated with the error.  `errlen` is the length of
the error message that was just printed via `fprintf` to the file
pointer returned by `err_fileio`.


See Also:  comerr_read, comerr_get, comerr_chk, err_set
!*/

#undef TITLE
#define TITLE "comerr_set"

{
/* Initialize if necessary */
   if( ErrorId == -1 )
      ErrorId = err_open( "comterp.err" );

/* Retrieve string */
   return err_set( ErrorId, errnum, errlen );

}







/*! 

comerr_get	Get last error if from COMTERP


Summary:

#include <ComUtil/comutil.h>
*/

int comerr_get()


/*!
Return Value:  error number if last error was from COMTERP, 
	       otherwise 0


Parameters:  none


/*!
Description:

`comerr_get` returns the number of the most recent error to be submitted
to the error system, if it was from COMTERP.


See Also:  comerr_read, comerr_set, comerr_chk, err_get
!*/

#undef TITLE
#define TITLE "comerr_get"

{
unsigned int get_errnum;
int get_errid;

/* If non-initialized, answer must be 0 */
   if( ErrorId == -1 )
      return 0;

/* Retrieve error number */
   err_get( &get_errid, &get_errnum );
   if( get_errid != ErrorId )
      return 0;
   else 
      return get_errnum;

}






/*! 

comerr_chk	Check if specific COMTERP error just occurred


Summary:

#include <ComUtil/comutil.h>
*/

BOOLEAN comerr_chk( unsigned errnum )


/*!
Return Value:  `TRUE` if last error was from COMTERP, and equal to `errnum`,
	       otherwise `FALSE`


Parameters: 

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        errnum    ;/* I   Error number to check for */
#endif

/*!
Description:

`comerr_chk` checks if a specific COMTERP error just occurred, meaning that
it would be the topmost error within the error system.


See Also:  comerr_read, comerr_set, comerr_get, err_get
!*/

#undef TITLE
#define TITLE "comerr_chk"

{
unsigned int get_errnum;
int get_errid;

/* If non-initialized, answer must be FALSE */
   if( ErrorId == -1 )
      return FALSE;

/* Retrieve error number */
   err_get( &get_errid, &get_errnum );
   if( get_errid != ErrorId )
      return FALSE;
   else if( get_errnum == errnum )
      return TRUE;
   else
      return FALSE;

}

