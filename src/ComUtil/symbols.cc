/*
 * Copyright (c) 1993-1995 Vectaport Inc.
 * Copyright (c) 1989 Triple Vision, Inc.
 * Copyright (c) 2010 Wave Semiconductor Inc.
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
                Complete rewritten by Scott Johnston, November 2010
*/

#include        <stdio.h>
#include	<string.h>
#include        <errno.h>

#include        "comutil.ci"
/* comutil.ci includes ComUtil/comutil.h, util.h,stdlib.h and malloc.h */

/*=============*/

typedef struct _symid symid;
struct _symid   {
		  unsigned nchars;	/* number of characters is a string */
                  char* symstr;         /* pointer to character string */
					/* NULL means available entry */
		  unsigned instances;   /* instances of this symbol */
		};
/* instances is incremented every symbol_add() and decremented every */
/* symbol_del().  When it is zero after a symbol_del() the symbol is removed */

/*=============*/

/* Static Functions: */

/* some pointers for the different open lists */

static symid *symid_beg=NULL;	/* start addr of symbol table id's */
static unsigned symid_nrecs;	/* number of records in the table */

static int symid_alloc_num;
#define LOTS_OF_MEM	32000
/* These are used when you don't have much memory */
#define SYMID_ALLOC_NUM_LOW        32
/* These are used when you have lots of memory */
#define SYMID_ALLOC_NUM_HIGH      256


/* reverse index to speed up symbol table insertion */
#define REVERSE_TABLE
#if defined(REVERSE_TABLE)
#include <OS/strtable.h>
declareStrTable(ReverseSymbolTable,int)
implementStrTable(ReverseSymbolTable,int)
ReverseSymbolTable* _reverse_table = nil;
#endif

/*=============*/

/*!

symbol_add	Add a new symbol to the symbol table.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_add (const char * string)

/*!
Return Value:  >= 0 unique identifier for this symbol, 
               -1 if insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
const char      * string   ;/*  I    NULL terminated string to add */
#endif

#ifdef DOC
/*!
Description:

Adds a symbol to the symbol table.  The symbol must be a NULL terminated
string in the `string` parameter.  

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
  int n;
  int id;
  symid* pntr;

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
    int found;

    if (symid_beg == NULL)	/* if NULL, allocate some space for symbol table */
    {
      symid_alloc_num = SYMID_ALLOC_NUM_HIGH;
      if ( dmm_calloc((void**)&symid_beg,(long) symid_alloc_num,sizeof(symid)) != 0)
         goto error_return;	/* goto error return; INSUFFICIENT MEMORY */
      for (int i=0; i<symid_alloc_num; i++) 
        symid_beg[i].symstr = NULL;;  /* set unused */
      symid_nrecs = symid_alloc_num;
    }

    /* hunt for empty cell in symid array */
    for (id=found=0, pntr=symid_beg; id<symid_nrecs; id++,pntr++)
    {
      if (pntr->symstr == NULL)	/* found one, break */
      {
	 found = 1;
         break;
      }
    }
    if (!found)		/* have to realloc some more symid table space */
    {	/* realloc some more */
      symid* symid_beg_after;
      if(!(symid_beg_after = (symid*)realloc(symid_beg, (long) (symid_nrecs + symid_alloc_num) * sizeof(symid))))
          goto error_return;
      symid_beg = symid_beg_after;
      id = symid_nrecs;	/* first new one allocated */
      symid_nrecs += symid_alloc_num;
      for (int i=id; i < symid_nrecs; i++)
        symid_beg[i].symstr = NULL;  /* set unused */
    }
    pntr = symid_beg + id;	/* index into entry of interest */
    pntr->nchars = n;		/* number of non-NULL characters */
    pntr->symstr = strdup(string);	/* make copy of string */
    pntr->instances = 1;		/* set first instance of this symbol */

    /* add to reverse index */
    if(!_reverse_table) _reverse_table = new ReverseSymbolTable(1024);
    _reverse_table->insert(pntr->symstr, id);

  }
  return id;
error_return:		/* return an error code */
  return -1;
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

See Also:  symbol_add(), symbol_find()

!*/
{
   symid *pntr;
   int retval = FUNCBAD;

   if(symid_beg == NULL || id < 0 || id >= symid_nrecs) /* params out of range */
      goto error_return;
   if (	((pntr = symid_beg+id)->symstr) != NULL &&  /* already deleted */
        --(pntr->instances) == 0)	/* only delete if instances zeroed */
   {	/* this will delete that actual symbol in the string table */
     _reverse_table->remove(pntr->symstr);
     free(pntr->symstr);
     pntr->symstr = NULL;
   } 
   retval = FUNCOK;
error_return:		/* return an error code */
   return (retval);
}

/*!

symbol_reference	Increment reference counter for a symbol in the symbol table.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_reference (int id)

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

Increments a reference counter for a symbol previously added with a `symbol_add()` 
function call.

See Also:  symbol_add(), symbol_del()

!*/
{
   symid *pntr;
   int retval = FUNCBAD;
   unsigned n,offset;
   unsigned bytes_left;

   if(symid_beg == NULL || id < 0 || id >= symid_nrecs) /* params out of range */
      goto error_return;
   if (	(pntr = symid_beg+id)->symstr != NULL)
	++(pntr->instances);
   else
     goto error_return;
   retval = FUNCOK;

error_return:		/* return an error code */
   return (retval);
}

/*!

symbol_refcount	  Return current symbol reference count

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_refcount (int id)

/*!
Return Value:  reference count, -1 if error

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I   Identifier returned by symbol_add() */
#endif

/*!
Description:

Returns current value of a reference counter for a symbol previously added with a `symbol_add()` 
function call.

See Also:  symbol_reference()

!*/
{
   symid *pntr;
   int retval = FUNCBAD;
   unsigned n,offset;
   unsigned bytes_left;

   if(symid_beg == NULL || id < 0 || id >= symid_nrecs) /* params out of range */
      goto error_return;
   if (	(pntr = symid_beg+id)->symstr != NULL)
	return pntr->instances;
   else
     goto error_return;

error_return:		/* return an error code */
   return -1;
}

/*!

symbol_find	Find a symbol string in the symbol table and return id.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_find (const char * string)

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
  if (!_reverse_table) return -1;
  int sym;
  if(_reverse_table->find(sym, string))
    return sym;
  else
    return -1;
}


/*!

symbol_pntr	Return a pointer to the NULL terminated symbol.

Summary:

#include <ComUtil/comutil.h>
*/

const char * symbol_pntr (int id)

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
DO NOT MODIFY THE STRING.  

See Also:  symbol_add(), symbol_len()

!*/
{
   symid *pntr;

   if (symid_beg == NULL || id < 0 || id >= symid_nrecs || (pntr = symid_beg+id)->symstr == NULL)
       return(NULL);
   return(pntr->symstr);
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

   if (symid_beg == NULL || id < 0 || id >= symid_nrecs ||
	(pntr = symid_beg+id)->symstr == NULL)
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

   if (symid_beg == NULL || id < 0 || id >= symid_nrecs)
     return(-1);		/* error */
   if ( (pntr = symid_beg+id)->symstr == NULL)
     return(0);		/* no instances */
   return(pntr->instances);
}

/*!

symbol_max	Maximum number of currently available symbols.

Summary:

#include <ComUtil/comutil.h>
*/

int symbol_max ()

/*!
Return Value:  current size of symbol table.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
#endif


/*!
Description:

Returns the current size of the symbol table.

See Also:  symbol_instances()

!*/
{
  return(symid_nrecs);
}
