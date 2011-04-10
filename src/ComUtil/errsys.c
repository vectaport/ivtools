/*
 * Copyright (c) 2001 Scott E. Johnston
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
errsys.c        General error handling support

Externals:      int err_open(), char * err_read(),
		void err_set(), void err_get(),
		void err_print(), void err_str(),
		void err_clear(), void err_level(), 
		FILE *err_fileio(), int err_cnt()

History:        Written by Scott E. Johnston, March 1989

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "comutil.ci"

/* Manifest Constants */
#define MAX_ERROR_OPENS         16      /* Maximum number of err_open calls */
#define MAX_ERROR_MESSAGE       512     /* Room for error messages */
#define MAX_ERROR_SETS          16      /* Maximum depth of error messages */
#define ERROR_IO_FILE ".errsys.comterp" /* Path name of error I/O file */

/* Local Statics */
static FILE *ErrorStreams[MAX_ERROR_OPENS];     /* Buffer for error file ptrs */

static char ErrorMessages[MAX_ERROR_MESSAGE];   /* Buffer for error messages */

static struct {                                 /* Buffer for error structs  */
   unsigned errid;
   unsigned errnum;
   unsigned erroff;
   unsigned errlen;
   } ErrorStructs[MAX_ERROR_SETS];

static int Initialize = TRUE;           /* Triggers error system initialization */
static int TopError = -1;               /* Topmost error on stack */
static int NextErrOff = 0;              /* Next place to store error string */
static BOOLEAN TooManyErrors = FALSE;   /* TRUE when ErrorStreams overflows */
static int ErrorLevel = USER_LEVEL;     /* 0 = user level        */
					/* 1 = programmers level */
static FILE *ErrorIOFile = NULL;        /* Pointer to error I/O file */



/*! 

err_open    Open access to error file through error system


Summary:

#include <ComUtil/comutil.h>
*/

int err_open(char * errfile )


/*!
Return Value:   unique identifier supplied to other error system calls,
		-1 if error


Parameters: 

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char *          errfile   ;/* I   Filename of error file. */
#endif


/*!
Description:

`err_open` opens a specific error file, then returns a unique identifier 
to be supplied to other error system calls. 


See Also:  err_read, err_set, err_get, err_print, err_str, err_clear, 
	   err_level, err_fileio, err_cnt
!*/

#undef TITLE
#define TITLE "err_open"

{
    char fullpath[PATH_LEN];
    int findex;
    char *errpath;
    FILE *fptr = NULL;
    
    /* Initialize array of pointers to open error files */
    if( Initialize ) {
	for( findex=0; findex<MAX_ERROR_OPENS; findex++ )
	    ErrorStreams[findex] = NULL;
	Initialize = FALSE;
    }
    
    /* Search for NULL id */
    for( findex=0; findex<MAX_ERROR_OPENS; findex++ )
	if( ErrorStreams[findex] == NULL ) 
	    break;
    if( findex == MAX_ERROR_OPENS )
	KAPUT1( "Exceeded maximum number of opened error files (%d)", 
		MAX_ERROR_OPENS );
    
    /* Attempt to open error file */
    errpath = (char*)getenv( "COMTERP_PATH" );
    if (errpath) {
	strcpy( fullpath, errpath );
	if (fullpath[strlen(fullpath)-1] != '/') strcat( fullpath, "/" );
	strcat( fullpath, errfile );
	fptr = fopen(fullpath, "r");
    }

    if (!fptr) {
	strcpy( fullpath, RELLIBALLDIR );
	if (fullpath[strlen(fullpath)-1] != '/') strcat( fullpath, "/" );
	strcat( fullpath, errfile );
	fptr = fopen(fullpath, "r");
    }
    
    if (!fptr) {
	strcpy( fullpath, ABSLIBALLDIR );
	if (fullpath[strlen(fullpath)-1] != '/') strcat( fullpath, "/" );
	strcat( fullpath, errfile );
	fptr = fopen(fullpath, "r");
    }
   
   if(( ErrorStreams[findex] = fptr) == NULL )
      KAPUT1( "Unable to open error file %s", fullpath );

   return findex;

}




/*! 

err_read    Read error format string from file


Summary:

#include <ComUtil/comutil.h>
*/

char * err_read(int errid,unsigned errnum)


/*!
Return Value:   Pointer to format string. 



Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             errid     ;/* I   Identifier generated by `err_open`. */
unsigned        errnum    ;/* I   Number of format string to search for. */
#endif


/*!
Description:

`err_read` reads a format string from an error file that is then used to
build a final error message.  See `err_readfile` for further details on
the file format.

Typically the pointer to the format string returned by `err_read` is supplied 
to `sprintf` for its format string argument, in order to build the final 
error message, then the result is passed to `err_set`, placing it on a stack 
of error messages that can be displayed to the user via `err_print and err_str`.


See Also:  err_readfile, err_open, err_set, err_get, err_print, err_str,
	   err_clear, err_level, err_fileio, err_cnt

!*/

{
/* Error checking */
   if( errid < 0 || errid >= MAX_ERROR_OPENS || ErrorStreams[errid] == NULL )
      KANIL( "errid out of bounds" );

/* Retrieve and return the string */
   return err_readfile( ErrorStreams[errid], errnum );
}







/*! 

err_set    Submit error to error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_set(int errid,unsigned errnum,unsigned errlen)


/*!
Return Value:  no error return


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             errid     ;/* I   Identifier generated by `err_open`. */
unsigned        errnum    ;/* I   Error number */
unsigned        errlen    ;/* I   Length of error message string just
				  printed by `fprintf` to file pointer
				  returned from `err_fileio`. */
#endif


/*!
Description:

`err_set` submits an error to the error system.  `errnum` is the
error number associated with the error.  `errlen` is the length of the string
just printed by `fprintf` to a file pointer returned from `err_fileio`.  
`errid` was returned from a previous call to `err_open`.


See Also:  err_open, err_read, err_get, err_print, err_str, err_clear, 
	   err_level, err_fileio, err_cnt

!*/

{

/* Error checking */
   if( errid < 0 || errid >= MAX_ERROR_OPENS || ErrorStreams[errid] == NULL ) {
      KANRET( "errid out of bounds" );
      return;
      }

/* Add errid and errnum to stack, if they fit */
   if( TopError+1 < MAX_ERROR_SETS ) {
      TopError++;
      ErrorStructs[TopError].errid = errid;
      ErrorStructs[TopError].errnum = errnum;
      ErrorStructs[TopError].erroff = NextErrOff;
      ErrorStructs[TopError].errlen = errlen;
      NextErrOff += errlen;
      }
   else
      TooManyErrors = TRUE;

}



/*!

err_get   Retrieve last error submitted to error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_get(int * errid,unsigned * errnum )


/*!
Return Value:  no error return


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int *           errid     ;/* I   Identifier generated by `err_open`. */
unsigned *      errnum    ;/* I   Positive error number, that should
				  have a unique association with `errid`. */
#endif


/*!
Description:

`err_get` retrieves information about the last error submitted to the
error system.  `errid` is the identifier used when `err_get` was called.
`errnum` is the error number associated with the error.


See Also:  err_open, err_read, err_set, err_print, err_str, err_clear, 
           err_level, err_fileio, err_cnt

!*/

{
/* Check if any errors are present */
   if( TopError == -1 ) {
      *errid = -1;
      return;
      }

/* Return errid and errnum */
   *errid = ErrorStructs[TopError].errid;
   *errnum = ErrorStructs[TopError].errnum;
   return;

}



/*! 

err_print   Print errors in error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_print(FILE * outstream,char * command)


/*!
Return Value:  no error return


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *          outstream ;/* I   File pointer for output of errors.  */
char *          command   ;/* I   Name of command where error occurred */
#endif


/*!
Description:

`err_print` prints the current error message(s) to `outstream`.  If
`user_level` is false, every error message on the stack gets printed.  If 
`user_level` is true, only the topmost error message gets printed, and
the `command` name gets substituted for any function name at the front of the
error message (function names are found by looking for a legal function
name followed by a colon and at least one space).


See Also:  err_open, err_read, err_set, err_get, err_clear, err_level,
	   err_fileio, err_cnt

!*/

{
int index;
char *ptr;
char buffer[BUFSIZ];

/* Abort if no errors have occurred */
   if( TopError == -1 )
       return;

/* Rewind error I/O file */
   if( ErrorIOFile == NULL )
      return;
   rewind( ErrorIOFile );

/* Print overflow messages */
   if( TooManyErrors ) {
      fprintf( outstream, "*** Warning:  Error depth greater than %d ***\n", 
               MAX_ERROR_SETS );
      fprintf( outstream, "     *** Unable to print all errors ***\n" );
      }

/* If programmer level error reporting dump the entire stack */
   if( ErrorLevel == PROG_LEVEL ) {
      for( index=TopError; index>=0; index-- ) {
	 fseek( ErrorIOFile, (long)ErrorStructs[index].erroff, SEEK_SET );
	 fgets( buffer,
		min( BUFSIZ, ErrorStructs[index].errlen+1),
		ErrorIOFile );
	 fprintf( outstream, "%s\n", buffer );
	 }
      fprintf( outstream, "%s:  Error in execution\n", command );
      }

/* Else if user level error reporting, print the topmost error */
/* with command substituted for the function name              */
   else {
      fseek( ErrorIOFile, (long)ErrorStructs[TopError].erroff, SEEK_SET );
      fgets( buffer, min( BUFSIZ, ErrorStructs[TopError].errlen+1),
	     ErrorIOFile );
      ptr = buffer;
      if( isident( *ptr ))
	 ++ptr;
      while( isident( *ptr ) || isdigit( *ptr ))
	 ++ptr;
      if( *ptr == ':' ) {
	 ptr++;
	 while( isspace( *ptr )) ptr++;
	 }
      else
	 ptr = buffer;
      fprintf( outstream, "%s:  %s\n", command, ptr );
      }

/* Clear the error messages */
   err_clear();

   return;

}




/*! 

err_str   return the errors as strings from the error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_str(char * errbuf,int bufsiz,char * command)


/*!
Return Value:  no error return


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char *		errbuf    ;/* O   Buffer to store formated error string */  
int		bufsiz	  ;/* I   size of errbuf */
char *          command   ;/* I   Name of command where error occurred */
#endif


/*!
Description:

`err_str` formats the current error message(s) to `errbuf`.  If
`user_level` is false, every error message on the stack gets formatted.  If 
`user_level` is true, only the topmost error message gets formatted, and
the `command` name gets substituted for any function name at the front of the
error message (function names are found by looking for a legal function
name followed by a colon and at least one space).


See Also:  err_open, err_read, err_set, err_get, err_print, err_clear, 
           err_level, err_fileio, err_cnt

!*/

{
int index;
char *ptr;
char buffer[bufsiz];

/* Abort if no errors have occurred */
   if( TopError == -1 )
       return;

/* Rewind error I/O file */
   if( ErrorIOFile == NULL )
      return;
   rewind( ErrorIOFile );

/* Print overflow messages */
   if( TooManyErrors ) {
      sprintf( errbuf, "*** Warning:  Error depth greater than %d ***\n", 
               MAX_ERROR_SETS );
      sprintf( errbuf, "     *** Unable to print all errors ***\n" );
      }
#if 0
/* If programmer level error reporting dump the entire stack */
   if( ErrorLevel == PROG_LEVEL ) {
      for( index=TopError; index>=0; index-- ) {
	 fseek( ErrorIOFile, (long)ErrorStructs[index].erroff, SEEK_SET );
	 fgets( buffer,
		min( BUFSIZ, ErrorStructs[index].errlen+1),
		ErrorIOFile );
	 fprintf( outstream, "%s\n", buffer );
	 }
      fprintf( outstream, "%s:  Error in execution\n", command );
      }

/* Else if user level error reporting, print the topmost error */
/* with command substituted for the function name              */
   else {
#endif
      fseek( ErrorIOFile, (long)ErrorStructs[TopError].erroff, SEEK_SET );
      fgets( buffer, min( BUFSIZ, ErrorStructs[TopError].errlen+1),
	     ErrorIOFile );
      ptr = buffer;
      if( isident( *ptr ))
	 ++ptr;
      while( isident( *ptr ) || isdigit( *ptr ))
	 ++ptr;
      if( *ptr == ':' ) {
	 ptr++;
	 while( isspace( *ptr )) ptr++;
	 }
      else
	 ptr = buffer;
      sprintf( errbuf, "%s:  %s", command, ptr );
#if 0
      }
#endif

/* Clear the error messages */
   err_clear();

   return;

}




/*! 

err_clear   Clear all errors in error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_clear( )


/*!
Return Value:  no error return


Parameters:  none


/*!
Description:

`err_clear` clears all errors in the error system, restoring
it to an initial state.


See Also:  err_open, err_read, err_set, err_get, err_print, err_str,
	   err_level, err_fileio, err_cnt

!*/

{

/* Reset error stack */
   TopError = -1;
   NextErrOff = 0;
   TooManyErrors = FALSE;
   fclose( ErrorIOFile );
   ErrorIOFile = NULL;
#if 0
   unlink( ERROR_IO_FILE );
#endif
   return;

}






/*! 

err_level   Set level of error printing by error system


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

void err_level(unsigned level)


/*!
Return Value:  no error return


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        level     ;/* I   USER_LEVEL or PROG_LEVEL */
#endif


/*!
Description:

`err_level` sets the level of error printing performed by the error system
when `err_print` or `err_str` is called.  Two values can currently be 
supplied to `err_level` (defined in "ComUtil/comutil.h"):  

	USER_LEVEL	Output only topmost error message
	PROG_LEVEL	Output entire stack of error messages


See Also:  err_open, err_read, err_set, err_get, err_print, err_str, 
	   err_clear, err_fileio, err_cnt

!*/

{
   ErrorLevel = level;
   return;

}






/*! 

err_fileio   Return pointer to file for error I/O


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

FILE *err_fileio()


/*!
Return Value:  pointer to error I/O file


Parameters:  none


/*!
Description:

`err_fileio` returns a pointer to the file used for error I/O.  In typical usage,
this would be supplied as the first argument to `fprintf`.  The second argument 
would be supplied by `err_read` (or its equivalent), and the rest of the 
arguments would be application specific.
 

See Also:  err_open, err_read, err_set, err_get, err_print, err_str, 
	   err_clear, err_level, err_cnt

!*/

{
#if 0
   char *errpath;
   if( ErrorIOFile == NULL ) {
      errpath = tmpnam(nil);
      ErrorIOFile = fopen( errpath, "w+" );
      if( ErrorIOFile == NULL )
	 KANIL1( "Unable to open error I/O file %s", errpath );
      }
   return ErrorIOFile;
#else
   if( ErrorIOFile == NULL ) {
      ErrorIOFile = tmpfile();
      if( ErrorIOFile == NULL )
	 KANIL( "Unable to open error I/O file" );
      }
   return ErrorIOFile;
#endif

}



/*! 

err_cnt   Return current count of error messages


Summary:

#include <nsfcfg.h>
#include <ComUtil/comutil.h>
*/

int err_cnt()


/*!
Return Value:  count of current error messages


Parameters:  none


/*!
Description:

`err_cnt` returns the number of error messages on the error message stack.
 

See Also:  err_open, err_read, err_set, err_get, err_print, err_str, 
	   err_clear, err_level, err_fileio

!*/

{
   return TopError+1;
}



