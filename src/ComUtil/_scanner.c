/*
 * Copyright (c) 2000 IET Inc.
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
scanner.c       COMTERP scanning routine

Externals:      int scanner()

Summary:        Scanning routine(s) developed to do the low-level scanning
                of textual input for the COMTERP language.

History:        Written by Scott E. Johnston, March 1989
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "comterp.ci"

int _angle_brackets = 0;

/*!

scanner   Return next token from COMTERP input text stream


Summary:

#include <comterp/comterp.h>
*/

int scanner(void * infile,char * (*infunc)(),int (*eoffunc)(), 
	    int (*errfunc)(),FILE * outfile,int (*outfunc)(),
	    char * buffer,unsigned bufsiz,unsigned * bufptr,
	    char * token,unsigned toksiz,unsigned * toklen,
	    unsigned * toktype,unsigned * tokstart,unsigned * linenum)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void *          infile    ;/* I   Pointer to input text source.  Typically
                                  a `FILE *`. */
char *        (*infunc)() ;/* I   Function for reading input text source
                                  (typically `fgets`). */
int           (*eoffunc)();/* I   Function for checking end-of-file condition
                                  (typically `ffeof`). */
int           (*errfunc)();/* I   Function for checking for error on file I/O
                                  (typically `fferror`). */
FILE *          outfile   ;/* I   Output file pointer for source listing
                                  (disabled with a NULL).      */
int           (*outfunc)();/* I   Function for writing output text
                                  (typically `fputs`). */
char *          buffer    ;/* I   Buffer or file I/O.          */
unsigned        bufsiz    ;/* I   Size of `buffer` in bytes.   */
unsigned *      bufptr    ;/* O   Current location in `buffer`.*/
char *          token     ;/* O   Token string.                */
unsigned        toksiz    ;/* I   Maximum possible token size. */
unsigned *      toklen    ;/* O   Actual token size.           */
unsigned *      toktype   ;/* O   Token type.                  */
unsigned *      tokstart  ;/* O   Starting offset of token in `buffer`. */
unsigned *      linenum   ;/* IO  Current line number of input file
                                  (initialize by setting to zero). */
#endif


/*!
Description:

`scanner` scans the input file (pointed to by `infile`) until it completes
a token and returns its value.  The possible token types are as follows
(defined in "comterp/comterp.h"):

        TOK_NONE        No token found (error occurred)
        TOK_LPAREN      Left parenthesis
        TOK_RPAREN      Right parenthesis
        TOK_LBRACKET    Left bracket
        TOK_RBRACKET    Right bracket
        TOK_LBRACE      Left brace
        TOK_RBRACE      Right brace
        TOK_LANGBRACK   Left angle bracket
        TOK_RANGBRACK   Right angle bracket
        TOK_KEYWORD     Keyword for free format parameters, i.e. ":SIZE"
        TOK_IDENTIFIER  Identifier, i.e. command name
        TOK_OPERATOR    Operator
        TOK_STRING      Character string constant
        TOK_CHAR        Character constant
        TOK_DFINT       Default integer, i.e. of type "int"
        TOK_DFUNS       Default unsigned integer, i.e. of type "unsigned"
        TOK_LNINT       Long integer, i.e. of type "long"
        TOK_LNUNS       Long unsigned integer, i.e. of type "long unsigned"
        TOK_DOUBLE      Double-size floating point number, i.e. of type "double"
        TOK_EOF         End of file

`infunc`, `eoffunc`, and `errfunc` are a set of functions that allow `scanner`
to access new lines of text as required.  If `infunc` is `NULL`, scanning is
done on the contents of `buffer` only.  In this case the contents of `buffer`
must terminate with a '\n'.

See Also:  lexscan

!*/

{

/* Constants for state machine to search for format string */
#define LOOK_START      0       /* Looking for start of comment or keyword */
#define LOOK_KEYWORD    1       /* Looking for keyword body */
#define LOOK_DONE       2       /* No more looking */

int search_state = LOOK_START; /* State of what has been found */
                                      /* in error file */
int status;

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* Loop through tokens and return one at a time                            */
/* ----------------------------------------------------------------------- */

   do {

   /* Use lexical scanner to provide next token in input file */
   /* Don't worry about status return, because token_type     */
   /* will be set to TOK_NONE                                 */
      status = lexscan( infile, infunc, eoffunc, errfunc, outfile, outfunc,
                        "/*", "*/", '#', buffer, bufsiz, bufptr,
                        token, toksiz, toklen, toktype,
                        tokstart, linenum );
      if( status != 0 )
         goto end;

   /* State machine that searches for the right format string */
      switch ( search_state ) {

      case LOOK_START:
         if( *toktype != TOK_OPERATOR )
            search_state = LOOK_DONE;
         else
	    switch( token[0] ) {

            case '(' :
               *toktype = TOK_LPAREN;
               search_state = LOOK_DONE;
               break;

            case ')' :
               *toktype = TOK_RPAREN;
               search_state = LOOK_DONE;
               break;

            case '[' :
               *toktype = TOK_LBRACKET;
               search_state = LOOK_DONE;
               break;

            case ']' :
               *toktype = TOK_RBRACKET;
               search_state = LOOK_DONE;
               break;

            case '{' :
               *toktype = TOK_LBRACE;
               search_state = LOOK_DONE;
               break;

            case '}' :
               *toktype = TOK_RBRACE;
               search_state = LOOK_DONE;
               break;

            case '<' :
	       if (_angle_brackets) {
		   if (buffer[*bufptr]=='<') {
		      (*bufptr)++;
                      *toktype = TOK_LANGBRACK2;
		      }
                   else
		      *toktype = TOK_LANGBRACK;
	           }
               search_state = LOOK_DONE;
               break;

            case '>' :
	       if (_angle_brackets) {
		   if (buffer[*bufptr]=='>') {
		      (*bufptr)++;
                      *toktype = TOK_RANGBRACK2;
		      }
                   else
		      *toktype = TOK_RANGBRACK;
	           }
               search_state = LOOK_DONE;
               break;

            case ':' :
               if( isident( buffer[*bufptr] ))
                  search_state = LOOK_KEYWORD;
               else
                  search_state = LOOK_DONE;
               break;

            default:
               search_state = LOOK_DONE;
               break;
               }
         break;

      case LOOK_KEYWORD:
         if( *toktype == TOK_IDENTIFIER ) {
	    *toktype = TOK_KEYWORD;
	    search_state = LOOK_DONE;
            }
         else {
            status = ERR_BADKEYWORD;
            goto end;
            }
         break;

         }
      }
   while( search_state != LOOK_DONE );

/* ----------------------------------------------------------------------- */
/* Done looping until next token is finished                               */
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */

   return FUNCOK;

/* ----------------------------------------------------------------------- */
/* Handle error if one occurred and return                                 */
/* ----------------------------------------------------------------------- */
end:

   switch ( status ) {
         case ERR_EOFCOMMENT:
         case ERR_EOFSTRING:
         case ERR_BADKEYWORD:
         case ERR_BADCHAR:
         case ERR_NOTOCTDIGIT:
         case ERR_NOTHEXDIGIT:
         case ERR_BADOCTCHAR:
         case ERR_BADHEXCHAR:
         case ERR_BADINT:
         case ERR_BADOCT:
         case ERR_BADHEX:
         case ERR_BADFLOAT:
         case ERR_TOOBIGINT:
         case ERR_TOOBIGOCT:
         case ERR_TOOBIGHEX:
         case ERR_TOOBIGFLOAT:
         case ERR_TOOSMLFLOAT:
         case ERR_NLINSTRING:
         case ERR_NLINCHAR:
         case ERR_CONSTSEP:
         case ERR_OUTFILE:
         case ERR_INFILE:
         case ERR_EOFNEWLINE:
            COMERR_SET1( status, *linenum );
            break;

         case ERR_LINELENGTH:
            COMERR_SET2( status, *linenum, bufsiz-2 );
            break;

         case ERR_TOKENLENGTH:
            COMERR_SET2( status, *linenum, toksiz-1 );
            break;

         case ERR_ILLEGALCHAR:
            COMERR_SET2( status, *linenum, buffer[*tokstart] );
            break;

         default:
            break;
            }

   return FUNCBAD;
}



