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
errfile.c       Retrieve error format strings from file

Externals:      int err_readfile()

History:        Written by Scott E. Johnston, March 1989
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "comutil.ci"

/* Manifest Constants */
#define MAX_FORMAT_LENGTH	80	/* Longest possible format string */
#define MAX_INPUT_LENGTH	132	/* Length of buffer for reading   */
					/* format string file             */
/* Local Statics */
static char FormatBuffer[MAX_FORMAT_LENGTH+1];	/* Buffer for format string */
static char InputBuffer[MAX_INPUT_LENGTH+2];	/* Buffer for input text    */


/*! 

err_readfile	Read error message format string from file


Summary:

#include <ComUtil/comutil.h>
*/

char * err_readfile(FILE* errstream,unsigned errnum )


/*!
Return Value:   Pointer to format string.  NULL is never returned,
		because a substitute format string is constructed
		if `errnum` is not found.


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *          errstream ;/* I   Error message format string file pointer. */
unsigned        errnum    ;/* I   Number of error message to search for. */
#endif


#ifdef DOC
/*!
Description:

`err_readfile` reads a format string which can be used to build an
error message.  `errstream` is a pointer to the file which contains
the format strings.  The `errstream` file is designed to serve a dual role, 
to define constants for various `errnum`s in a C program, and to store the
error message format strings for access once an error occurs.  An example 
entry in this file is as follows:

	#define ERR_OUTOFBOUNDS         12
	/* "process1:  Input size parameter out of bounds (%d)" */

`err_readfile` searches for definitions of macros that begin with "ERR_", then
reads the positive integer value that follows.  When the entry that matches 
`errnum` is found, a C-format string buried in the following comment is
extracted, and a pointer to this string is returned.  


See Also:  err_open, err_submit, err_match, err_print, err_clear


!*/
#endif /* DOC */


{
/* Constants for state machine to search for format string */
#define LOOK_POUND	0	/* Looking for start of "#define" */
#define LOOK_DEFINE 	1	/* Looking for "define" token next */
#define LOOK_ERRMACRO	2	/* Looking for "ERR_" macro next */
#define LOOK_DFINT	3	/* Looking for default integer next */
#define LOOK_SLASH	4	/* Looking for "/" to prestart comment */
#define LOOK_STAR	5	/* Looking for "*" to start comment */
#define LOOK_STRING	6	/* Looking for string in comment */
#define LOOK_DONE       7       /* Found everything needed */

int search_state = LOOK_POUND;  /* State of what has been found */
				/* in error file */
int token_type;                 /* Type of token returned from lexscan */
int linenum = 0;                /* Current line number in error file */
int colnum = 0;                 /* Current location in buffer */
int token_length;               /* Token length */
int token_start;                /* Start of token in FormatBuffer */
int status;                     /* Status from lexscan */


/* Rewind error file */
   rewind( errstream );

/* Search through tokens in error file until */
/* conditions are met, or end-of-file        */
   do {

   /* Use lexical scanner to provide next token in error file */
   /* Don't worry about status return, because token_type     */
   /* will be set to TOK_NONE                                 */
      status = lexscan( errstream, fgets, ffeof, fferror,
			NULL, NULL, NULL, NULL, 0,
			InputBuffer, MAX_INPUT_LENGTH+2, &colnum,
			FormatBuffer, MAX_FORMAT_LENGTH+1, &token_length,
			&token_type, &token_start, &linenum );

   /* State machine that searches for the right format string */
      switch ( search_state ) {

      case LOOK_POUND:
	 if( token_type == TOK_OPERATOR && FormatBuffer[0] == '#' &&
	     (token_start == 0 || isspace( InputBuffer[token_start-1] )))
	    search_state = LOOK_DEFINE;
	 break;
      case LOOK_DEFINE:
	 if( token_type == TOK_IDENTIFIER &&
	     strcmp( FormatBuffer, "define" ) == 0 &&
	     token_start > 0 && InputBuffer[token_start-1] == '#' )
	    search_state = LOOK_ERRMACRO;
	 else
	    search_state = LOOK_POUND;
	 break;
      case LOOK_ERRMACRO:
	 if( token_type == TOK_IDENTIFIER &&
	     strncmp( FormatBuffer, "ERR_", 4 ) == 0 )
	    search_state = LOOK_DFINT;
	 else
	    search_state = LOOK_POUND;
	 break;
      case LOOK_DFINT:
	 if( token_type == TOK_DFINT && *(int *)FormatBuffer == errnum )
	    search_state = LOOK_SLASH;
	 else
	    search_state = LOOK_POUND;
	 break;
      case LOOK_SLASH:
	 if( token_type == TOK_OPERATOR && FormatBuffer[0] == '/' )
	    search_state = LOOK_STAR;
	 else
	    search_state = LOOK_POUND;
	 break;
      case LOOK_STAR:
	 if( token_type == TOK_OPERATOR && FormatBuffer[0] == '*' &&
	     token_start > 0 && InputBuffer[token_start-1] == '/' )
	    search_state = LOOK_STRING;
	 else
	    search_state = LOOK_POUND;
	 break;
      case LOOK_STRING:
	 if( token_type == TOK_STRING )
	    search_state = LOOK_DONE;
	 else
	    search_state = LOOK_POUND;
	 break;
	 }
      }
   while( search_state != LOOK_DONE && status == 0 &&
	  token_type != TOK_EOF && !ferror( errstream ));
/* Done searching through tokens in error file */
/* until conditions are met, or end-of-file    */

/* Finalize format string (if necessary) and return pointer to it */
   if( status != 0 )
      sprintf( FormatBuffer, "Status %d returned from error system",
	 status );
   else if( token_type == TOK_EOF )
       sprintf( FormatBuffer, "Error number %d (no error message)",
		errnum );
   else if( ferror( errstream ))
       sprintf( FormatBuffer, "Error in accessing error message file" );
   return FormatBuffer;

}

