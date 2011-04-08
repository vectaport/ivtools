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
parser.c        COMTERP postfix parsing routine

Externals:      int parser()

Summary:        

History:        Written by Scott E. Johnston, April 1989
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "comterp.ci"

int _continuation_prompt;
int _continuation_prompt_disabled = 0;
int _skip_shell_comments = 0;
infuncptr _oneshot_infunc;

static int get_next_token(void *infile, char *(*infunc)(), int (*eoffunc)(),
			  int (*errfunc)(), FILE *outfile, int (*outfunc)(),
                          char *buffer, unsigned bufsiz, unsigned *bufptr,
			  char *token, unsigned toksiz, unsigned *toklen,
                          unsigned *toktype, unsigned *tokstart, unsigned *linenum,
                          int *op_ids, unsigned nop_ids);

/* TYPDEFS for oper stack - should be put in comterp/comterp.h? */
#define OPERATOR  0
#define LEFTPAREN 1
#define KEYWORD   2

/* Structure of stack to count command arguments and keywords */
typedef struct _paren_stack paren_stack;
struct _paren_stack 
{
  unsigned nids;
  unsigned narg;
  unsigned nkey;
  unsigned paren_type;
  int comm_id;
};

/* Structure of stack to store operators and keywords */
typedef struct _oper_stack oper_stack;
struct _oper_stack 
{
  int id;
  int oper_type;
};

/* Static Variables */
static unsigned expecting;              /* Type of operator expected next */

static paren_stack *ParenStack = NULL;  /* Stack to count args and keywords */
static int TopOfParenStack = -1;        /* Top of ParenStack */
static int SizeOfParenStack;            /* Allocated size of ParenStack */

static oper_stack *OperStack = NULL;   /* Operator stack */
static int TopOfOperStack = -1;        /* Top of OperStack */
static int SizeOfOperStack;            /* Allocated size of OperStack */

static unsigned NextBufptr;            /* Variables for look-ahead token */
static char *NextToken = NULL;
static unsigned NextToklen;    
static unsigned NextToktype;
static unsigned NextTokstart;
static unsigned NextLinenum;
static int NextOp_ids[OPTYPE_NUM];

/* Parenthesis stack macros */
#define INITIAL_PAREN_STACK_SIZE 32
#define PARENSTK_PUSH( paren_val, comm_val, nids_val ) {\
if( ++TopOfParenStack == SizeOfParenStack ) {\
   SizeOfParenStack *= 2;\
   dmm_realloc_size(sizeof(paren_stack));\
   if( dmm_realloc( (void **) &ParenStack, (long)SizeOfParenStack )) {\
      COMERR_SET( ERR_MEMORY );\
      goto error_return;}}\
ParenStack[TopOfParenStack].paren_type = paren_val;\
ParenStack[TopOfParenStack].narg = 0;\
ParenStack[TopOfParenStack].nkey = 0;\
ParenStack[TopOfParenStack].nids = nids_val;\
ParenStack[TopOfParenStack].comm_id = comm_val;}

/* Operator stack macros */
#define INITIAL_OPER_STACK_SIZE 32
#define OPERSTK_PUSH( oper_id, type_val ) {\
if( ++TopOfOperStack == SizeOfOperStack ) {\
    SizeOfOperStack *= 2;\
    dmm_realloc_size(sizeof(oper_stack));\
    if( dmm_realloc( (void **) &OperStack, (long)SizeOfOperStack )) {\
        COMERR_SET( ERR_MEMORY );\
        goto error_return;}}\
OperStack[TopOfOperStack].id = oper_id;\
OperStack[TopOfOperStack].oper_type = type_val;}

#define OPERSTK_POP( oper_id ) {\
if( TopOfOperStack < 0 )\
   KAPUT( "Unexpected empty operator stack" );\
oper_id = OperStack[TopOfOperStack--].id;}


#define OPERSTK_FLUSH \
{\
while( (OperStack[TopOfOperStack].oper_type != LEFTPAREN) &&\
       ( TopOfOperStack >=0) )\
{\
    if (OperStack[TopOfOperStack].oper_type == OPERATOR)\
    {\
        OPERSTK_POP( temp_id );\
        PFOUT( TOK_COMMAND, opr_tbl_commid( temp_id ),\
           (opr_tbl_optype( temp_id ) == OPTYPE_BINARY ? 2 : 1), 0, 1 );\
    }\
    else\
    {\
        OPERSTK_POP( temp_id );\
        PFOUT( TOK_KEYWORD, temp_id, 1, 0, 0);\
    }\
}}

#define LOOK_AHEAD {\
char *(*infunc2)();\
if( TopOfParenStack < 0 ) infunc2 = NULL;\
else infunc2 = infunc;\
NextBufptr=*bufptr;\
NextLinenum=*linenum;\
status = get_next_token(\
   infile, infunc2, eoffunc, errfunc, outfile, outfunc,\
   buffer, bufsiz, &NextBufptr, NextToken, toksiz, \
   &NextToklen, &NextToktype, &NextTokstart, &NextLinenum,\
   NextOp_ids, OPTYPE_NUM );\
if( status != 0 ) goto error_return;}


#define UNEXPECTED_RPAREN_ERROR( toktype ){\
int errid;\
if( toktype == TOK_RPAREN ) \
   errid = ERR_UNEXPECTED_RPAREN;\
else if( toktype == TOK_RBRACKET )\
   errid = ERR_UNEXPECTED_RBRACKET;\
else\
   errid = ERR_UNEXPECTED_RBRACE;\
COMERR_SET1( errid, *linenum );\
goto error_return;}

#define UNEXPECTED_LPAREN_ERROR( toktype ){\
int errid;\
if( toktype == TOK_LPAREN ) \
   errid = ERR_UNEXPECTED_LPAREN;\
else if( toktype == TOK_LBRACKET )\
   errid = ERR_UNEXPECTED_LBRACKET;\
else\
   errid = ERR_UNEXPECTED_LBRACE;\
COMERR_SET1( errid, *linenum );\
goto error_return;}

#define INSTACK_PRIORITY_HIGHER( incoming_priority ) rkg_instack( incoming_priority)

#define LEFT_PAREN( toktype ) \
(toktype == TOK_LPAREN || toktype == TOK_LBRACKET || toktype == TOK_LBRACE)

#define RIGHT_PAREN( toktype ) \
(toktype == TOK_RPAREN || toktype == TOK_RBRACKET || toktype == TOK_RBRACE)

#define PROCEEDING_WHITESPACE( tokstart ) \
(tokstart == 0 || isspace( buffer[tokstart-1] ))

#define TRAILING_WHITESPACE( bufptr ) \
(isspace( buffer[bufptr] ))

#define UNEXPECTED_NEW_EXPRESSION \
(TopOfParenStack >= 0 && \
(ParenStack[ TopOfParenStack ].comm_id < 0 || \
ParenStack[ TopOfParenStack ].nkey > 0 ))


#define UNARY_AMBIGUITY( op_ids )\
(op_ids[OPTYPE_UNARY_PREFIX]>=0 && op_ids[OPTYPE_UNARY_POSTFIX]>=0)


#define BINARY_AMBIGUITY( op_ids )\
(op_ids[OPTYPE_UNARY_PREFIX]>=0 && op_ids[OPTYPE_BINARY]>=0)


#define AMBIGUITY( op_ids )\
(UNARY_AMBIGUITY(op_ids)||BINARY_AMBIGUITY(op_ids))


/* === Static functions ================================================== */

/* Check for instack priority higher */
static int rkg_instack(prior)
  int prior;
{
    int x;

    switch(OperStack[TopOfOperStack].oper_type)
    {
    case LEFTPAREN:
    case KEYWORD:
        return(0);
    case OPERATOR:
        x  = opr_tbl_priority(OperStack[TopOfOperStack].id)*2;
        x += (opr_tbl_rtol(OperStack[TopOfOperStack].id)?-1:1);
        return( x > prior*2 );
    default:
	printf ("rkg_instack: unknown type\n");
	return -1;
    }
}

/* Output a token in its postfix order */
#define PFOUT( toktype, tokid, narg_val, nkey_val, nids_val ) {\
if(pfout(pfbuf,pfsiz,pfnum,toktype,tokid,narg_val,nkey_val,nids_val))\
    goto error_return;}

static int pfout( pfbuf, pfsiz, pfnum, toktype, tokid, narg_val, nkey_val, nids_val )

postfix_token** pfbuf;	/* Double pointer to buffer to receive postfix expression. */
unsigned *      pfsiz;  /* Size of `pfbuf`. */
unsigned *      pfnum;  /* Number of tokens returned in `pfbuf`. */
unsigned        toktype;/* Token type */
int             tokid;  /* Identifier that corresponds to this token */
unsigned        narg_val;/* Number of arguments associated with this token */
unsigned        nkey_val;/* Number of keywords associated with this token */
unsigned        nids_val;/* Number of ids associated with this token */

{

/* Increase size of output buffer if necessary by doubling it */
   if( *pfnum+1 == *pfsiz ) {
      *pfsiz *= 2; 
      dmm_realloc_size(sizeof(postfix_token));
      if( dmm_realloc( (void **)pfbuf, (long)*pfsiz )) {
         COMERR_SET( ERR_MEMORY );
         return FUNCBAD;
	 }
      }

/* Fill in postfix token structure */
   (*pfbuf+*pfnum)->nids = nids_val;
   (*pfbuf+*pfnum)->type = toktype;
   (*pfbuf+*pfnum)->narg = narg_val;
   (*pfbuf+*pfnum)->nkey = nkey_val;
   (*pfbuf+*pfnum)->v.symbolid = tokid;
   ++*pfnum;

   return FUNCOK;

}

/* set value in literal token */
#define PFOUT_LITERAL( toktype, token ) {\
if(pfout_literal(pfbuf,pfsiz,pfnum,toktype,token))\
   goto error_return;}

#define EMPTY_OPER_STACK {\
   while( TopOfOperStack >= 0 ) \
   {\
      if (OperStack[TopOfOperStack].oper_type == OPERATOR)\
      {\
         OPERSTK_POP( temp_id );\
         PFOUT( TOK_COMMAND, opr_tbl_commid( temp_id ),\
              (opr_tbl_optype( temp_id ) == OPTYPE_BINARY ? 2 : 1), 0, 1 );\
      }\
      else\
      {\
         OPERSTK_POP( temp_id );\
         PFOUT( TOK_KEYWORD, temp_id, 1, 0, 0);\
      }\
   }\
}


static int pfout_literal( pfbuf, pfsiz, pfnum, toktype, token)

postfix_token** pfbuf;	/* Double pointer to buffer to receive postfix expression. */
unsigned *      pfsiz;  /* Size of `pfbuf`. */
unsigned *      pfnum;  /* Number of tokens returned in `pfbuf`. */
unsigned        toktype;/* Token type */
char *          token;  /* Token buffer */

{
    if(pfout(pfbuf,pfsiz,pfnum,toktype,0,0,0,0))
	return FUNCBAD;
    memcpy(&(*pfbuf+*pfnum-1)->v.doublval, token, sizeof(double));
    return FUNCOK;
}


/* Check if the following tokens on this line can be resolved without  */
/* ambiguity.  Used when one ambiguous operator has already been found */

#define STRING_OF_AMBIGUITY_CHECK( ambiguous ) {\
status = string_of_ambiguity_check( &ambiguous, &NextBufptr, *bufptr, \
   &NextLinenum, *linenum, buffer, bufsiz, NextToken, toksiz, \
   &NextToklen, &NextToktype,  &NextTokstart, NextOp_ids ); \
if( status != 0 ) goto error_return;}

static int string_of_ambiguity_check( ambiguous, next_bufptr, bufptr, 
   next_linenum, linenum, buffer, bufsiz, next_token, toksiz, 
   next_toklen, next_toktype, next_tokstart, next_op_ids )

BOOLEAN  *ambiguous;	/* flag returned TRUE if operators are ambiguous */
unsigned *next_bufptr;	/* pointer to variable to use for temp bufptr    */
unsigned bufptr;	/* current value of bufptr			 */
unsigned *next_linenum; /* pointer to variable to use for temp linenum   */
unsigned linenum;	/* current value of linenum			 */
char     *buffer;	/* pointer to input character buffer             */
unsigned  bufsiz;	/* size of buffer				 */
char     *next_token;   /* pointer to temporary token buffer             */
unsigned  toksiz;	/* size of next_token				 */
unsigned *next_toklen;  /* pointer to temporary token length variable    */
unsigned *next_toktype; /* pointer to temporary token type variable      */
unsigned *next_tokstart;/* pointer to temporary token start variable     */
int	 *next_op_ids;  /* pointer to temporary token op ids array       */

{
int status;

/* Set ambiguity flag, and set out to disprove starting with next token */
   *ambiguous = TRUE;
   *next_bufptr=bufptr;
   *next_linenum=linenum;

/* Loop until end of this line of input, or ambiguity is proven or disproven */
   do{
      status = get_next_token( NULL, NULL, NULL, NULL, NULL, NULL,
                               buffer, bufsiz, next_bufptr, next_token, toksiz, 
                               next_toklen, next_toktype, next_tokstart, 
                               next_linenum,  next_op_ids, OPTYPE_NUM );
      if( status != 0 )
	 return FUNCBAD;

   /* If a token is found that is not an operator or its an operator */
   /* with no ambiguity, the search is over, ambiguity does not reign */
      if( *next_toktype != TOK_OPERATOR || !AMBIGUITY( next_op_ids ) )
	 *ambiguous = FALSE;
      }
   while( *ambiguous && *next_toktype != TOK_EOF &&
	  !TRAILING_WHITESPACE( *next_bufptr ));
/* Done looping until end of this line of input */

/* Reset length indicator for next token, to throw away any look-ahead tokens */
   *next_toklen = 0;
   return FUNCOK;
}


/*!

get_next_token  Get next token with operators fully typed and expanded
		to maximum length.


Summary:

#include <comterp/comterp.h>
*/

static int get_next_token(
   void * infile,char * (*infunc)(),int (*eoffunc)(),int (*errfunc)(), 
   FILE * outfile, int (*outfunc)(),char * buffer,unsigned bufsiz,
   unsigned * bufptr,char * token,unsigned toksiz,unsigned * toklen,
   unsigned * toktype, unsigned * tokstart, unsigned * linenum, 
   int * op_ids, unsigned nop_ids )


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
char *          token     ;/* O   Buffer for parsing tokens. */
unsigned        toksiz    ;/* I   Maximum possible token size. */
unsigned *      toklen    ;/* O   Actual token size.           */
unsigned *      toktype   ;/* O   Token type.                  */
unsigned *      tokstart  ;/* O   Starting offset of token in `buffer`. */
unsigned *      linenum   ;/* IO  Current line number of input file
				  (initialize by setting to zero). */
int *           op_ids    ;/* O   Array of operator ids associated with a   
				  given operator. */
unsigned        nop_ids   ;/* I   Length of `nop_ids` -- should be 3. */
#endif


/*!
Description:

`get_next_token` is a static function to return the next token from
the input stream, with any operators fully typed (binary versus unary
prefix/postfix) and expanded to maximum length.

!*/

{

int nchar;              /* Number of characters in operator */
int status;

/* Read token from input file */
   status = scanner( infile, infunc, eoffunc, errfunc, outfile, outfunc,
                     buffer, bufsiz, bufptr, token, toksiz, toklen,
                     toktype, tokstart, linenum );
   if( status != 0 ) 
      return FUNCBAD;

/* Expand operator token to longest possible match */
   if( *toktype == TOK_OPERATOR ) {

   /* Get ids of commands associated with this operator */
      if( opr_tbl_entries( buffer+*tokstart, op_ids, nop_ids, &nchar ))
	 return FUNCBAD;

   /* Augment token to entire operator */
      if( nchar > 1 )
	 if( toksiz > nchar ) {
	    strncat( token, buffer+*tokstart+1, nchar-1 );
	    *bufptr += nchar - 1;
	    }
         else
	    KAPUT( "Token too short for expanded operator" );
      }

/* Replace identifiers, keywords, and strings with symbol id's */
   else if( *toktype == TOK_IDENTIFIER || 
            *toktype == TOK_KEYWORD ||
            *toktype == TOK_STRING ) {
      *toklen = sizeof(int);
      *(int *)token = symbol_add( token );
      if( *(int *) token < 0 ) {
         COMERR_SET( ERR_MEMORY );
	 return FUNCBAD;
         } 
      }

   return FUNCOK;

}

/*!

parser   Return next expression from input text source, in postfix order


Summary:

#include <comterp/comterp.h>
*/

int parser(void * infile,char * (*infunc)(),int (*eoffunc)(),int (*errfunc)(),
	   FILE * outfile,int (*outfunc)(),char * buffer,unsigned bufsiz,
	   unsigned * bufptr,char * token,unsigned toksiz,unsigned * linenum,
	   postfix_token ** pfbuf,unsigned * pfsiz,unsigned * pfnum)


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
char *          token     ;/* O   Buffer for parsing tokens. */
unsigned        toksiz    ;/* I   Maximum possible token size. */
unsigned *      linenum   ;/* IO  Current line number of input file
				  (initialize by setting to zero). */
postfix_token** pfbuf     ;/* O   Double pointer to buffer to receive
				  postfix expression. */
unsigned *      pfsiz     ;/* IO  Size of `pfbuf`. */
unsigned *      pfnum     ;/* IO  Number of tokens returned in `pfbuf`. */
#endif


/*!
Description:

`parser` parses the input file (pointed to by `infile`) and outputs a buffer
of postfix ordered tokens. The possible token types are as follows
(defined in "comutil.h"):

        TOK_NONE        No token found (error occurred)
	TOK_KEYWORD     Keyword for free format parameters, i.e. ":SIZE"
	TOK_COMMAND     Command name
        TOK_STRING      Character string constant
	TOK_CHAR        Character constant
        TOK_DFINT       Default integer, i.e. of type "int"
        TOK_DFUNS       Default unsigned integer, i.e. of type "unsigned"
        TOK_LNINT       Long integer, i.e. of type "long"
        TOK_LNUNS       Long unsigned integer, i.e. of type "long unsigned"
	TOK_DOUBLE      Double-size floating point number, i.e. of type "double"
        TOK_EOF         End of file

`infunc`, `eoffunc`, and `errfunc` are a set of functions that allow `parser`
to access new lines of text as required.  If `infunc` is `NULL`, parsing is
done on the contents of `buffer` only.  In this case the contents of `buffer`
must terminate with a '\n'.


Reference:

Horowitz, E. and S. Sahni, "Fundamentals of Data Structures", Computer Science
Press Inc., Potomac, MD, 1976, pp. 91-97.


See Also:  scanner, opr_tbl_create, opr_tbl_insert

!*/

{

int op_ids[OPTYPE_NUM]; /* Array of potential operator ids */
int optype;             /* Operator type that was decided on. */
unsigned toklen;        /* Length of current token */
unsigned toktype;       /* Type of current token */
unsigned tokstart;      /* Start of current token in buffer */
BOOLEAN done;           /* Set true when expression is complete */
BOOLEAN ambiguous;      /* Used to indicate string of amb. ops. */

int temp_id;            /* Temporary variables */
int index;
int status;

/* Static initialization */
   if( *linenum == 0 ) {

   /* Allocate space for look-ahead token */
      NextToklen = 0;
      if( NextToken != NULL ) 
         if( dmm_free( (void **)&NextToken ))
            KAPUT( "Error in freeing NextToken" );
      if( dmm_calloc( (void **)&NextToken, (long)toksiz, 
                      (long)sizeof(char))) {
         COMERR_SET( ERR_MEMORY );
	 goto error_return;
         }
     
   /* Initialize parenthesis stack */
      TopOfParenStack = -1;
      if( ParenStack == NULL ) {
         SizeOfParenStack = INITIAL_PAREN_STACK_SIZE;
         if( dmm_calloc( (void **)&ParenStack, (long)SizeOfParenStack,
                         (long)sizeof(paren_stack) )) {
            COMERR_SET( ERR_MEMORY ); 
            goto error_return; 
            }
         }
      else if( SizeOfParenStack > INITIAL_PAREN_STACK_SIZE ) {
         SizeOfParenStack = INITIAL_PAREN_STACK_SIZE;
         dmm_realloc_size(sizeof(paren_stack));
         if( dmm_realloc( (void **)&ParenStack, (long)SizeOfParenStack ))
            KAPUT( "Unable to shrink ParenStack" );
         }

   /* Initialize operator stack */
      TopOfOperStack = -1;
      if( OperStack == NULL ) {
         SizeOfOperStack = INITIAL_OPER_STACK_SIZE;
         if( dmm_calloc( (void **)&OperStack, (long)SizeOfOperStack,
                         (long)sizeof(oper_stack) )) {
            COMERR_SET( ERR_MEMORY ); 
            goto error_return;
            } 
         }
      else if( SizeOfOperStack > INITIAL_OPER_STACK_SIZE ) {
         SizeOfOperStack = INITIAL_OPER_STACK_SIZE;
         dmm_realloc_size(sizeof(oper_stack));
         if( dmm_realloc( (void **)&OperStack, (long)SizeOfOperStack ))
            KAPUT( "Unable to shrink OperStack" );
         }

      }

/* Ensure initialization has occurred */
   else if( ParenStack == NULL ) 
      KAPUT( "Needs to be called with linenum set to 0" );

/* Auto variable initialization */
   if (TopOfOperStack == -1 && TopOfParenStack == -1) {
      *pfnum = 0;
      expecting = OPTYPE_UNARY_PREFIX;
      }
   done = FALSE;


/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* Loop through tokens until an expression is complete                     */
/* ----------------------------------------------------------------------- */

   do {

   /* Get current token, either from 'Next' variables if they are full */
   /* or directly from the scanner                                     */
      if( NextToklen == 0 ) {
         status = get_next_token( 
            infile, infunc, eoffunc, errfunc, outfile, outfunc,
            buffer, bufsiz, bufptr, token, toksiz, &toklen,
	    &toktype, &tokstart, linenum, op_ids, OPTYPE_NUM );
         if( status != 0 ) 
	    goto error_return;
         }
      else {
	 *bufptr = NextBufptr;
         for( index=0; index<NextToklen; index++ )
            token[index] = NextToken[index];
         token[index] = '\0';
	 toklen = NextToklen;
	 toktype = NextToktype;
	 tokstart = NextTokstart;
	 *linenum = NextLinenum;
	 for( index=0; index<OPTYPE_NUM; index++ )
	    op_ids[index] = NextOp_ids[index];
	 NextToklen = 0;
	 }
      _continuation_prompt = 1;


   /* --------------------------------------*/
   /* Switch on token type                  */

   /* Switch on token type */
      switch( toktype ) {


/*-OPERATOR-OPERATOR-OPERATOR-OPERATOR-OPERATOR-OPERATOR-OPERATOR-OPERATOR-*/

      case TOK_OPERATOR:

      /* expecting unary prefix.  Did we get it? */
	 if( expecting == OPTYPE_UNARY_PREFIX ) {
	    if( op_ids[OPTYPE_UNARY_PREFIX] >= 0 )
	       optype = OPTYPE_UNARY_PREFIX;
	    else {
	       COMERR_SET2( ERR_UNEXPECTED_OPERATOR, *linenum, token );
	       goto error_return;
	       }
	    }

      /* expecting binary.  What did we really get? */
	 else {

	 /* Resolve ambiguous binary/prefix operators */
	    if( op_ids[OPTYPE_BINARY] >= 0 &&
		op_ids[OPTYPE_UNARY_PREFIX] >= 0 ) {
	       if( PROCEEDING_WHITESPACE( tokstart ) &&
		   !TRAILING_WHITESPACE( *bufptr ) )
		  optype = OPTYPE_UNARY_PREFIX;
	       else
		  optype = OPTYPE_BINARY;
	       }

	 /* Resolve ambiguous prefix/postfix operators */
	    else if( op_ids[OPTYPE_UNARY_PREFIX] >= 0 &&
		     op_ids[OPTYPE_UNARY_POSTFIX] >= 0 ) {

	       if( !PROCEEDING_WHITESPACE( tokstart ) )
		  optype = OPTYPE_UNARY_POSTFIX;

	       else if( !TRAILING_WHITESPACE( *bufptr ) ) {
		  STRING_OF_AMBIGUITY_CHECK( ambiguous );
		  if( ambiguous ) {
		      COMERR_SET2( ERR_AMBIGUOUS_OPERATOR, *linenum,
				   NextToken );
		      goto error_return;
		      }
		  optype = OPTYPE_UNARY_PREFIX;
                  }

	       else {
		  COMERR_SET2( ERR_AMBIGUOUS_OPERATOR, *linenum,
			       token );
		  goto error_return;
		  }
	       }

	 /* Binary only */
	    else if( op_ids[OPTYPE_BINARY] >= 0 ) {
	       optype = OPTYPE_BINARY;
	       }

	 /* Prefix only */
	    else if( op_ids[OPTYPE_UNARY_PREFIX] >= 0 ) {

	       if( PROCEEDING_WHITESPACE( tokstart ))
		  optype = OPTYPE_UNARY_PREFIX;

	       else {
		  COMERR_SET2( ERR_UNEXPECTED_OPERATOR, *linenum,
			       symbol_pntr( *(int *)token ) );
		  goto error_return;
		  }
	       }

	 /* Postfix only */
	    else if( op_ids[OPTYPE_UNARY_POSTFIX] >= 0 ) {
	       optype = OPTYPE_UNARY_POSTFIX;
	       }

	    }

      /* If unary prefix, but no new expression expected, raise an error */
	 if( expecting == OPTYPE_BINARY &&
	     optype == OPTYPE_UNARY_PREFIX ) {
	    if( UNEXPECTED_NEW_EXPRESSION ) {
	       COMERR_SET2( ERR_UNEXPECTED_OPERATOR, *linenum, token );
	       goto error_return;
	       }

	 /* End of an argument                     */
	 /* Clear the stack back down to the paren */
	    if( TopOfParenStack >= 0 ) {
	       ParenStack[ TopOfParenStack ].narg++;
	       OPERSTK_FLUSH;
	       }

	    }

      /* Set up for next operator */
	 if( optype == OPTYPE_UNARY_PREFIX || optype == OPTYPE_BINARY )
	   expecting = OPTYPE_UNARY_PREFIX;
	 else
	   expecting = OPTYPE_BINARY;

      /* Take all operators off the stack of higher priority than this one */
	 while ( TopOfOperStack >= 0 &&
		 INSTACK_PRIORITY_HIGHER(opr_tbl_priority(op_ids[optype]))) 
         {
             if (OperStack[TopOfOperStack].oper_type == OPERATOR)
             {
                 OPERSTK_POP( temp_id );
                 PFOUT( TOK_COMMAND, opr_tbl_commid( temp_id ),
                    (opr_tbl_optype( temp_id ) == OPTYPE_BINARY ? 2 : 1), 0, 1 );
             }
             else
             {
                 OPERSTK_POP( temp_id );
                 PFOUT( TOK_KEYWORD, temp_id, 1, 0, 0);
             }
	 }

      /* Postfix operators do not go on stack */
	 if( opr_tbl_optype( op_ids[ optype ] ) == OPTYPE_UNARY_POSTFIX ) {
	    PFOUT( TOK_COMMAND, opr_tbl_commid( op_ids[optype] ), 1, 0, 1 );
	    break;
	    }

      /* Place this one on the stack */
	 OPERSTK_PUSH( op_ids[optype], OPERATOR );

	 break;


/*-LITERALS-LITERALS-LITERALS-LITERALS-LITERALS-LITERALS-LITERALS-LITERALS-*/

      default:

      /* Expecting a binary is either an error, or an indicator of  */
      /* separation between free format parameters                  */
         if( expecting == OPTYPE_BINARY ) {

	    if( !PROCEEDING_WHITESPACE( tokstart ) ||
                UNEXPECTED_NEW_EXPRESSION ) {
	       strncpy( token, buffer+tokstart, *bufptr-tokstart );
	       token[*bufptr-tokstart] = '\0';
	       COMERR_SET2( ERR_UNEXPECTED_LITERAL, *linenum, token);
	       goto error_return;
	       }

	 /* End of an argument                     */
	 /* Clear the stack back down to the paren */
	    if( TopOfParenStack >= 0 ) {
	       ParenStack[ TopOfParenStack ].narg++;
	       OPERSTK_FLUSH;
	       }

	    }

      /* Output a literal */
	 PFOUT_LITERAL( toktype, token );

      /* Setup for binary operator */
	 expecting = OPTYPE_BINARY;
	 break;


/*-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-IDENTIFIER-*/

      case TOK_IDENTIFIER:

      /* Expecting a binary is either an error, or an indicator of  */
      /* separation between free format parameters                  */
	 if( expecting == OPTYPE_BINARY ) {

	    if( !PROCEEDING_WHITESPACE( tokstart ) ||
		UNEXPECTED_NEW_EXPRESSION ) {
	       COMERR_SET2( ERR_UNEXPECTED_IDENTIFIER, *linenum,
			    symbol_pntr( *(int *)token ) );
	       goto error_return;
	       }

	 /* End of an argument                     */
	 /* Clear the stack back down to the paren */
	    if( TopOfParenStack >= 0 ) {
	       ParenStack[ TopOfParenStack ].narg++;
	       OPERSTK_FLUSH;
	       }

	    }

	 LOOK_AHEAD;
         if (0 && (NextToktype == TOK_OPERATOR) && (*NextToken == '.') /* && (NextToklen == 1)*/ )
         {
            int i, ids[20];
            int nids = 0;

            ids[nids++] = *(int *)token;

            while ( (NextToktype == TOK_OPERATOR) && (*NextToken == '.') )
            {
	       *bufptr    = NextBufptr;
	       *linenum   = NextLinenum;
	       NextToklen = 0;
               LOOK_AHEAD;

               if (NextToktype != TOK_IDENTIFIER)
               {
                  printf("Error in parsing - expected an identifier\n");
                  printf("NextToktype: %d\n",NextToktype);
                  return(FUNCBAD);
               }
               ids[nids++] = *(int *)NextToken;

	       *bufptr    = NextBufptr;
	       *linenum   = NextLinenum;
	       NextToklen = 0;
               LOOK_AHEAD;
            }

            /* If next token is a left paren, this identifier is part of */
            /* a command name, so save the command id on the paren stack  */
            /* which will trigger the counting of narg and nkey to begin. */
	    if( LEFT_PAREN( NextToktype )) 
            {
               for (i = 0 ; i < nids; i++)
               {
  	          PARENSTK_PUSH( NextToktype, ids[i], nids );
               }
            
	       OPERSTK_PUSH ( -NextToktype, LEFTPAREN );
	       *bufptr    = NextBufptr;
	       *linenum   = NextLinenum;
	       NextToklen = 0;
	       expecting  = OPTYPE_UNARY_PREFIX;
	    }
	    else
            {
               for (i = nids ; i > 0; i--)
  	          PFOUT( TOK_COMMAND, ids[i-1], 0, 0, nids );
	       expecting = OPTYPE_BINARY;
	    }
         }
         else
         {
            /* If next token is a left paren, this identifier is part of */
            /* a command name, so save the command id on the paren stack  */
            /* which will trigger the counting of narg and nkey to begin. */
	    if( LEFT_PAREN( NextToktype )) 
            {
	       PARENSTK_PUSH( NextToktype, *(int *)token, 1 );
	       OPERSTK_PUSH ( -NextToktype, LEFTPAREN );
	       *bufptr    = NextBufptr;
	       *linenum   = NextLinenum;
	       NextToklen = 0;
	       expecting  = OPTYPE_UNARY_PREFIX;
	    }
	    else
            {
	       expecting = OPTYPE_BINARY;
  	       PFOUT( TOK_COMMAND, *(int *)token, 0, 0, 1 );
	    }
         }
	 break;


/*-KEYWORD-KEYWORD-KEYWORD-KEYWORD-KEYWORD-KEYWORD-KEYWORD-KEYWORD-KEYWORD-*/

      case TOK_KEYWORD:

      /* Must be inside parenthesis associated with a command */
      /* for a keyword to be legal                            */
	 if( TopOfParenStack < 0 || ParenStack[ TopOfParenStack ].comm_id < 0 ) {
	    COMERR_SET2( ERR_UNEXPECTED_KEYWORD, *linenum,
			 symbol_pntr( *(int *)token ) );
	    goto error_return;
	    }

      /* Increment the number of keywords associated with this */
      /* level of parens                                       */
	 ParenStack[ TopOfParenStack ].nkey++;

         /* Its an error if anything is on the operator */
         /* stack and a unary prefix was expected.      */
	 if( OperStack[TopOfOperStack].oper_type == OPERATOR &&
	     expecting == OPTYPE_UNARY_PREFIX ) 
         {
	     COMERR_SET2( ERR_UNEXPECTED_KEYWORD, *linenum,
			 symbol_pntr( *(int *) token ) );
	     goto error_return;
	 }

      /* If a binary was expected, this means that a previous  */
      /* argument was underway.  Increment number of arguments */
	 if( expecting == OPTYPE_BINARY ) 
         {
	    ParenStack[ TopOfParenStack ].narg++;
	    OPERSTK_FLUSH;
	 }

      /* If next token type is KEY or right_paren just output this keyword  */
      /* because it has no argument.  Otherwise place this one on the stack */
      /* Note that this means that a Keyword on the stack implies theat it  */
      /* has an argument.                                                   */
         LOOK_AHEAD;
         if ( (NextToktype == TOK_KEYWORD) ||
              (RIGHT_PAREN(NextToktype)))
         {
            PFOUT( TOK_KEYWORD, *(int *)token, 0, 0, 0 );
         }
         else
         {
             OPERSTK_PUSH( *(int *)token, KEYWORD ); 
         }

	 expecting = OPTYPE_UNARY_PREFIX;
	 break;


/*-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-LPAREN-*/
/*-LBRACKET-LBRACKET-LBRACKET-LBRACKET-LBRACKET-LBRACKET-LBRACKET-LBRACKET-*/
/*-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-LBRACE-*/

      case TOK_LPAREN:
      case TOK_LBRACKET:
      case TOK_LBRACE:

      /* Expecting a binary is either an error, or an indicator of  */
      /* separation between free format parameters                  */
	 if( expecting == OPTYPE_BINARY ) {
	     if( !PROCEEDING_WHITESPACE( tokstart ) ||
		 UNEXPECTED_NEW_EXPRESSION ) {
		 UNEXPECTED_LPAREN_ERROR( tokstart );
	     }

             /* End of an argument                     */
	     if( TopOfParenStack >= 0) {
	         ParenStack[ TopOfParenStack ].narg++;
	     }

	 }

      /* If left paren was encountered without a proceeding identifier   */
      /* it is not to be associated with a command.  Push it on the      */
      /* paren stack with a -1 indicator for the command id to show such.*/
      /* Push the negated value of the token type onto the operator      */
      /* stack, in order to distinguish it from the other operators.     */
	 PARENSTK_PUSH( toktype, -1, 1 );
	 OPERSTK_PUSH( -toktype, LEFTPAREN );

      /* Setup for unary prefix operator */
	 expecting = OPTYPE_UNARY_PREFIX;

	 break;


/*-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-RPAREN-*/
/*-RBRACKET-RBRACKET-RBRACKET-RBRACKET-RBRACKET-RBRACKET-RBRACKET-RBRACKET-*/
/*-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-RBRACE-*/

      case TOK_RPAREN:
      case TOK_RBRACKET:
      case TOK_RBRACE:

      /* Parenthesis integrity checking */
	 if( TopOfParenStack < 0  ||

#if 0
	     ParenStack[TopOfParenStack].narg == 0 &&
	     ParenStack[TopOfParenStack].nkey == 0 &&
	     expecting == OPTYPE_UNARY_PREFIX ||
#endif

	     toktype == TOK_RPAREN &&
		ParenStack[TopOfParenStack].paren_type != TOK_LPAREN ||

	     toktype == TOK_RBRACKET &&
		ParenStack[TopOfParenStack].paren_type != TOK_LBRACKET ||

	     toktype == TOK_RBRACE &&
		ParenStack[TopOfParenStack].paren_type != TOK_LBRACE ) {

	    UNEXPECTED_RPAREN_ERROR(toktype);
	    }

      /* Its an error if anything is on the operator stack, and a     */
      /* unary prefix was expected                                    */
	 if( OperStack[TopOfOperStack].oper_type == OPERATOR &&
	     expecting == OPTYPE_UNARY_PREFIX ) 
	    UNEXPECTED_RPAREN_ERROR(toktype);

      /* Take everything off of the operator stack until the matching */
      /* parenthesis is found.                                        */
	 while ( (OperStack[TopOfOperStack].oper_type != LEFTPAREN) &&
                 (TopOfOperStack >= 0 ))
         {
             if (OperStack[TopOfOperStack].oper_type == OPERATOR)
             {
                 OPERSTK_POP( temp_id );
                 PFOUT( TOK_COMMAND, opr_tbl_commid( temp_id ),
                      (opr_tbl_optype( temp_id ) == OPTYPE_BINARY ? 2 : 1), 0, 1 );
             }
             else
             {
                 OPERSTK_POP( temp_id );
                 PFOUT( TOK_KEYWORD, temp_id, 1, 0, 0);
             }
	 }
         OPERSTK_POP( temp_id );

      /* If this parenthesis corresponds to a command, set up the */
      /* the number of embedded arguments and keywords, and output */
	 if( ParenStack[TopOfParenStack].comm_id >= 0 )
         {
	    if( expecting == OPTYPE_BINARY) {
	      
	      /* End of an argument                     */
	      if( TopOfParenStack >= 0) {
		ParenStack[ TopOfParenStack ].narg++;
	      }
	    }

            if (ParenStack[TopOfParenStack].nids > 0)
            {
               int i, lp;

               lp = (ParenStack[TopOfParenStack].nids);

               for (i = 0; i < lp; i++)
               {
  	          PFOUT( TOK_COMMAND,
                     ParenStack[TopOfParenStack].comm_id,
                     ParenStack[TopOfParenStack].narg,
		     ParenStack[TopOfParenStack].nkey,
		     ParenStack[TopOfParenStack].nids );
                  --TopOfParenStack;
               }
            }
            else
            {
	       PFOUT( TOK_COMMAND, 
                   ParenStack[TopOfParenStack].comm_id,
		   ParenStack[TopOfParenStack].narg,
		   ParenStack[TopOfParenStack].nkey,
		   1);
               --TopOfParenStack;
            }
         }
         else
         {

	   PFOUT_LITERAL( TOK_BLANK, token );
	   --TopOfParenStack;
#if 0
	   if (TopOfParenStack>=0) {
	        ParenStack[TopOfParenStack].narg++;
	   }
#endif
         }

      /* Set up to expect a binary */
	 expecting = OPTYPE_BINARY;
	 break;


/*-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-EOF-*/

      case TOK_EOF:
	 PFOUT( TOK_EOF, 0, 0, 0, 0);
	 if( TopOfParenStack >= 0 ) {
	    COMERR_SET1( ERR_UNEXPECTED_EOF, *linenum );
	    goto error_return;
	    }
	 done = TRUE;
	 break;


/*-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-NONE-*/

      case TOK_NONE:
         done = TRUE;
         break;


	 }


   /* End of switch on token type           */
   /* --------------------------------------*/

   /* Make determination if expression is finished.  This happens under the */
   /* following conditions:                                                 */
   /*   1) All nested parenthesis are closed                                */
   /*   2) A binary operator is expected next                               */
   /*   3) The next token on the current line of input is separated by      */
   /*      whitespace from the current token.                               */
   /*   4) If the next token is an operator, it is not a binary operator    */
      if( !done && TopOfParenStack < 0 && expecting == OPTYPE_BINARY ) {

	 if( NextToklen == 0 ) 
	    LOOK_AHEAD;

	 if( NextToktype == TOK_KEYWORD ) {
	    COMERR_SET2( ERR_UNEXPECTED_KEYWORD, *linenum,
			 symbol_pntr( *(int *)NextToken ));
	    goto error_return;
	    }

	 if( RIGHT_PAREN( NextToktype ))
	    UNEXPECTED_RPAREN_ERROR( NextToktype );

	 if( NextToktype == TOK_EOF )
	    done = TRUE;

	 if( PROCEEDING_WHITESPACE( NextTokstart ) )

	    if( NextToktype != TOK_OPERATOR )
	       done = TRUE;

	    else if( NextOp_ids[OPTYPE_UNARY_PREFIX] >= 0 )
	       if( NextOp_ids[OPTYPE_BINARY ] >= 0 ||
		   NextOp_ids[OPTYPE_UNARY_POSTFIX ] ) {

	       /* If no trailing whitespace, then the current expression */
	       /* is done.  However, it is an error if no trailing       */
               /* whitespace is in itself caused by a string of ambiguous*/
               /* operators concatenated together                        */
		  if( !TRAILING_WHITESPACE( NextBufptr ) )  {
		     STRING_OF_AMBIGUITY_CHECK( ambiguous );
		     if( ambiguous ) {
			 COMERR_SET2( ERR_AMBIGUOUS_OPERATOR, *linenum,
				      NextToken );
			 goto error_return;
			 }
		     done = TRUE;
		     }

		  }
	       else
		  done = TRUE;
	 }

      }
   while( !done );
   _continuation_prompt = 0;

/* ----------------------------------------------------------------------- */
/* Done looping until an expression is finished                            */
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */

/* Raise exception if expression is incomplete */
   if (toktype == TOK_NONE) {
       if (infunc != _oneshot_infunc) {
           COMERR_SET( ERR_INCOMPLETE_EXPRESSION );
	   goto error_return;
       } else {
	   goto ok_return;
       }
   }

/* Put what remains on the operator stack onto the output */
   EMPTY_OPER_STACK;

ok_return:
   return FUNCOK;

error_return:

   NextToklen = 0;
   while( buffer[*bufptr] != '\0' )
      ++*bufptr;
   _continuation_prompt = 0;
   EMPTY_OPER_STACK;
   TopOfParenStack = -1;
   return FUNCBAD;

}

int print_pfbuf(pfbuf, index)
     postfix_token *pfbuf;
     int index;
{
    int counter;
    
    fprintf(stdout,"%d: ",index);
    
    switch( pfbuf[index].type )
	{
	case TOK_KEYWORD:
	    fprintf( stdout, "KEYWORD (%s)  narg: %d\n",
		     symbol_pntr( pfbuf[index].v.symbolid), 
		     pfbuf[index].narg);
	    break;
	    
	case TOK_COMMAND:
	    counter = fprintf( stdout, "COMMAND (%s",
			       symbol_pntr( pfbuf[index].v.symbolid ));
	    while( ++counter < 32 ) putc( ' ', stdout );
	    fprintf( stdout, ")   narg %d  nkey %d  nids: %d\n",
		     pfbuf[index].narg, pfbuf[index].nkey, pfbuf[index].nids );
	    break;
	    
	case TOK_STRING:
	    fprintf( stdout, "STRING (%s)\n",
		     symbol_pntr( pfbuf[index].v.symbolid ));
	    break;
	    
	case TOK_CHAR:
	    fprintf( stdout, "CHAR (%c:%d)\n", pfbuf[index].v.char_val,
		     pfbuf[index].v.char_val );
	    break;
	    
	case TOK_DFINT:
	    fprintf( stdout, "DFINT (" );
	    print_type( stdout, DFINTPAR, (char *)&pfbuf[index].v.dfintval, 0 );
	    fprintf( stdout, ")\n" );
	    break;
	    
	case TOK_DFUNS:
	    fprintf( stdout, "DFUNS (" );
	    print_type( stdout, DFUNSPAR, (char *)&pfbuf[index].v.dfunsval, 0 );
	    fprintf( stdout, ")\n" );
	    break;
	    
	case TOK_LNINT:
	    fprintf( stdout, "LNINT (" );
	    print_type( stdout, LNINTPAR, (char *)&pfbuf[index].v.lnintval, 0 );
	    fprintf( stdout, ")\n" );
	    break;
	    
	case TOK_LNUNS:
	    fprintf( stdout, "LNUNS (" );
	    print_type( stdout, LNUNSPAR, (char *)&pfbuf[index].v.lnunsval, 0 );
	    fprintf( stdout, ")\n" );
	    break;
	    
	case TOK_DOUBLE:
	    fprintf( stdout, "DOUBLE (" );
	    print_type( stdout, DOUBLPAR, (char *)&pfbuf[index].v.doublval, 0 );
	    fprintf( stdout, ")\n" );
	    break;
	    
	case TOK_EOF:
	    fprintf( stdout, "EOF\n" );
	    break;
	    
	case TOK_BLANK:
	    fprintf( stdout, "BLANK\n" );
	    break;
	    
	default:
	    break;
	}
    fflush( stdout );
    return(0);
}
