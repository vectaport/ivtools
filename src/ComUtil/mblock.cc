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
MBLOCK.C	Allocate a memory block and return a unique identifier.

Externals:	

Summary:        Functions to support memory allocation and return unique
		integer identifiers to be used to point to the block.  The
		identifier allows the memory block to be moved but still
		have the same identifier.  A pointer to the start of the
		block would not be guaranteed to point always to the same place.

History:        Written by Robert C. Fitch, 26 Aug 1989
*/


#include        <stdio.h>

#include        "comutil.ci"
/* comutil.ci includes stdlib.h and malloc.h */

#ifndef DMM_OFF

/* This contains all information for an open mblock */

typedef struct  {
		  unsigned nel; /* number of elements in the block */
		  unsigned size; /* size of the element in bytes */
			/* 0 size means this id cell is available */
		  char *head;		/* pointer to the start of the block */
		} mblock;

/* some pointers for the different open mblocks */

static mblock *mblock_head,*mblock_curr;        /* updatable pointers */
static int nmblocks=0;       /* number of lists in mblock id array */

/* this makes sure an id is valid and the entry has an allocated mem block */
#define CHECKID {\
if(id < 0 || id >= nmblocks) goto error_return;\
mblock_curr=mblock_head+id;\
if(mblock_curr->size == 0) goto error_return; }


/*!

mblock_open	Open a memory block of specified size.

Summary:

#include <ComUtil/comutil.h>
*/

int mblock_open (unsigned nel,unsigned size)

/*!
Return Value:  != -1 identifier for this mblock structure, -1 if 
		insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned	nel    ;/*     I  number of elements to allocate */
unsigned	size   ;/*     I  number of bytes per element */
#endif

#ifdef DOC
/*!
Description:

Allocates an area of memory (using the dmm_calloc() function) and returns
a unique integer identifier that can be used to reference the starting
address of the memory block.  NOTE: it is ok to specify a memory block with
`nel` of zero and increase its size later using mblock_resize(), but the
`size` must always be greater than zero.


See Also:  mblock_close(), mblock_pntr(), mblock_resize().


Example:

main() 
{
	int id1, id2
	char *pntr1;
	int  *pntr2;
	unsigned nel;
	unsigned long nbytes1,nbytes2;

	extern void *mblock_pntr();

	/* open a memory block with 2000 bytes in it */

	if ( (id1 = mblock_open(2000,sizeof(char))) == -1)
	{
	  fprintf(stderr,"Couldn't open memory block #1\n");
	  return(-1);
	}

	/* open a 2nd memory block with 1000 integers */

	if ( (id1 = mblock_open(1000,sizeof(int))) == -1)
	{
	  fprintf(stderr,"Couldn't open memory block #2\n");
	  return(-1);
	}

	/* get the starting address of the memory blocks */
	pntr1 = (char *) mblock_pntr(id1);
	pntr2 = (int *) mblock_pntr(id2);

	/* get the size of the memory blocks in bytes and print out */
	/* get the number of elements in the mblock and the total */
	/* number of bytes used for block 1 but do not get size of element */
	/* get just the total bytes for block 2 */

	mblock_sizes(id1,&nel1,NULL,&nbytes1);	/* NULL says don't return it */
	mblock_sizes(id2,NULL,NULL,&nbytes2);

	printf("Mblock 1 id# %d has %u records and %lu bytes\n",
		id1,nel,nbytes1);
	printf("Mblock 2 id# %d has %lu bytes\n",id2,nbytes2);

	/* increase the size of the second mem block to 3000 integers */

	if ( mblock_resize(id2,3000)) != 0)
	{
	  fprintf(stderr,"Couldn't increase size of memory block #2\n");
	  return(-1);
	}

	/* close both memory blocks */

	mblock_close(id1);
	mblock_close(id2);
}
!*/
#endif /* DOC */

{
  int id,found;
  int retval = -1;

#define  ALLOC_NUM	4		/* alloc 4 id blocks at a time */

  if (nmblocks <= 0)    /* see if no callocing has been done yet */
  {	/* calloc ALLOC_NUM entires in mblock structure array */
    if ( dmm_calloc((void **)&mblock_head,(long)ALLOC_NUM,sizeof(mblock)) != 0)
       goto error_return;	/* goto error return; no message */
    /* declare pointers */
    if ( dmm_pntr_alloc(&mblock_head->head) != 0 ||
         dmm_pntr_scalar(&mblock_curr) != 0 )
    {
	dmm_free((void **)&mblock_head);/* free mblock and all alloc pointers */
	goto error_return;
    }
    mblock_curr = mblock_head;	/* set to be the same */
    id = 0;		/* set id to top of this list */
    /* zeroes automatically null out everything */
    nmblocks = ALLOC_NUM;		/* One mblock so far */
  }
  else	/* nmblocks > 0 */
  {
    /* first search current list for any empty ones and use them first */
    for (id=found=0, mblock_curr=mblock_head; id < nmblocks; 
			id++,mblock_curr++)
    {
      if (mblock_curr->size == 0)  	/* is it empty ?? */
      {
	found = 1;	/* set this flag true;OS9 didn't like it in 'if ()' */
        break;		/* found one, break for loop and use it */
      }
    }
    if (!found)		/* true if no empty ones found */
    {			/* which means you have to alloc some more space */
      /* dmm_realloc() also clears new memory locations, so don't worry */
      /* about them being zero; they are */
      if ( dmm_realloc((void **)&mblock_head,(long)nmblocks+ALLOC_NUM) != 0)
         goto error_return;	/* just return an error; no message */
				/* can't have any more mblocks */
      mblock_curr = mblock_head + nmblocks;   /* position of newest entry */
      mblock_curr->size = 0;	/* make it empty */
      id = nmblocks;	/* use this as id, then incr number of mblocks by 1 */
      nmblocks += ALLOC_NUM;	/* update the total */
    }
  }
  /* at this point id is the identifying index and mblock_curr points to */
  /* an available mblock entry.  Now calloc the mblock memory */
  /* both structure arrays are cleared by calloc's which will automatically */
  /* make all of their entries empty */
  if ( dmm_calloc((void **)&mblock_curr->head,(long)nel,size) != 0)
     goto error_return;	/* goto error return; no message */

  /* set the sizes into the list and all done */
  mblock_curr->nel = nel;
  mblock_curr->size = size;

  retval = id;
error_return:		/* return an error code */
  return (retval);
}

/*!

mblock_close	Close the specified or all mblocks.

Summary:

#include <ComUtil/comutil.h>
*/

int mblock_close (int id)

/*!
Return Value:  0 if close OK, -1 if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id       ;/*   I  id returned by mblock_open() to close or
				  -1 if all mblocks are to be closed */
#endif

/*!
Description:

Close the specified mblock.  If id is -1,then close all open mblocks.

See also:  mblock_open()

Example:   See end of example in mblock_open ().

!*/
{
   int i;
   int retval = -1;

   if (id != -1)	/* just close one */
   {
     if (id >= nmblocks)	/* error if out of range */
	goto error_return;
     i = id;
   }
   else
     i = 0;
   for (mblock_curr=mblock_head+i; i<nmblocks; i++,mblock_curr++)
   {
     if ( mblock_curr->size > 0  )   /* see if opened */
     {
          mblock_curr->size = 0;	/* free this entry */
          /* it is open, free internal pointers */
	  dmm_free ((void **)&mblock_curr->head);
     }
     if (id != -1)	/* if only closing 1, exit */
       break;
   }
   retval = 0;
error_return:
   return (retval);  /* error return */
}

/*!

mblock_resize	Change the size of an already open'd mblock

Summary:

#include <ComUtil/comutil.h>
*/

int mblock_resize (int id,unsigned nel)

/*!
Return Value:  != -1 identifier for this mblock structure, -1 if 
		insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id      ;/*    I  mblock identifier from mblock_open() */
unsigned	nel    ;/*     I  number of elements to change to */
#endif

/*!
Description:

Changes the size of a memory block already allocated.  The number of elements
to change to is specified by `nel`.  The size of each element stays the same as
specified in mblock_open().  

See Also:  mblock_open()


Example:

!*/

{
  int retval = -1;

  CHECKID;      /* make sure id is valid */
  if ( dmm_realloc((void **)&mblock_curr->head,(long)nel) != 0)
    goto error_return;	/* just return an error; no message */
  mblock_curr->nel = nel;	/* set new size in id structure */
  retval = 0;
error_return:		/* return an error code */
  return (retval);
}

/*!

mblock_pntr		Return pointer in specified mblock.

Summary:

#include <ComUtil/comutil.h>
*/

void *mblock_pntr(int id)

/*!
Return Value:  pntr to mblock position if OK, NULL if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id      ;/*    I  mblock identifier from mblock_open() */
#endif

/*!
Description:

Returns a pointer to the position in the mblock specified by the `id`.
A NULL is returned if the block is not in use.

See also: 

Example:   See end of example in mblock_open ().

!*/
{
   void *retval = NULL;

   CHECKID;
   retval = (void *) (mblock_curr->head);
error_return:
   return (retval);  /* error return */
}

/*!

mblock_sizes	Returns memory block size information.

Summary:

#include <ComUtil/comutil.h>
*/

int	mblock_sizes(int id,unsigned * nel,unsigned * size,unsigned long * nbytes)

/*!
Return Value:  0 if OK, -1 if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             id      ;/*    I  mblock identifier from mblock_open() */
unsigned       *nel     ;/*    O  returns number of elements
				  if NULL, do not return this value. */
unsigned       *size    ;/*    O  returns size of element in bytes
				  if NULL, do not return this value. */
unsigned long  *nbytes  ;/*    O  returns total number of bytes in the block,
				  if NULL, do not return this value. */
#endif
/*!
Description:

Returns size information in the parameters `nel`,`size` and `nbytes'.  If
any of these pointers are NULL, no value is returned.  The value returned
for `nbytes` is just the product of `nel` and `size`, but is a long instead
of just an unsigned.

DON'T FORGET THOSE & symbols in your calling routine!!!

See also: 

Example:   See end of example in mblock_open ().

!*/
{
   int retval=-1;

   CHECKID;

   if (nel != NULL)
     *nel = mblock_curr->nel;
   if (size != NULL)
     *size = mblock_curr->size;
   if (nbytes != NULL)
     *nbytes = ((unsigned long) mblock_curr->nel) * mblock_curr->size;

   retval = 0;
error_return:
   return (retval);  /* error return */
}

#endif /* ifndef DMM_OFF */
