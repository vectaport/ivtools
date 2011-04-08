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
lexscan.c       Lexical scanner 

Externals:      int lexscan()

Summary:        Lexical scanning routine to recognize C-like tokens

History:        Written by Scott E. Johnston, March 1989
*/

extern int _continuation_prompt;
extern int _continuation_prompt_disabled;

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "comutil.ci"

/* MACROS */

/* States for parsing floating point numbers */
#define FLOAT_INTEGER	0
#define FLOAT_FRACTION	1
#define FLOAT_NEWEXPON  2
#define FLOAT_EXPONENT	3

#define FLOAT_IS_STARTING( ch1, ch2 ) \
(((ch1) == '.' && (ch2) != '.' ) || (ch1) == 'E' || (ch1) == 'e')

#define LOOKS_LIKE_LONG( ch1, ch2 ) \
(((ch1) == 'l' || (ch1) == 'L') && !(isdigit(ch2) || isident(ch2)))

#define ADVANCE_PAST_QUOTE \
while( CURR_CHAR != '\n' && \
CURR_CHAR != ( token_state == TOK_STRING ? '"' : '\'' )) \
ADVANCE_CHAR; ADVANCE_CHAR



/*!

lexscan   Lexical scanning routine to recognize C-like tokens


Summary:

#include <ComUtil/comutil.h>
*/

int lexscan(void * infile,char * (*infunc)(),int (*eoffunc)(), int (*errfunc)(), 
	    void * outfile,int (*outfunc)(),
	    char * begcmt,char * endcmt, char linecmt,
            char * buffer,unsigned bufsiz,unsigned * bufptr,char * token,
	    unsigned toksiz, unsigned * toklen,unsigned * toktype,
	    unsigned * tokstart,unsigned * linenum )


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
int 	      (*eoffunc)();/* I   Function for checking end-of-file condition
                                  (typically `ffeof`). */
int	      (*errfunc)();/* I   Function for checking for error on file I/O
				  (typically `fferror`). */
void *          outfile   ;/* I   Pointer to output text source.  Typically
			          a `FILE *` (disabled with a NULL).      */
int           (*outfunc)();/* I   Function for writing output text
				  (typically `fputs`). */
char *	        begcmt    ;/* I   String that begins comment. */
char *          endcmt    ;/* I   String that ends comment. */
char            linecmt   ;/* I   Character to start comment line.  */
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

`lexscan` scans an input text source, recognizing the following token types
(defined in "ComUtil/comutil.h"):

	TOK_NONE        No token found (error occurred)
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

`infunc`, `eoffunc`, and `errfunc` are a set of functions that allow `lexscan`
to access new lines of text as required.  If `infunc` is `NULL`, scanning is
done on the contents of `buffer` only.  In this case the contents of `buffer`
must terminate with a '\n'.


!*/

{
register char ch;		/* For efficient access of latest char */
unsigned double_state = FLOAT_INTEGER;
				/* Extra state variable for float parsing */
BOOLEAN long_num = FALSE;       /* Indicates long integer to be used */
unsigned token_state = TOK_WHITESPACE;
				/* Internal token state variable */
static unsigned token_state_save = TOK_WHITESPACE;
				/* variable to save token state between calls */
unsigned begcmt_len =           /* Number of characters in comment beginning */
   (begcmt != NULL ? strlen(begcmt) : 0 );
unsigned endcmt_len =           /* Number of characters in comment ending */
   (endcmt != NULL ? strlen(endcmt) : 0 );
unsigned no_comment =           /* TRUE if comments are disabled */
   (begcmt_len == 0 || endcmt_len == 0);
int index;
char* infunc_retval;


/* ----------------------------------------------------------------------- */
/* Initialize                                                              */
/* ----------------------------------------------------------------------- */

/* Safety check */
#ifdef SAFETY_FIRST
   if( bufsiz < 3 || toksiz < sizeof(double) )
      KAPUT( "Ridiculous buffer sizes" );
#endif

/* Initialize */
   *toktype = TOK_NONE;
   *toklen = 0;
   *tokstart = 0;
   token_state = token_state_save;
   token_state_save = TOK_WHITESPACE;

/* Initialize if linenumber is 0 */
   if( *linenum == 0 ) {
      *bufptr = 0;
      if( infunc != NULL )
	 buffer[0] = '\0';
      else {
	 *linenum = 1;
#if 0
	 if( outfile != NULL )
	    if( (*outfunc)( buffer, outfile ) != 0 )
	       return ERR_OUTFILE;
#endif
         }
      }
   CURR_CHAR = buffer[*bufptr];

/* Check if end-of-file condition is true */
   if( (infunc != NULL && (*eoffunc)( infile )) ||
       (infunc == NULL && CURR_CHAR == '\0' )) {
      *toktype = TOK_EOF;
      return FUNCOK;
      }

/* Check if error occurred previously */
   if( infunc != NULL && (*errfunc)( infile )) {
      *toktype = TOK_NONE;
      return ERR_INFILE;
      }


/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* Loop until next token is finished                                       */
/* ----------------------------------------------------------------------- */

   while( TRUE ) {


   /* ----------------- GET NEXT CHARACTER -------------------- */

   /* Get New Line of Text */
      if( CURR_CHAR == '\0' ) {

      /* If infunc is NULL, emulate end-of-file */
	 if( infunc == NULL ) {
	    *toktype = TOK_EOF;
	    if( token_state == TOK_COMMENT )
	       return ERR_EOFCOMMENT;
	    if( token_state == TOK_STRING || token_state == TOK_CHAR)
	       return ERR_EOFSTRING;
	    return FUNCOK;
	    }

      /* Clear next to last byte for "line too long" check */
	 buffer[bufsiz-2] = '\0';

      /* If error returned from infunc, return current token as is */
	 if( token_state != TOK_STRING && token_state != TOK_CHAR )
	    *tokstart = 0;
	 if (_continuation_prompt && outfunc) {
	   if (!_continuation_prompt_disabled) 
	     (*outfunc) ( "> ", outfile);
	   _continuation_prompt = 0;
	 }
	 if (linecmt)
	   while( (infunc_retval = (*infunc)( buffer, bufsiz, infile )) != NULL && 
		  buffer[0] == linecmt) {} /* skip all script comments */
	 else
	   infunc_retval = (*infunc)( buffer, bufsiz, infile );
	 if( infunc_retval == NULL ) {
	    if( *toklen > 0 )
	       goto token_return;
	    else {
	       if( (*eoffunc)( infile )) {
		  *toktype = TOK_EOF;
		  if( token_state == TOK_COMMENT )
		     return ERR_EOFCOMMENT;
		  if( token_state == TOK_STRING || token_state == TOK_CHAR )
		     return ERR_EOFSTRING;
		  return FUNCOK;
		  }


	    /* File error check */
	       else if( (*errfunc)( infile )) {
		  *toktype = TOK_NONE;
		  return ERR_INFILE;
		  }
	       }
	}

      /* If end-of-file was encountered, yet a legimitate read */
      /* occurred, warn the user that last line in file does   */
      /* not end with new-line char                            */
	 ++*linenum;
         if( (*eoffunc)( infile ))
            return ERR_EOFNEWLINE;

      /* In order to detect if line is too long for buffer */
      /* the next to last byte was nulled.  If after the   */
      /* call to infunc this byte is anything but '\n' the  */
      /* line of input was too long for the buffer.        */
	 if( buffer[bufsiz-2] != '\0' && buffer[bufsiz-2] != '\n' ) {

	 /* Get the rest of the line and toss it */
	    while( buffer[bufsiz-2] != '\0' && buffer[bufsiz-2] != '\n' ) {
	       buffer[bufsiz-2] = '\0';
	       if (_continuation_prompt && outfunc) {
		 if (!_continuation_prompt_disabled) 
		   (*outfunc) ( "> ", outfile);
		 _continuation_prompt = 0;
	       }
	       (*infunc)( buffer, bufsiz, infile );
	       }

	    *bufptr = 0;
	    buffer[0] = '\0';

	    return ERR_LINELENGTH;
	    }

      /* Reset pointer to front of buffer */
	 *bufptr = 0;
         CURR_CHAR = buffer[0];
	 if (CURR_CHAR == '\0') {
	   if (token_state == TOK_COMMENT)
	     token_state_save = token_state;
	    *linenum--;
	    return  FUNCOK;
	 }

      /* Echo source line if so desired */
#if 0
	 if( outfile != NULL )
	    if( (*outfunc)( buffer, outfile ) == EOF )
	       return ERR_OUTFILE;
#endif
	 }



   /* -------------------- TOKEN STATE SWITCH ------------------ */

      switch ( token_state ) {


      /*-WHITESPACE-WHITESPACE-WHITESPACE-WHITESPACE-WHITESPACE-WHITESPACE-*/

      case TOK_WHITESPACE:     

      /* This could be the start of something */
	 *tokstart = *bufptr;

      /* Still whitespace */
	 if( isspace( CURR_CHAR )) {
	    }

      /* Start of comment */
	 else if( !no_comment &&
	    strncmp( begcmt, buffer+*bufptr, begcmt_len ) == 0 ) {
	    token_state = TOK_COMMENT;
	    for( index=1; index<begcmt_len; index++)
	       ADVANCE_CHAR;
            }

      /* empty input buffer */
	 else if ( CURR_CHAR == '\0') {
	    *linenum--;
	    return  FUNCOK;
	    }

      /* Illegal characters */
	 else if( !isprint( CURR_CHAR )) {
            ADVANCE_CHAR;
	    return ERR_ILLEGALCHAR;
            }

      /* Start of identifier */
	 else if( isident( CURR_CHAR )) {
	    token_state = TOK_IDENTIFIER;
	    TOKEN_ADD( CURR_CHAR );
	    }

      /* Potential start of octal or hexadecimal number */
	 else if( CURR_CHAR == '0' ) {
	    if( NEXT_CHAR == 'x' || NEXT_CHAR == 'X' ) {
	       token_state = TOK_HEX;
	       ADVANCE_CHAR;
	       }
	    else
	       token_state = TOK_OCT;
	    }

      /* Potential start of int */
	 else if( isdigit( CURR_CHAR )) {
	    token_state = TOK_DFINT;
	    TOKEN_ADD( CURR_CHAR );
	    }

      /* Floating point that starts with decimal */
	 else if( CURR_CHAR == '.' && isdigit( NEXT_CHAR ) &&
		( *bufptr == 0 || (!isalpha( PREV_CHAR ) && !isdigit( PREV_CHAR)))) {
	    token_state = TOK_DOUBLE;
	    TOKEN_ADD( '.' );
	    }

      /* Other possibilities */
	 else {
	    switch( CURR_CHAR ) {

	 /* Start of string constant */
	    case '"':
	       token_state = TOK_STRING;
	       break;

	 /* Start of character constant */
	    case '\'':
	       token_state = TOK_CHAR;
	       break;

         /* Any other character must be an operator */	 
            default:
               token_state = TOK_OPERATOR;
               TOKEN_ADD( CURR_CHAR );
               ADVANCE_CHAR;
               goto token_return;
               }
            }


         break; /* end of TOK_WHITESPACE case */


      /*-COMMENT-COMMENT-COMMENT-COMMENT-COMMENT-COMMENT-COMMENT-COMMENT-*/

      case TOK_COMMENT:
	 if( strncmp( endcmt, buffer+*bufptr, endcmt_len ) == 0 ) {
	    token_state = TOK_WHITESPACE;
	    for( index=1; index<endcmt_len; index++)
               ADVANCE_CHAR;
            }
         break; /* end of TOK_COMMENT case */


      /*-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-*/

      case TOK_IDENTIFIER:
	 if( isident( CURR_CHAR ) || isdigit( CURR_CHAR ))
	    TOKEN_ADD( CURR_CHAR )
         else 
            goto token_return;
	 break; /* end of TOK_KEYWORD and TOK_IDENTIFIER case */


      /*-STRING-STRING-STRING-STRING-STRING-STRING-STRING-STRING-STRING-*/
      /*-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-CHAR-*/

      case TOK_STRING:          /* Character string constant */
      case TOK_CHAR:            /* Character constant */

      /* End of string */
	 if( CURR_CHAR == '"' && token_state == TOK_STRING ) {
            ADVANCE_CHAR;
            goto token_return;
	    }

      /* Check for an unexpected new-line character */
	 if( CURR_CHAR == '\n') 
            if( *toktype == TOK_STRING )
	       return ERR_NLINSTRING;
	    else
	       return ERR_NLINCHAR;


      /* Normal character added to string or character constant */
	 else if( CURR_CHAR != '\\' )
	    TOKEN_ADD( CURR_CHAR )

      /* Backslash encountered, which can mean many things */
	 else if( CURR_CHAR == '\\' ) {

	 /* Line continuation, for both strings and characters */
	   if( NEXT_CHAR == '\n' ) {
	       ADVANCE_CHAR;
	       _continuation_prompt = 1;
	   } else if( NEXT_CHAR == '\r') {
	       ADVANCE_CHAR;
	       if ( NEXT_CHAR == '\n') 
		  ADVANCE_CHAR;
	       _continuation_prompt = 1;
	   } 

	 /* Octal digit indicates ASCII character in octal notation */
	    else if( isodigit( NEXT_CHAR )) {
	       unsigned char octval;
	       octval = OCTVAL( NEXT_CHAR );
	       ADVANCE_CHAR;
	       if( isodigit( NEXT_CHAR )) {
		  octval <<= 3;
		  octval |= OCTVAL( NEXT_CHAR );
		  ADVANCE_CHAR;
		  if( isodigit( NEXT_CHAR )) {
		     if( octval < 0x20 ) {
			octval <<= 3;
			octval |= OCTVAL( NEXT_CHAR );
			ADVANCE_CHAR;
			}
		     else {
			ADVANCE_PAST_QUOTE;
			return ERR_BADOCTCHAR;
                        }
		     }
		  }
	       TOKEN_ADD( octval );
	       }

	 /* 'x' indicates ASCII character in hexadecimal notation */
	    else if( NEXT_CHAR == 'x' ) {
	       unsigned char hexval;
	       ADVANCE_CHAR;
	       if( !isxdigit( NEXT_CHAR )) {
                  ADVANCE_PAST_QUOTE;
		  return ERR_NOTHEXDIGIT;
                  }
	       hexval= HEXVAL( NEXT_CHAR );
	       ADVANCE_CHAR;
	       if( isxdigit( NEXT_CHAR )) {
		  hexval <<= 4;
		  hexval |= HEXVAL( NEXT_CHAR );
		  ADVANCE_CHAR;
		  if( isxdigit( NEXT_CHAR )) {
		     if( hexval < 0x10 ) {
			hexval <<= 4;
			hexval |= HEXVAL( NEXT_CHAR );
			ADVANCE_CHAR;
			}
		     else {
			ADVANCE_PAST_QUOTE;
			return ERR_BADHEXCHAR;
                        }
		     }
		  }
	       TOKEN_ADD( hexval );
               }

	 /* The rest of the valid escape sequences.           */
         /* '\n' and '\r' are left to the compiler to resolve */
         /* in order to be system independent.                */
            else {
               switch( NEXT_CHAR ) {
	       case 'a':
		  TOKEN_ADD( '\007' );
                  break;
               case 'b':
                  TOKEN_ADD( '\010' );
                  break;
               case 't':
                  TOKEN_ADD( '\011' );
		  break;
	       case 'n':
                  TOKEN_ADD( '\n' );
                  break;
               case 'v':
		  TOKEN_ADD( '\013' );
                  break;
               case 'f':
                  TOKEN_ADD( '\014' );
                  break;
               case 'r':
                  TOKEN_ADD( '\r' );
                  break;
               default:
		  TOKEN_ADD( NEXT_CHAR );
		  break;
		  }
	       ADVANCE_CHAR;
	       }
	    }

      /* If character token, single-quote should be here */
	 if( token_state == TOK_CHAR && *toklen > 0 )
	    if( NEXT_CHAR == '\'' ) {
	       ADVANCE_CHAR;
               ADVANCE_CHAR;
               goto token_return;
	       }
	    else {
               ADVANCE_PAST_QUOTE;
	       return ERR_BADCHAR;
               }
	 break; /* end of TOK_CHAR and TOK_STRING case */


      /*-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-*/

      case TOK_DFINT:           /* Default integer */
	 if( isdigit( CURR_CHAR ))
	    TOKEN_ADD( CURR_CHAR )
	 else if( FLOAT_IS_STARTING( ch, NEXT_CHAR )) {
	    token_state = TOK_DOUBLE;
	    TOKEN_ADD( CURR_CHAR );
	    }
	 else if( LOOKS_LIKE_LONG( ch, NEXT_CHAR )) {
	    long_num = TRUE;
	    ADVANCE_CHAR;
	    goto token_return;
	    }
	 else if( isident( CURR_CHAR ))
	    return ERR_BADINT;
         else
   	    goto token_return;
	 break; /* end of TOK_DFINT case */


      /*-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-*/

      case TOK_OCT:             /* Octal token */
	 if( *toklen == 0 && CURR_CHAR == '0' )
	    {}
	 else if( isodigit( CURR_CHAR ))
	    TOKEN_ADD( CURR_CHAR )
	 else if( FLOAT_IS_STARTING( ch, NEXT_CHAR )) {
	    token_state = TOK_DOUBLE;
	    if( *toklen == 0 )
	       TOKEN_ADD( '0' );
	    TOKEN_ADD( CURR_CHAR );
	    }
	 else if( LOOKS_LIKE_LONG( ch, NEXT_CHAR )) {
	    long_num = TRUE;
	    ADVANCE_CHAR;
	    if( *toklen == 0 ) {
	       token_state = TOK_DFINT;
	       TOKEN_ADD( '0' );
	       goto token_return;
	       }
	    goto token_return;
	    }
	 else if( isdigit( CURR_CHAR ) || isident( CURR_CHAR ))
	    return ERR_BADOCT;
	 else if( *toklen == 0 ) {
	    token_state = TOK_DFINT;
	    TOKEN_ADD( '0' );
	    goto token_return;
	    }
	 else
	    goto token_return;
	 break; /* end of TOK_OCT case */


      /*-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-*/

      case TOK_HEX:             /* Hexadecimal token */
	 if( isxdigit( CURR_CHAR ))
	    TOKEN_ADD( CURR_CHAR )
	 else if( LOOKS_LIKE_LONG( ch, NEXT_CHAR ) && *toklen > 0 ) {
	    long_num = TRUE;
	    ADVANCE_CHAR;
	    goto token_return;
	    }
	 else if( isident( CURR_CHAR ) || *toklen == 0 )
	    return ERR_BADHEX;
	 else
	    goto token_return;
	 break; /* end of TOK_HEX case */


      /*-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-*/

      case TOK_DOUBLE:          /* Double-size floating point number */

      /* First time here */
	 if( double_state == FLOAT_INTEGER )
	    if( token[*toklen-1] == '.' ) 
	       double_state = FLOAT_FRACTION;
            else 
               double_state = FLOAT_NEWEXPON;
            
      /* Completing the fractional part */
	 if( double_state == FLOAT_FRACTION ) {
	    if( isdigit( CURR_CHAR ))
	       TOKEN_ADD( CURR_CHAR )
	    else if( CURR_CHAR == 'E' || CURR_CHAR == 'e' ) {
	       TOKEN_ADD( CURR_CHAR );
	       ADVANCE_CHAR;
	       double_state = FLOAT_NEWEXPON;
	       }
	    else if( isident( CURR_CHAR ))
	       return ERR_BADFLOAT;
	    else
	       goto token_return;
	    }

      /* Starting the exponent */
	 if( double_state == FLOAT_NEWEXPON ) {
	    if( CURR_CHAR == '+' || CURR_CHAR == '-' ) {
	       TOKEN_ADD( CURR_CHAR );
	       ADVANCE_CHAR;
	       }
	    if( isdigit( CURR_CHAR ))
	       double_state = FLOAT_EXPONENT;
	    else
	       return ERR_BADFLOAT;
	    }

      /* Completing the exponent */
	 if( double_state == FLOAT_EXPONENT ) {
	    if( isdigit( CURR_CHAR ))
	       TOKEN_ADD( CURR_CHAR )
	    else if( isident( CURR_CHAR ))
	       return ERR_BADFLOAT;
	    else
	       goto token_return;
	    }

	 break; /* end of TOK_DOUBLE case */


      default:
	 break;
	 }


   /* Advance to Next Character */
      ADVANCE_CHAR;

      }

/* ----------------------------------------------------------------------- */
/* Done looping until next token is finished                               */
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */


/* ----------------------------------------------------------------------- */
/* Finalize token value                                                    */
/* ----------------------------------------------------------------------- */

token_return:

/* Append null-byte to finish the token */
   TOKEN_ADD( '\0' );

/* Check to make sure a constant is not butted up against another */
   if ( token_state != TOK_OPERATOR )
      if ( CURR_CHAR  == '"' ||  CURR_CHAR  == '\'' || isdigit( CURR_CHAR ) ||
	 ( CURR_CHAR == '.' && isdigit( NEXT_CHAR )))
	 return ERR_CONSTSEP;

   switch( token_state ) {   


   /*-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-DFINT-*/

   case TOK_DFINT: 

   /* Output to default sized integer */
#if SHORT_INT_MACHINE || 1
      if( !long_num &&
	  ( *toklen-1 < DFINT_MAX_DIGITS ||
	    ( *toklen-1 == DFINT_MAX_DIGITS  &&
	    strcmp( DFINT_MAX_STRING, token ) >= 0 ))) {
	 int value;
	 value = atoi( token );
	 *(int *)token = value;
	 *toklen = sizeof( int );
	 }

      else
#endif

   /* Output to long sized integer handling */
	 {
	 long value;
	 if( *toklen-1 > LNINT_MAX_DIGITS ||
	     ( *toklen-1 == LNINT_MAX_DIGITS  &&
	       strcmp( LNINT_MAX_STRING, token ) < 0 ))
	    return ERR_TOOBIGINT;
	 value = atol( token );
	 *(long *)token = value;
	 *toklen = sizeof( long );
	 token_state = TOK_LNINT;
	 }

      break;


   /*-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-OCT-*/

   case TOK_OCT: 

      {
      unsigned long value;

   /* Output to default sized integer */
#if SHORT_INT_MACHINE || 1
      if( !long_num &&
	  ( *toklen-1 < DFOCTS_MAX_DIGITS ||
	    ( *toklen-1 == DFOCTS_MAX_DIGITS  &&
	    strcmp( DFOCTS_MAX_STRING, token ) >= 0 ))) {
	 value = atoo( token );
	 *(int *)token = (int) value;
	 *toklen = sizeof( int );
	 token_state = TOK_DFINT;
	 }

   /* Output to default sized unsigned integer */
      else if ( !long_num &&
		( *toklen-1 < DFOCTU_MAX_DIGITS ||
		  ( *toklen-1 == DFOCTU_MAX_DIGITS  &&
		  strcmp( DFOCTU_MAX_STRING, token ) >= 0 ))) {
	 value = atoo( token );
	 *(unsigned *)token = (unsigned) value;
	 *toklen = sizeof( unsigned );
	 token_state = TOK_DFUNS;
	 }

      else
#endif

   /* Output to long sized integer */
      if ( *toklen-1 < LNOCTS_MAX_DIGITS ||
	   ( *toklen-1 == LNOCTS_MAX_DIGITS  &&
	 strcmp( LNOCTS_MAX_STRING, token ) >= 0 )) {
	 value = atoo( token );
	 *(long *)token = (long) value;
	 *toklen = sizeof( long );
	 token_state = TOK_LNINT;
	 }

   /* Output to long sized unsigned integer */
      else {
	 if( *toklen-1 > LNOCTU_MAX_DIGITS ||
	    ( *toklen-1 == LNOCTU_MAX_DIGITS  &&
	    strcmp( LNOCTU_MAX_STRING, token ) < 0 ))
	    return ERR_TOOBIGOCT;
	 value = atoo( token );
	 *(unsigned long *)token = (unsigned long) value;
	 *toklen = sizeof( unsigned long );
	 token_state = TOK_LNUNS;
	 }
      }

      break;


   /*-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-HEX-*/

   case TOK_HEX:
   
      {
      unsigned long value;

   /* Output to default sized integer */
#if SHORT_INT_MACHINE || 1
      if( !long_num &&
	  ( *toklen-1 < DFHEXS_MAX_DIGITS ||
	    ( *toklen-1 == DFHEXS_MAX_DIGITS  &&
	    strcasecmp( DFHEXS_MAX_STRING, token ) >= 0 ))) {
	 value = atox( token );
	 *(int *)token = (int) value;
	 *toklen = sizeof( int );
	 token_state = TOK_DFINT;
	 }

   /* Output to default sized unsigned integer */
      else if ( !long_num &&
		( *toklen-1 < DFHEXU_MAX_DIGITS ||
		  ( *toklen-1 == DFHEXU_MAX_DIGITS  &&
		  strcasecmp( DFHEXU_MAX_STRING, token ) >= 0 ))) {
	 value = atox( token );
	 *(unsigned *)token = (unsigned) value;
	 *toklen = sizeof( unsigned );
	 token_state = TOK_DFUNS;
	 }

      else
#endif

   /* Output to long sized integer */
      if ( *toklen-1 < LNHEXS_MAX_DIGITS ||
	   ( *toklen-1 == LNHEXS_MAX_DIGITS  &&
	 strcasecmp( LNHEXS_MAX_STRING, token ) >= 0 )) {
	 value = atox( token );
	 *(long *)token = (long) value;
	 *toklen = sizeof( long );
	 token_state = TOK_LNINT;
	 }

   /* Output to long sized unsigned integer */
      else {
	 if( *toklen-1 > LNHEXU_MAX_DIGITS ||
	    ( *toklen-1 == LNHEXU_MAX_DIGITS  &&
	    strcasecmp( LNHEXU_MAX_STRING, token ) < 0 ))
	    return ERR_TOOBIGHEX;
	 value = atox( token );
	 *(unsigned long *)token = (unsigned long) value;
	 *toklen = sizeof( unsigned long );
	 token_state = TOK_LNUNS;
	 }
      }

      break;

   /*-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-DOUBLE-*/

   case TOK_DOUBLE: 
  
      {
      double value;
      sscanf( token, DOUBL_SCFORM, &value );
      *(double *)token = value;
      *toklen = sizeof( double );
      }

      break;


   default:
      break;
      }


/* ----------------------------------------------------------------------- */
/* Assign token type and return                                            */
/* ----------------------------------------------------------------------- */

   *toktype = token_state;
   return FUNCOK;

}
