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
optable.c       COMTERP operator table support routines

Externals:      int opr_tbl_create(), int opr_tbl_insert(),
		int opr_tbl_print(), 
		int opr_tbl_entries(), int opr_tbl_operid(), 
		int opr_tbl_commid(), int opr_tbl_priority(),
		BOOLEAN opr_tbl_rtol(), int opr_tbl_optype(),
		int opr_tbl_default()

Summary:        

History:        Written by Scott E. Johnston, April 1989
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <ComUtil/comterp.h>

#define TITLE "optable"

/* Entry in operator table */
typedef struct _opr_tbl_entry opr_tbl_entry;
struct _opr_tbl_entry {
   int          operid;         /* Id for string that defines operator */
   int          commid;         /* Id for command associated with operator */
   unsigned     priority;       /* Relative priority of operator */
   BOOLEAN      rtol;           /* Flag for right-to-left operator associativity */
   unsigned     optype;         /* Type of operator:  OPTYPE_BINARY,
				   OPTYPE_UNARY_PREFIX, or OPTYPE_UNARY_POSTFIX */
   };

/* Static Variables */
static opr_tbl_entry            /* The operator table */
   *OperatorTable = NULL;
static unsigned NumOperators;	/* Number of operators currently in table */
static unsigned MaxOperators;	/* Maximum number of operators */
static unsigned MaxPriority;	/* Maximum priority encountered so far */
				/* Minimum can always be considered zero */
static int last_operid = -1;

#define OPSTR( index ) (symbol_pntr(OperatorTable[index].operid))
#define OPSTR_LEN( index ) (symbol_len(OperatorTable[index].operid))
#define COMMAND( index ) (symbol_pntr(OperatorTable[index].commid))

#define breakpt() {}

/* Default operator table */
struct _opr_tbl_default_entry {
  char *opchars;
  char *opname;
  unsigned priority;
  BOOLEAN rtol;
  unsigned optype;
} DefaultOperatorTable[] = {
  {".",          "dot",                130,        FALSE,      OPTYPE_BINARY },
  {"`",          "bquote",             125,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"$",          "stream",             125,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"!",          "negate",             110,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"~",          "bit_not",            110,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"++",         "incr",               110,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"++",         "incr_after",         110,        TRUE,       OPTYPE_UNARY_POSTFIX },
  {"-",          "minus",              110,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"--",         "decr",               110,        TRUE,       OPTYPE_UNARY_PREFIX },
  {"--",         "decr_after",         110,        TRUE,       OPTYPE_UNARY_POSTFIX },
  {"**",         "repeat",             90,         FALSE,      OPTYPE_BINARY },
  {"..",         "iterate",            80,         FALSE,      OPTYPE_BINARY },
  {"%",          "mod",                70,         FALSE,      OPTYPE_BINARY },
  {"*",          "mpy",                70,         FALSE,      OPTYPE_BINARY },
  {"/",          "div",                70,         FALSE,      OPTYPE_BINARY },
  {"+",          "add",                60,         FALSE,      OPTYPE_BINARY },
  {"-",          "sub",                60,         FALSE,      OPTYPE_BINARY },
  {"<<",         "lshift",             55,         FALSE,      OPTYPE_BINARY },
  {">>",         "rshift",             55,         FALSE,      OPTYPE_BINARY },
  {"<",          "lt",                 50,         FALSE,      OPTYPE_BINARY },
  {"<=",         "lt_or_eq",           50,         FALSE,      OPTYPE_BINARY },
  {">",          "gt",                 50,         FALSE,      OPTYPE_BINARY },
  {">=",         "gt_or_eq",           50,         FALSE,      OPTYPE_BINARY },
  {"!=",         "not_eq",             45,         FALSE,      OPTYPE_BINARY },
  {"==",         "eq",                 45,         FALSE,      OPTYPE_BINARY },
  {"&",          "bit_and",            44,         FALSE,      OPTYPE_BINARY },
  {"^",          "bit_xor",            43,         FALSE,      OPTYPE_BINARY },
  {"|",          "bit_or",             42,         FALSE,      OPTYPE_BINARY },
  {"&&",         "and",                41,         FALSE,      OPTYPE_BINARY },
  {"||",         "or",                 40,         FALSE,      OPTYPE_BINARY },
  {",",          "tuple",              35,         FALSE,      OPTYPE_BINARY },
  {",,",         "concat",             33,         FALSE,      OPTYPE_BINARY },
  {"%=",         "mod_assign",         30,         TRUE,       OPTYPE_BINARY },
  {"*=",         "mpy_assign",         30,         TRUE,       OPTYPE_BINARY },
  {"+=",         "add_assign",         30,         TRUE,       OPTYPE_BINARY },
  {"-=",         "sub_assign",         30,         TRUE,       OPTYPE_BINARY },
  {"/=",         "div_assign",         30,         TRUE,       OPTYPE_BINARY },
  {"=",          "assign",             30,         TRUE,       OPTYPE_BINARY },
  {";",          "seq",                10,         FALSE,      OPTYPE_BINARY },
};

/*!

opr_tbl_create   Create the operator table


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_create(unsigned  maxop)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        maxop     ;/* I   Maximum number of operators table 
				  will support. */
#endif


/*!
Description:

`opr_tbl_create` creates the operator table, initializing its contents to 
an empty state.  Does nothing if the table already exists.  Use 
`opr_tlb_delete` first if a new table is desired.

!*/

{
int index;

   if( OperatorTable != NULL ) 
      return FUNCOK;

/* Allocate memory for operator table */
   if( maxop == 0 )
      OperatorTable = NULL;
   else if( dmm_calloc( (void **)&OperatorTable, (long) maxop, 
                   sizeof( opr_tbl_entry ))) {
      COMERR_SET( ERR_MEMORY );
      return FUNCBAD;
      }

/* Initialize table to an empty state */
   NumOperators = 0;
   MaxOperators = maxop;
   MaxPriority  = 0;

   return FUNCOK;

}




/*!

opr_tbl_delete   Delete the existing operator table


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_delete()


/*!
Return Value:  0 if OK, -1 if Error


Parameters: none


Description:

`opr_tbl_delete` deletes the operator table, initializing its contents to 
an empty state.  If a previous operator table existed it is replaced by the 
new one. 

!*/

{
int index;

/* Free table if one existed previously */
   if( OperatorTable != NULL ) {
      for (index=0; index<NumOperators; index++ ) 
         if( symbol_del( OperatorTable[index].operid ) ||
	     symbol_del( OperatorTable[index].commid ))
            KAPUT( "Unable to delete symbol from table" );
      if( dmm_free( (void **)&OperatorTable ))
         KAPUT( "Error in freeing previously existing operator table" );

      }

   OperatorTable = NULL;    

   return FUNCOK;
}




/*!
opr_tbl_insert   Insert entry into operator table


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_insert(char * opstr,char * command,unsigned priority,
		   BOOLEAN rtol,unsigned  optype)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char *          opstr	  ;/* I   String of characters that define 
				  an operator. */
char *          command   ;/* I   Name of command associated with operator. */
unsigned        priority  ;/* I   Relative operator priority. */
BOOLEAN         rtol      ;/* I   Indicates whether operator associates 
                                  right-to-left or left-to-right. */
unsigned        optype    ;/* I   Type of operator:  OPTYPE_BINARY,
				  OPTYPE_UNARY_PREFIX, or OPTYPE_UNARY_POSTFIX
			          (constants defined in "comterp.ci"). */
#endif



/*!
Description:

`opr_tbl_insert` inserts an entry into the operator table.  `opstr` is the
string of characters that defines an operator, i.e. strings like "+",
"++", or "+=".  They must be special characters not previously reserved 
for in COMTERP, which means they cannot contain digits, letters, single colons, 
or any "parenthetical" character (i.e. "(", ")", "[", "]", "{", and "}").

`priority` specifies the relative priority of this operator.  `rtol`
indicates whether the operator associates right-to-left or left-to-right 
(the default).  `optype` selects whether the operator is a binary or 
unary operator, and if it a unary operator whether it is prefix or
postfix unary operator.

`opr_tbl_insert` can be called once for each possible operator type, i.e. for
the `-` operator it would be used once to set up the binary version,
and a second time to set up the prefix unary version.  If an operator
entry already exists for the specified `opstr` and operator type, it
will be replaced.

!*/

{
unsigned table_off = 0;         /* Offset into operator table */
BOOLEAN in_place = FALSE;       /* Indicates whether insertion is on top */
				/* existing operator */
BOOLEAN optype_exists[3];       /* Flags to indicate if specific operator of
				   given type already exists */
unsigned save_off;		/* Temporary store for table_off */
int operid;			/* Id of opstr in symbol table */
int commid;			/* Id of command in symbol table */

/* Check if table exists */
   if( OperatorTable == NULL ) {
      COMERR_SET( ERR_NO_OPTABLE );
      return FUNCBAD;
      }

/* Check that priority is in range */
   if( priority > 32767 ) {
      COMERR_SET1( ERR_PRIORITY_RANGE, priority );
      return FUNCBAD;
      }
   MaxPriority = max( MaxPriority, priority );

/* Search for place to insert operator */
   while( table_off < NumOperators &&
	  strcmp( OPSTR( table_off ), opstr ) < 0 )
      table_off++;

/* Check if operator already exists */
   if( table_off < NumOperators &&
       strcmp( OPSTR( table_off ), opstr ) == 0 ) {

   /* Set flags to indicate what operators of this type exists */
      optype_exists[OPTYPE_BINARY] = optype_exists[OPTYPE_UNARY_PREFIX] =
	 optype_exists[OPTYPE_UNARY_POSTFIX] = FALSE;
      save_off = table_off;
      while( table_off < NumOperators && 
	     strcmp( OPSTR( table_off ), opstr ) == 0 ) {
	 optype_exists[ OperatorTable[ table_off ].optype ] = TRUE;
	 table_off++;
	 }

   /* Position table offset at insert location */
      table_off = save_off;
      while( table_off < NumOperators &&
	     strcmp( OPSTR( table_off ), opstr ) == 0 &&
	     OperatorTable[ table_off ].optype < optype )
	 table_off++;

   /* Operator replaces existing one, clear out old entry */
      if( table_off < NumOperators &&
	  strcmp( OPSTR( table_off ), opstr ) == 0 &&
	  OperatorTable[ table_off ].optype == optype ) {
	 if( symbol_del( OperatorTable[ table_off ].operid ) ||
	     symbol_del( OperatorTable[ table_off ].commid ))
	    KAPUT( "Error in deleting symbols" );
	 in_place = TRUE;
	 }

   /* If not a replacement, ensure it is compatible with existing operators */
   /* The rule is a unary prefix operator can only be overdefined as a      */
   /* binary operator or unary postfix operator, but never both, and a      */
   /* binary operator can never be overdefined as a unary postfix operator  */
   /* (and visa versa) */
      else {
	 if( optype_exists[OPTYPE_UNARY_PREFIX] &&
	     optype_exists[OPTYPE_BINARY] ) {
	    COMERR_SET1( ERR_NO_POSTFIX, opstr );
	    return FUNCBAD;
	    }
	 else if( optype_exists[OPTYPE_UNARY_PREFIX] &&
		  optype_exists[OPTYPE_UNARY_POSTFIX] ) {
	    COMERR_SET1( ERR_NO_BINARY, opstr );
	    return FUNCBAD;
	    }
	 else if( optype_exists[OPTYPE_BINARY] &&
		  optype != OPTYPE_UNARY_PREFIX ) {
	    COMERR_SET1( ERR_NO_POSTFIX_WITH_BINARY, opstr );
	    return FUNCBAD;
	    }
	 else if( optype_exists[OPTYPE_UNARY_POSTFIX] &&
		  optype != OPTYPE_UNARY_PREFIX ) {
	    COMERR_SET1( ERR_NO_BINARY_WITH_POSTFIX, opstr );
	    return FUNCBAD;
	    }
	 }

      }

/* If one too many operators, inform the user */
   if( !in_place && NumOperators == MaxOperators ) {
      COMERR_SET1( ERR_OPRTBLMAXED, MaxOperators );
      return FUNCBAD;
      }

/* Slide the rest of the entries up by one */
   if( !in_place && table_off < NumOperators )
#ifndef DMM_OFF
      if( dmm_movrecs( (void **)&OperatorTable, (long)(table_off+1),
		   (long)(table_off), (long)(NumOperators-table_off)))
         KAPUT( "Error in attempt to move operator table entries" );
#else
      memmove((void*)(OperatorTable+table_off+1),
              (void*)(OperatorTable+table_off),
	      (NumOperators-table_off)*sizeof( opr_tbl_entry ));
#endif

/* Get symbol ids for operator and command strings */
   if(( operid = symbol_add( opstr )) < 0 ||
      ( commid = symbol_add( command )) < 0 ) {
      COMERR_SET( ERR_MEMORY );
      return FUNCBAD;
      }

/* Fill table entry */
   OperatorTable[ table_off ].operid     = operid;
   OperatorTable[ table_off ].commid     = commid;
   OperatorTable[ table_off ].priority   = priority;
   OperatorTable[ table_off ].rtol       = rtol; 
   OperatorTable[ table_off ].optype     = optype;
   last_operid = operid;

   if( !in_place )
      ++NumOperators;
   return FUNCOK;


}



/*!

opr_tbl_print   Print contents of operator table


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_print(FILE * outfile,unsigned  by)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
FILE *	        outfile   ;/* I   File pointer for output. */
unsigned        by        ;/* I   Ordering of output, by one of the following:
			          OPBY_OPERATOR, OPBY_COMMAND, or 
				  OPBY_PRIORITY (constants defined in
				  "comterp.ci"). */
#endif


/*!
Description:

`opr_tbl_print` prints the contents of the operator table to `outfile`, in 
one of three modes:  sorted by operator, sorted by command, or sorted by 
priority.


!*/

{
int index;
int counter;

/* Non-existent table */
   if( OperatorTable == NULL ) {
      fprintf( outfile, "Non-existent operator table\n" );
      return FUNCOK;
      }

/* Empty table */
   if( NumOperators == 0 ) {
      fprintf( outfile, "Empty operator table\n" );
      return FUNCOK;
      }

/* Print contents of table sorted by operator */
   fprintf( outfile, "Operator   Command            Priority   RtoL   Type\n" );
   fprintf( outfile, "--------   -------            --------   ----   ----\n" );
   for( index=0; index<NumOperators; index++ ) {
      counter = fprintf( outfile, "%s", OPSTR( index )); 
      while( counter++ < 11 ) putc( ' ', outfile );
      counter += fprintf( outfile, "%s", COMMAND( index ));
      while( counter++ < 31 ) putc( ' ', outfile );
      counter += fprintf( outfile, "%d", OperatorTable[ index ].priority );
      while( counter++ < 43 ) putc( ' ', outfile );
      fprintf( outfile, "%c      %s\n",
	 OperatorTable[ index ].rtol ? 'Y' : 'N',
	 OperatorTable[ index ].optype == OPTYPE_UNARY_POSTFIX
	    ? "UNARY POSTFIX"
	    : ( OperatorTable[ index ].optype == OPTYPE_UNARY_PREFIX
	       ? "UNARY PREFIX" : "BINARY" ));
      }

   return FUNCOK;

}





/*!

opr_tbl_entries   Return all ids associated with an operator


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_entries(char * buffer,int * op_ids,unsigned nop_ids,unsigned * nchar)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char *          buffer    ;/* I   Buffer to look for operator within. */
int  *          op_ids    ;/* I   Array of command symbol ids. */
unsigned	nop_ids   ;/* I   Size of `op_ids` (should be 3). */
unsigned *      nchar     ;/* O   Number of characters in operator */
#endif


/*!
Description:

`opr_tbl_entries` returns ids for all the operators in the table associated  
with a given operator string.  `op_ids` will contain one entry for each type of
operator:  OPTYPE_BINARY, OPTYPE_UNARY_PREFIX, OPTYPE_UNARY_POSTFIX
(defined in "ComUtil/comterp.h").  The operator is assumed to start at the first
character within `buffer`.

!*/

{
int op_index = 0;       /* Index into operator table */
int first_op;		/* First operator in table that matches character */
int index;

/* Error checking */
   if( nop_ids != 3 )
      KAPUT( "Number of operator ids must be 3" );

/* Null out operator ids */
   for( index=0; index<OPTYPE_NUM; index++ )
      op_ids[index] = -1;

/* Search for operator that begins with this character */
   while( op_index < NumOperators && 
          buffer[0] != *OPSTR( op_index ))
      op_index++;

/* At least one operator found that matches initial character */
   if( op_index < NumOperators ) {

   /* Find the longest operator that begins with this character */
      first_op = op_index;
      while( op_index < NumOperators &&
             buffer[0] == *OPSTR( op_index ))
         op_index++;
      op_index--;

   /* Backup until an exact match is found */
      while( op_index > first_op &&
	     strncmp( buffer, OPSTR( op_index ), OPSTR_LEN( op_index )) != 0 )
	 op_index--;

   /* No operator found that fits this character */
      if( strncmp( buffer, OPSTR( op_index ),
		   OPSTR_LEN( op_index )) != 0 ) {
	 COMERR_SET1( ERR_ILLEGALOP, *buffer );
	 return FUNCBAD;
	 }

   /* Backup further to the first version of this operator */
   /* At the same time load the command array */
      do {
	 op_ids[ OperatorTable[op_index].optype ] = op_index;
	 --op_index;
	 }
      while( op_index >= 0 &&
	     strcmp( OPSTR( op_index ), OPSTR( op_index+1 )) == 0 );

   /* Return number of characters taken for this operator */
      ++op_index;
      *nchar = OPSTR_LEN( op_index );

      }

/* No operator found that starts with initial character */
   else {
      COMERR_SET1( ERR_ILLEGALOP, *buffer );
      return FUNCBAD;
      }

   return FUNCOK;


}





/*!

opr_tbl_operid   Return id of operator symbol


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_operid(unsigned opnum)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        opnum     ;/* I   Number of operator in table. */
#endif


/*!
Description:

`opr_tbl_operid` returns the id of the operator symbol for a given 
entry in the operator table.

!*/

{
   if( opnum < NumOperators )
      return OperatorTable[opnum].operid;
   else
      return -1;
}





/*!

opr_tbl_commid   Return id of command symbol


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_commid(unsigned opnum)


/*!
Return Value:  0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        opnum     ;/* I   Number of operator in table. */
#endif

/*!
Description:

`opr_tbl_commid` returns the id of the command symbol for a given 
entry in the operator table.

!*/

{
   if( opnum < NumOperators )
      return OperatorTable[opnum].commid;
   else
      return -1;
}





/*!

opr_tbl_priority   Return priority of operator


Summary:


#include <ComUtil/comterp.h>
*/

int opr_tbl_priority(unsigned opnum)


/*!
Return Value:  priority if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        opnum     ;/* I   Number of operator in table. */
#endif

/*!
Description:

`opr_tbl_priority` returns the priority of an operator in the operator table.

!*/

{
   if( opnum < NumOperators )
      return OperatorTable[opnum].priority;
   else
      return -1;
}





/*!

opr_tbl_rtol   Return right-to-left indicator for an operator


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_rtol(unsigned opnum)


/*!
Return Value:  TRUE or FALSE if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        opnum     ;/* I   Number of operator in table. */
#endif

/*!
Description:

`opr_tbl_rtol` returns the right-to-left indicator of an operator in 
the operator table.

!*/

{
   if( opnum < NumOperators )
      return OperatorTable[opnum].rtol;
   else
      return -1;
}





/*!

opr_tbl_optype   Return type of operator


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_optype(unsigned opnum)


/*!
Return Value:  type of operator if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        opnum     ;/* I   Number of operator in table. */
#endif


/*!
Description:

`opr_tbl_optype` returns the type of an operator in the operator table:
OPTYPE_BINARY, OPTYPE_UNARY_PREFIX, OPTYPE_UNARY_POSTFIX (defined in
"ComUtil/comterp.h").

!*/

{
   if( opnum < NumOperators )
      return OperatorTable[opnum].optype;
   else
      return -1;
}





/*!

opr_tbl_maxprior   Return maximum priority in operator table


Summary:

#include <ComUtil/comterp.h>
*/

unsigned int opr_tbl_maxprior()


/*!
Return Value:  maximum priority in operator table


Parameters:  none


/*!
Description:

`opr_tbl_maxprior` returns the maximum priority of all operators currently
in the operator table.

!*/

{
   return MaxPriority;
}






/*!

opr_tbl_default   Build a default operator table for COMTERP


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_default()


/*!
Return Value:  0 if OK, -1 if Error


Parameters:  none


/*!
Description:

`opr_tbl_default` builds a default operator table for the COMTERP
language, as follows:

Operator   Command            Priority   RtoL   Type
--------   -------            --------   ----   ----
.          dot                130        N      BINARY
`          bquote             125        Y      UNARY PREFIX
$          stream             125        Y      UNARY PREFIX
!          negate             110        Y      UNARY PREFIX
~          bit_not            110        Y      UNARY PREFIX
++         incr               110        Y      UNARY PREFIX
++         incr_after         110        Y      UNARY POSTFIX
-          minus              110        Y      UNARY PREFIX
--         decr               110        Y      UNARY PREFIX
--         decr_after         110        Y      UNARY POSTFIX
**         repeat             90         N      BINARY
..         iterate            80         N      BINARY
%          mod                70         N      BINARY
*          mpy                70         N      BINARY
/          div                70         N      BINARY
+          add                60         N      BINARY
-          sub                60         N      BINARY
<<         lshift             55         N      BINARY
>>         rshift             55         N      BINARY
<          lt                 50         N      BINARY
<=         lt_or_eq           50         N      BINARY
>          gt                 50         N      BINARY
>=         gt_or_eq           50         N      BINARY
!=         not_eq             45         N      BINARY
==         eq                 45         N      BINARY
&          bit_and            44         N      BINARY
^          bit_xor            43         N      BINARY
|          bit_or             42         N      BINARY
&&         and                41         N      BINARY
||         or                 40         N      BINARY
,          tuple              35         N      BINARY
,,         concat             33         N      BINARY
%=         mod_assign         30         Y      BINARY
*=         mpy_assign         30         Y      BINARY
+=         add_assign         30         Y      BINARY
-=         sub_assign         30         Y      BINARY
/=         div_assign         30         Y      BINARY
=          assign             30         Y      BINARY
;          seq                10         N      BINARY


!*/

{
  int table_size = sizeof( DefaultOperatorTable ) / 
    sizeof( struct _opr_tbl_default_entry );
  int index;

  if (OperatorTable)
     return 0;

  /* Initalize table to the right size */
  if( opr_tbl_create( table_size ) != 0 )
     KAPUT( "Unable to create default operator table" );

  /* Fill it up */
  for( index=0; index<table_size; index++ ) 
     if( opr_tbl_insert( DefaultOperatorTable[index].opchars,
                         DefaultOperatorTable[index].opname, 
			 DefaultOperatorTable[index].priority,
                         DefaultOperatorTable[index].rtol,
			 DefaultOperatorTable[index].optype ) != 0 )
        KAPUT1( "Unable to add the %d entry to the default operator table", index );

  return 0;

}

/*!

opr_tbl_opstr   Return operator string given a command symbol


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_opstr(unsigned commid)


/*!
Return Value:  >=0 if OK, -1 if Error


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned        commid     ;/* I  Symbol id of command name. */
#endif

/*!
Description:

`opr_tbl_opstr` returns the id of the operator symbol that matches
the command referred to by commid.

!*/

{
   int opnum = 0;
   while ( opnum < NumOperators ) {
      if (opr_tbl_commid(opnum)==commid)
	return opnum;
      opnum++;
   }
   return -1;
}


/*!

opr_tbl_topstr   Return symbol id of last operator inserted


Summary:

#include <ComUtil/comterp.h>
*/

int opr_tbl_topstr()


/*!
Return Value:  >=0 if OK, -1 if Error


Parameters: none
*/


/*!
Description:

`opr_tbl_opstr` returns the id of the operator symbol that was registered lastmatches
the command referred to by commid.

!*/

{
   return last_operid;
}





