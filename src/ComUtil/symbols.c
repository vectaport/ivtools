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
SYMBOLS.C	Symbol Table Functions.

Externals:	

Summary:        These functions support the structuring and accessing
		of a string based symbol table.  It allows you to add,
		delete and find symbols of any size.

History:        Written by Robert C. Fitch, 10 April 1989
*/

#include        <stdio.h>
#include	<string.h>

#include        "comutil.ci"
/* comutil.ci includes ComUtil/comutil.h, util.h,stdlib.h and malloc.h */

/*=============*/

typedef struct _symid symid;
struct _symid   {
		  unsigned nchars;	/* number of characters is a string */
		  int offset;     	/* offset into symbol table */
					/* -1 means available entry */
		  unsigned instances;   /* instances of this symbol */
		};
/* instances is incremented every symbol_add() and decremented every */
/* symbol_del().  When it is zero after a symbol_del() the symbol is removed */

/*=============*/

/* Static Functions: */

/* some pointers for the different open lists */

static char *sym_beg=NULL;  /* starting address of symbol table array */
				/* NULL means not allocated yet */
static unsigned sym_used;   /* Bytes used in symbol table so far */
static unsigned sym_nbytes;  /* number of total bytes in symbol table */

static symid *symid_beg=NULL;	/* start addr of symbol table id's */
static unsigned symid_nrecs;	/* number of records in the table */

static int sym_alloc_num;
static int symid_alloc_num;
#define LOTS_OF_MEM	32000
/* These are used when you don't have much memory */
#define SYM_ALLOC_NUM_LOW	  512
#define SYMID_ALLOC_NUM_LOW        32
/* These are used when you have lots of memory */
#define SYM_ALLOC_NUM_HIGH	 4096
#define SYMID_ALLOC_NUM_HIGH      256

/*=============*/

/*!

symbol_add	Add a new symbol to the symbol table.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_add (char * string)

/*!
Return Value:  >= 0 unique identifier for this symbol, 
               -1 if insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char	      * string   ;/*  I    NULL terminated string to add */
#endif

#ifdef DOC
/*!
Description:

Adds a symbol to the symbol table.  The symbol must be a NULL terminated
string in the `string` parameter.  

Everytime a symbol is added that already exists, the
`instances` element of the `symid` structure is incremented.  This prevents
subsequent deletes for new instances of a symbol from removing the symbol
entry until all instances are deleted.  It is essentially the difference
of the number of times a symbol was added and the number of times it was
deleted.

All symbols are maintained in a character array allocated by this routine.  
Each symbol is identified by a unique identifier that is an index into the
`symid` structure.  This `symid` structure contains the number of characters
in the string (not including the NULL terminator) and the offset to the
start of the string in the symbol character array.  The identifier cannot be
the offset into the array since the positions of the strings may be moved
due to packing the array after symbols are deleted.

The string array and the `symid` structure array are both dynamically and
automatically allocated by these routines as needed.  

See Also:  symbol_del(), symbol_find()

Example:

main() 
{
	int id1,id2,id3,id4,id5;

	id1 = symbol_add("symbol1");
	id2 = symbol_add("symbol2");
	id3 = symbol_add("symbol3");
	id4 = symbol_add("symbol4");

	/* find the third symbol */
	if (symbol_find("symbol3") < 0)
	   printf("ERROR: Can't find symbol3\n");
	/* Print number of chars in symbol */
	printf("symbol2 has %d characters\n",symbol_len(id2));
	/* return a pointer to symbol4 and print it */
	printf("Returned string for symbol4 is %s\n",symbol_pntr(id4));
	/* now delete the second symbol */
	symbol_del(id2);
	/* add symbol 4 again using symbol_id() */
	symbol_id("symbol4");
	/* delete symbol 4; it should not delete it since there are */
	/* two instances */
  	symbol_del(id4);
	/* make sure you can find symbol 4 */
	if (symbol_find("symbol4") < 0)
	   printf("ERROR: Can't find symbol4\n");
        /* now delete symbol 4 id5 */
	symbol_del(id5);
	/* now symbol 4 should be gone */
	if (symbol_find("symbol4") < 0)
	   printf("ERROR: Symbol 4 should have been deleted but was found\n");

}

#endif /* DOC */
/*
!*/

{
  int retval = -1;
  unsigned bytes_left;
  unsigned n;
  int id,found;
  symid *pntr;
  unsigned long nbytes;
  int i;

/* don't allow NULL input string */
  n = strlen(string);
  if (string == NULL)
	goto error_return; /* can't be NULL,zero len or too big */

  if ( (id = symbol_find(string)) >= 0)	/* found it */
  {	/* found an existing string; increment instances */
      pntr = symid_beg + id;
      (pntr->instances)++;	/* increment instances */
  }
  else 	/* you have to add the symbol */
  {
    if (sym_beg == NULL)	/* if NULL, allocate some space for symbol table */
    {
      dmm_mblock_stats(NULL,&nbytes,NULL,NULL,NULL);
      sym_alloc_num = (nbytes < LOTS_OF_MEM) ? SYM_ALLOC_NUM_LOW:
					       SYM_ALLOC_NUM_HIGH;
      if ( dmm_calloc((void**)&sym_beg,(long) sym_alloc_num,sizeof(char)) != 0)
           goto error_return;	/* goto error return; INSUFFICIENT MEMORY */
      sym_nbytes = sym_alloc_num;
      sym_used = 0;
      symid_alloc_num = (nbytes < LOTS_OF_MEM) ? SYMID_ALLOC_NUM_LOW:
					         SYMID_ALLOC_NUM_HIGH;
      if ( dmm_calloc((void**)&symid_beg,(long) symid_alloc_num,sizeof(symid)) != 0)
         goto error_return;	/* goto error return; INSUFFICIENT MEMORY */
      for (i=0; i<symid_alloc_num; i++) 
	 symid_beg[i].offset = -1;  /* set unused */
      symid_nrecs = symid_alloc_num;
    }
    if (n > sym_alloc_num)
      goto error_return;	/* bigger than can be allocated */
    bytes_left = sym_nbytes - sym_used;
    if (bytes_left < (n+1)) /* if not enough room left in the symbol array */
    {	/* realloc some more */
      dmm_realloc_size(sizeof(char));
      if ( dmm_realloc((void**)&sym_beg,(long) (sym_nbytes + sym_alloc_num)) != 0)
	  goto error_return;
      sym_nbytes += sym_alloc_num;
    }
    /* hunt for empty cell in symid array */
    for (id=found=0, pntr=symid_beg; id<symid_nrecs; id++,pntr++)
    {
      if (pntr->offset == -1)	/* found one, break */
      {
	 found = 1;
         break;
      }
    }
    if (!found)		/* have to realloc some more symid table space */
    {	/* realloc some more */
      dmm_realloc_size(sizeof(symid));
      if ( dmm_realloc((void**)&symid_beg,(long) (symid_nrecs + symid_alloc_num)) != 0)
	  goto error_return;
      id = symid_nrecs;	/* first new one allocated */
      symid_nrecs += symid_alloc_num;
      for (i=id; i < symid_nrecs; i++)
 	  symid_beg[i].offset = -1;  /* set unused */
    }
    pntr = symid_beg + id;	/* index into entry of interest */
    pntr->nchars = n;		/* number of non-NULL characters */
    pntr->offset = sym_used;	/* start of string */
    pntr->instances = 1;		/* set first instance of this symbol */
    strcpy(sym_beg+sym_used,string);	/* copy in string */
    sym_used += (n+1);		/* update used byte total; add in NULL byte */
  }
  retval = id;
error_return:		/* return an error code */
  return (retval);
}

/*!

symbol_del	Delete a symbol in the symbol table.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_del (int id)

/*!
Return Value:  0 if OK, -1 if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I   Identifier returned by symbol_add() */
#endif

/*!
Description:

Deletes a symbol previously added with a `symbol_add()` function call.  You
can delete a symbol by its string also by first using a `symbol_find()` call
that returns the id.  The symbol will not be deleted until the `instances`
element of the `symid` structure goes to zero.

Currently this routine will repack the memory everytime a deletion is made.
This is fairly efficient since the entire block can be moved down in one
call versus a fragemented packing approach.  It can be changed to something
more predicatable later.

See Also:  symbol_add(), symbol_find()

!*/
{
   symid *pntr;
   int retval = FUNCBAD;
   unsigned n,offset;
   unsigned bytes_left;

   if(sym_beg == NULL || id < 0 || id >= symid_nrecs) /* params out of range */
      goto error_return;
   if (	(n=(pntr = symid_beg+id)->offset) != -1 &&  /* already deleted */
        --(pntr->instances) == 0)	/* only delete if instances zeroed */
   {	/* this will delete that actual symbol in the string table */
     offset = pntr->offset;
     pntr->offset = -1;		/* set this one empty and available */
     /* go through every entry in the symid table and subtract n+1 from */
     /* everyone with an offset > offset */
     for (id=0, pntr=symid_beg; id<symid_nrecs; id++,pntr++)
     {
        if (pntr->offset != -1 && pntr->offset > offset)
    	    pntr->offset -= (n+1);	/* +1 for NULL byte at the end */
     }
     /* now pack memory down */
     MEMCPY(sym_beg+offset,sym_beg+(offset+n+1),sym_used-(offset+n+1));
     sym_used -= (n+1);	/* adjust size down */
     /* now shrink size of symbol string table if too much space left */
     bytes_left = sym_nbytes - sym_used;	/* bytes remaining */
     if (bytes_left >= (sym_alloc_num << 1) ) /* same as times 2 */
     { /* resize if bigger than two blocks; 1 block hysteresis */
       dmm_realloc_size(sizeof(char));
       if ( dmm_realloc((void**)&sym_beg,(long) (sym_nbytes - sym_alloc_num)) != 0)
	   goto error_return;
       sym_nbytes -= sym_alloc_num;
     }
     /* can't do much with resizing symid table, sorry */
   } 
   retval = FUNCOK;
error_return:		/* return an error code */
   return (retval);
}

/*!

symbol_find	Find a symbol string in the symbol table and return id.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_find (char * string)

/*!
Return Value:  >=0 unique identifier for symbol if symbol found,
               -1 if symbol does not exist.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
char	      * string   ;/*  I    NULL terminated symbol string to find */
#endif

/*!
Description:

Finds a symbol already in the symbol table that matches `string`.  Will return
the id of the symbol if found and -1 if not found.

See Also:  symbol_add(), symbol_del()

!*/
{
   symid *pntr;
   unsigned n;
   int retval = -1;
   int id;

   n = strlen(string);
   if (sym_beg == NULL || string == NULL)
      goto error_return;
   /* hunt through the symid table for this string */
   for (id=0, pntr=symid_beg; id<symid_nrecs; id++,pntr++)
   {
      if (pntr->offset != -1 && pntr->nchars == n &&
	  strcmp(string,sym_beg+pntr->offset) == 0)
      {	/* all equal, I found it */
	retval = id;	/* this id is return value */
        break;
      }
   }
error_return:		/* return an error code */
   return (retval);
}


/*!

symbol_pntr	Return a pointer to the NULL terminated symbol.

Summary:

#include <ComUtil/comutil.h>
*/

char * symbol_pntr (int id)

/*!
Return Value:  Pointer to NULL terminated string if OK, NULL if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I   Identifier returned by symbol_add() */
#endif


/*!
Description:

Returns a pointer to the symbol string for the `id` parameter.  CAUTION:
DO NOT MODIFY THE STRING.  May change this to duplicate the string in the
future.

See Also:  symbol_add(), symbol_len()

!*/
{
   symid *pntr;

   if (sym_beg == NULL || id < 0 || id >= symid_nrecs ||
       (pntr = symid_beg+id)->offset == -1)
         return(NULL);
   return(sym_beg + pntr->offset);
}


/*!

symbol_len	Return the number of characters in a symbol.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_len (int id)

/*!
Return Value:  >0 is number of chars in symbol if OK, -1 if none exists.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I   Identifier returned by symbol_add() */
#endif


/*!
Description:

Returns the number of characters in a symbol.  This does not include any
NULL terminator character.

See Also:  symbol_add(), symbol_pntr()

!*/
{
   symid *pntr;

   if (sym_beg == NULL || id < 0 || id >= symid_nrecs ||
	(pntr = symid_beg+id)->offset == -1)
     return(-1);

   return(pntr->nchars);
}

/*!

symbol_instances	Return the number of instances of a symbol.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_instances (int id)

/*!
Return Value:  >=0 is number of instances of symbol if OK, -1 if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I   Identifier returned by symbol_add() */
#endif


/*!
Description:

Returns the number of current instances of the symbol identified by id.  This
is how many times the same symbol has been added minus the times it has been
deleted.  

Errors returned if the symbol table is empty or `id` is illegal.

See Also:  symbol_add(), symbol_pntr()

!*/
{
   symid *pntr;

   if (sym_beg == NULL || id < 0 || id >= symid_nrecs)
     return(-1);		/* error */
   if ( (pntr = symid_beg+id)->offset == -1)
     return(0);		/* no instances */
   return(pntr->instances);
}

