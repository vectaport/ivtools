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
XDLL.C		Extended Doubly-Linked List Support.

Externals:	

Summary:        These functions support the structuring and accessing
		of a doubly-linked list.  They are a modified version
		of Scott's dll functions made more general.  His should
		have been directly modified, but these routines were
		already laying around from another effort.

History:        Written by Robert C. Fitch, 4 Mar 1989

Improvements:

- In xdll_open(): if `beg` is NULL, allocate the area for the user.
	(this is being held off to see if the dmm_*() dynamic allocation
	routines are going to be used for this purpose).
- All: support for `huge` memory areas.  These routines really
  only support normal size memory areas.  Actually only int size since
  offsets are integers in xdllink structure (maybe should be longs).
*/

#include        <stdio.h>

#include        "comutil.ci"
/* comutil.ci includes stdlib.h and malloc.h */

/*=============*/

/* This contains all information for an open linked list */
/* assumes the cells for the linked list are linear in memory */
/* i.e., from address `beg` to `end` */

typedef struct _xdllist xdllist;
struct _xdllist {
		  int size;       /* size of node structure size */
				       /* 0 means cell is empty */
		  xdllink *beg;    /* head of user's alloc structure */
		  int nlinks;      /* max number of links in list */
		  xdllink *head;   /* head node of the list; can be anywhere */
		  xdllink *curr;   /* current level entry in link list */
		};

#ifdef C_terp	
#define XDLL_LISTS	16
#else
#define XDLL_LISTS	32
#endif
static xdllist xdllist_array[XDLL_LISTS];	/* now a static array */
						/* calloc/realloc removed */
/*=============*/

/* Static Functions: */
static  int next_free_link();
static  int clear_links();

/* some pointers for the different open lists */

static xdllist *xdllist_beg,*xdllist_curr;
static int nxdlls=0;                /* number of lists in list_beg array */
static int xdllist_in_use_id = -1;     /* current list in use; -1 is none in use */

/* this checks to make sure there is a list in use */
#define CHECKUSE  { if (xdllist_in_use_id < 0) goto error_return; }

/* These macros efficiently increment structure pointers by byte offsets */
	/* this one returns a pointer to an xdllink structure entry in beg */
#define ADDBEG(nbytes) ( (xdllink *) (((char *)xdllist_curr->beg) + nbytes))

	/* this one returns a byte offset from beginning of structure beg */
#define SUBBEG(pntr) ((int) (((char *) pntr) - ((char *) xdllist_curr->beg)) )

#define HEADID	-2
/* this is identifier of a head node */
/*=============*/

/*!

xdll_open    Open a New doubly-linked-list structure.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_open (void * beg,int nlinks,int nsize)

/*!
Return Value:  >=0 identifier for this link structure, -1 if params
		out of range or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void *		beg      ;/* I   Pointer to link area alloc'd by user */
int             nlinks    ;/* I   Max number of links in the list */
int             nsize     ;/* I   Size of a link entry in bytes */
#endif

#ifdef DOC

/*!
Description:

Open's a new link list in an area of memory allocated by the user starting at
address `beg`.  The number of records (links) in the memory area is
`nlinks` and the number of bytes per record (link) is `nsize`.  

This routine also sets the current linked-list in use to the identifier
returned by this routine.


/* Link Structure */

The links are done using the link structure array.  It contains two elements, 
the first being a node pointer in to the node array.  The second element
is a link to the next entry in the link list.  

See `xdllink` structure. If the *node entry in listink is NULL, 
then this is an empty entry.

/* List Pointer Reference Structure */

Another data structure declares a set of pointers for each link list.

See `list` structure for details.

===============


Two link lists:

	A <-> B <-> C 
	  -and-
	D <-> E



links                  LINK Structure
index           prev        next
-------         -----       -----
  L0            NULL         L1         "A"
  L1            L0           L2         "B"
  L2            L1           NULL       "C"     end of this list

  L3            NULL         L4         "D"
  L4            L3           NULL       "E"     end of this list

===============


See Also:  xdll_close()


Example:


/* Code to set up the earlier example */
main() 
{
	int i,id1,id2;
	static char *strings[5] = {"A","B","C","D","E"};
	void *pos,*cpos;

	typedef struct _mystruct mystruct;      /* define application struct */
	struct _mystruct {
			  xdllink node;        /* must be first in struct */
			  char *string;    /* to point to a string name */
			} array1[3],array2[2],*mine;

/* Note: no error checking shown.  */
/* I'm assuming everything is legal.  You shouldn't. */

	id1 = xdll_open (array1,3,sizeof(mystruct));
	id2 = xdll_open (array2,2,sizeof(mystruct));

	xdll_use(id1);
	xdll_head();
	for (i=0; i<=2; i++)    /* add A,B,C */
	{
	   mine = (mystruct *) xdll_add (); 
			/* add links to "A" parent for "B","C","D" */
	   mine->string = strings[i];  /* add to string info to structure */
		/* note: you put your stuff in after you add the link */
	   if (i==1)	/* if "C" */
	     cpos = xdll_curr ();	/* save position of "B" entry */
	}
	xdll_use(id2);
	xdll_head();
	for (i=3; i<=4; i++)    /* add D,E */
	{
	   mine = (mystruct *) xdll_add (); 
			/* add links to "A" parent for "B","C","D" */
	   mine->string = strings[i];  /* add to string info to structure */
		/* note: you put your stuff in after you add the link */
	}
	xdll_use(id1);

/* print number of nodes and links left; should both be zero */
	printf ("links left = %d\n",xdll_links_left());

/* Delete "B" and its children "E" and "F" from the list */
	xdll_goto (cpos);  /* move to "C" entry saved earlier */
	xdll_delete (1);   /* delete it and its children from the list */
			/* reposition on "B" */

	xdll_close ();                /* close and release memory */
}

#endif /* DOC */
/*
!*/
{
  int id;

/* check input parameter ranges */
  if (beg == NULL ||
      nlinks < 1 ||                      /* must have at least one node */
      nsize < sizeof(xdllink) )       /* at least size of a link */
    goto error_return;  /* parameter error, get out of here */

  /* Now make an entry in the list struct array; calloc it as needed */
  if (nxdlls <= 0)	/* see if this is the first call */
  {	
      for(id=0,xdllist_curr=xdllist_array; 
          id < XDLL_LISTS; id++, xdllist_curr++)  /* make all lists empty */
      {  xdllist_curr->size = 0; }	/* 0 means empty */
      xdllist_beg = xdllist_array; 	/* set beginning of list pointer */
      nxdlls = XDLL_LISTS;		/* set number of lists */
  }
  /* first search current list for any empty ones and use them first */
  for (id=0, xdllist_curr=xdllist_beg; 
       id < nxdlls && xdllist_curr->size != 0;  id++,xdllist_curr++)
  { /* no body */ }
  if (id >= nxdlls)		/* true if no empty ones found */
    goto error_return;	/* out of memory ; all lists in use */
  xdllist_curr->size = nsize;
  xdllist_curr->beg = (xdllink *) beg;  /* set to user furnished pointer */
  xdllist_curr->nlinks = nlinks;  /* remember this too */

          /* do this one with bytes since struct type is unknown */

  /* set current position pointers to the start of the lists */
  xdllist_curr->head = xdllist_curr->curr = NULL;
           /* set the head current link in the list to none */
		/* this means they are empty */
  clear_links(beg,nlinks,NULL);	/* clear all links in the table */
  return (xdllist_in_use_id = id);		/* OK return */

error_return:		/* return an error code */
    return (FUNCBAD);
}


/*!

xdll_reopen    Re-Open an existing doubly-linked-list structure.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_reopen (int newflag,void * beg,int nlinks,int newhead)

/*!
Return Value:    0 the id reopened if OK,  
		-1 if params out of range or insufficient memory 
                     	or no list is in use
			or head does not point to a head entry node.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int	        newflag  ;/*  I   true - this is a new list (no head) */
			  /*     false - this is an old list (use newhead)*/
void *		beg       ;/* I   Pointer to link area alloc'd by user */
			   /*     NULL means use current pntr value    */
int             nlinks    ;/* I   Max number of links in the list      */
			   /*     0 means don't change number of links */
int     	newhead   ;/* I   Byte Offset to head of linked list    */
			   /*     < 0 means leave head at old offset */
#endif


/*!
Description:

Reopen's an link list in an area of memory allocated by the user starting at
address `beg`.  The number of records (links) in the memory area is
`nlinks` and the number of bytes per record (link) is `nsize`.  The `head`
parameter is used to specify the entry into the linked-list.  If `head`
is -1, the current position of the head is used.  If `head` is >= 0, this
is the new head; if the head is not a legal head an error is returned.
If `newflag` is true, the list is set to empty and the `newhead` parameter
is not used.  The current position in the list is also set to the head.

This call is mainly used to change the position and sizes of existing
link lists.  


!*/

{
  int old_nlinks,delta;
  xdllink *curr;
  int oldhead;	
  int retval = FUNCBAD;

  CHECKUSE;		/* check that there is one open */
  if(xdllist_curr->head != NULL)	
    oldhead = SUBBEG(xdllist_curr->head);	/* offset to current head */
  old_nlinks = xdllist_curr->nlinks;	/* what they are coming in */
  if (beg  != NULL) xdllist_curr->beg = (xdllink *) beg;
  if (nlinks > 0) xdllist_curr->nlinks = nlinks;  /* remember this too */

  curr = (newflag) ? NULL : 	/* no head if newflag true */
         ((newhead >= 0) ? ADDBEG(newhead) :  /* corrects for new beg */
	 ((xdllist_curr->head == NULL) ? NULL : ADDBEG(oldhead)) );
		/* use current head offset or NULL if head param negative */
  /* don't set head and curr if the head node asked for is not a head node */
  if (curr != NULL && curr->prev != HEADID)
     goto error_return;	/* illegal head, error returned */
  xdllist_curr->head = xdllist_curr->curr = curr;  /* set head and curr pos */

  /* set current position pointers to the start of the lists */
  delta = xdllist_curr->nlinks - old_nlinks;  /* pos means it got bigger */
  if (delta > 0)	/* if size increased */
  {	/* yes, you have to clear the new links on the end */
    curr = ADDBEG( (old_nlinks * xdllist_curr->size) ); 
	/* move to where new part starts and clear it to the end */
    clear_links(curr,delta,NULL);	/* clear from curr to end */
  }
    retval = FUNCOK;	/* return will be OK */
error_return:		/* return an error code */
    return (retval);
}

/*!

xdll_use		Set the current linked list in use.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_use (int id)

/*!
Return Value:  >=0 id in use if OK, -1 if id is out-of-range or the
					linked list is undefined.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		id	  ;/* I   identifier returned by xdll_open() */
#endif


/*!
Description:

Sets the linked list to use for all other function calls.  If `id` is negative
or greater than the current number of entries in the link array (`nxdlls`) or
the entry selected by the id is not defined, then the current identifier in
use is set to and returned as a -1.


See also:  xdll_open(), xdll_in_use()


Example:   See end of example in xdll_open ().

!*/
{
     if (id < 0 || id >= nxdlls)	
       goto error_return; 		/* out of legal range */
     xdllist_curr = xdllist_beg + id;   /* go to this ones entry */
     if (xdllist_curr->size == 0)
       goto error_return;               /* selected list is not open */
     xdllist_in_use_id = id;	        /* save identifier */
     return (id);			/* OK return */

error_return:
     xdllist_in_use_id = -1;	/* make it so none are in use */
     return (-1);               /* error return */
}


/*!

xdll_in_use	Return id of the current linked list in use.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_in_use ()

/*!
Return Value:  >=0 id in use if OK, -1 if no link list in use.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:


Returns the identifier (id) for the current linked list in use.  If no
linked list is in use, it returns a -1.  This routine is mainly used for
saving the current identifier in use and its associated state.


See also:  xdll_use()


Example:   

!*/
{
     return (xdllist_in_use_id);	/* pretty simple */
}


/*!

xdll_close	Close the current linked list in use or close all lists.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_close (int allflag)

/*!
Return Value:  0 (FUNCOK) if close OK, -1 (FUNCBAD) if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		allflag  ;/*   I  allflag true, close all lists */
			  /*              false, close list spec'd by id */
#endif

/*!
Description:

This closes an already opened list structure.  The user must have already
free'd any alloc'd pointers in his structures before calling this routine.

Improvements:
- Add a free function parameter to be used to call user structure free.


See also:  xdll_open(), xdll_use()


Example:   See end of example in xdll_open ().

!*/
{
   int i;

   if (!allflag)
     CHECKUSE;	/* make sure one is in use */
   for (i=0, xdllist_curr=xdllist_beg; i<nxdlls; i++,xdllist_curr++)
   {
      if ( allflag || i==xdllist_in_use_id )
		/* see if closing all of them or just xdllist_in_use_id */
         xdllist_curr->size = 0;	/* free entry entry */
   }
   xdllist_in_use_id = -1;	/* set list in use to no list */
   return (FUNCOK);	/* OK return */

error_return:		/* return an error code */
    return (FUNCBAD);
}

/*!

xdll_curr 		Return current list position.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_curr ()   

/*!
Return Value:  current position if OK, NULL if list not opened.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Returns current position in the list.

See Also:  xdll_use()

!*/
{
   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   return ((void *)xdllist_curr->curr);	/* return current level */
				/* it could also be NULL sometimes */
error_return:
    return (NULL);
}




/*!

xdll_head	Go to head entry in the link list.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_head () 

/*!
Return Value:  Head position if OK, NULL if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Goes to the head position in the linked list (the top).  Note that this
does not always have to be the beginning of the list.

See Also:  xdll_open().

!*/
{
   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   xdllist_curr->curr = xdllist_curr->head;
   return ((void *) xdllist_curr->head);

error_return:
    return (NULL);
}

/*!

xdll_next	Go to next link in the list.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_next ()   

/*!
Return Value:  New position if OK, NULL if no next.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Goes to the next entry in the link list.  If none, it returns NULL.

See Also:   xdll_use().

!*/
{
   xdllink *curr;

   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   curr = xdllist_curr->curr; /* pointer to current entry */
   if (curr->next < 0)
      goto error_return;
   return ((void *) (xdllist_curr->curr = ADDBEG(curr->next) ) );

error_return:
    return (NULL);
}

/*!

xdll_prev	Go to previous link in the list.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_prev ()   

/*!
Return Value:  New position if OK, NULL if no previous.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Goes to the previous entry in the link list.  If none, it returns NULL.

See Also:   xdll_use().

!*/
{
   xdllink *curr;

   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   curr = xdllist_curr->curr; /* pointer to current entry */
   if (curr->prev < 0)
      goto error_return;
   return ((void *) (xdllist_curr->curr = ADDBEG(curr->prev)) );

error_return:
    return (NULL);
}

/*!

xdll_tail	Go to the last entry in the link list.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_tail () 

/*!
Return Value:  Last position if OK, NULL if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Goes to the last position in the linked list (the bottom).

See Also:  

!*/
{
   xdllink *curr,*next;

   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   if ( (curr = (xdllink *) xdllist_curr->curr) == NULL)/* start from current */
     xdll_head();	/* otherwise, start from head of list */

   /* goto the end from current position */
   while ( (next = (xdllink *) xdll_next()) != NULL)
   { curr = next; }

   return ((void *)curr);	/* this is the last one */

error_return:
    return (NULL);
}

/*!

xdll_insert	Insert a(nother) link in the list relative to curr pos.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_insert (int before)

/*!
Return Value:  position of new link if OK, NULL if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		before    ;/* I   flag, True - insert before current link */
			   /*           False - insert after current link */
#endif

/*!
Description:

Adds a link relative to the current position in the of the linked list
currently in use. Returns the position of where the link was added.  
Returns a NULL if the list is full or no list is in use.

Adds the link relative to the current position.  The `before` parameter 
determines whether the child is inserted before or after the current 
position.

If the current position is NULL, then the list is empty and the new
link will be the first one added.


See Also:   xdll_use().

!*/
{
   xdllink *curr, *newl;
   int nnew,ncurr;

   CHECKUSE;
   /* first find a node that is empty */
   if ( (nnew = next_free_link(1)) < 0 )  
      goto error_return;	/* uh oh, out of memory; die */
   newl =  ADDBEG(nnew);
   if (xdllist_curr->head == NULL)	/* if list is empty */
   {  /* first entry, make it the head */
     xdllist_curr->head = xdllist_curr->curr =  newl;
			/* set to start of list */
     newl->prev = HEADID; /* HEADID means its the head node */
     newl->next = -1;  /* initialize */
   }
   else	/* there are already links in the table */
   {
      if ( (curr = xdllist_curr->curr) == NULL)
          goto error_return;     /* uh oh, NULL or ran out of space */
      ncurr = SUBBEG(curr);
      if ( before )	/* if inserting before curr */
      {	
	if (curr->prev >= 0) /* not head of list ? */
          ADDBEG(curr->prev)->next = nnew;
	else
	  xdllist_curr->head = newl;  /* new head of the list */
	newl->prev = curr->prev; /* OK even if curr->prev is < 0 */
	newl->next = ncurr; /* next is previous current entry */
        curr->prev = nnew;
      }
      else  /* !before (after)  inserting after curr */
      {
	if (curr->next >= 0) /* not tail of list ? */
          ADDBEG(curr->next)->prev = nnew;
	newl->next = curr->next;  /* OK even if curr->next is <0 */
	newl->prev = ncurr; /* next is previous current entry */
	curr->next = nnew;	
      }
   }
   return ((void *) newl);	/* return position added */

error_return:
    return (NULL);	/* means out of memory or id not opened */
}


/*!

xdll_delete	Delete the current link.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_delete (int flag)

/*!
Return Value:  FUNCOK if OK, FUNCBAD if error.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		flag      ;/* I   True - reposition to previous link */
			   /*     False - reposition to next link */
#endif

/*!
Description:

Deletes the current link.  It repositions the link to either the previous
link or next link depending on the state of `flag`.


See Also:   xdll_use().

!*/
{
   xdllink  *prev,*curr,*next;   

   CHECKUSE;
   if (xdllist_curr->head != NULL)	/* if list is empty */
   {  /* no, go ahead and delete */
      curr = xdllist_curr->curr; /* pointer to current entry */
      if (curr == NULL)
	goto error_return;	/* you forgot to position it */
      if (curr->next < 0)
         next = NULL;
      else
         (next = ADDBEG(curr->next))->prev = curr->prev;

      if (curr->prev < 0)		/* if curr is head of the list */
      {
        prev = NULL;
	xdllist_curr->head = next;
      }
      else
          (prev = ADDBEG(curr->prev))->next = curr->next;

      curr->prev = curr->next = -1;	/* delete this link */
      xdllist_curr->curr = ( (flag) ? prev : next);
			/* set to either prev or next */
			/* note: could be NULL position, but thats OK */
   }
   return (FUNCOK);	

error_return:
    return (FUNCBAD);	
}


/*!

xdll_goto	Change to (goto) a specific position in the link list.

Summary:

#include <ComUtil/comutil.h>
*/

void *xdll_goto (void * pos)

/*!
Return Value:  New position if OK, NULL if pos does not exist or is empty.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void	       *pos	  ;/* I   Pointer to position to goto */
#endif

/*!
Description:

The moves the current position to a new absolute position in the list.  It
assumes that 'pos' was retrieved using other position returning functions.
If the new position is not in the list or is an empty cell, then a NULL is
returned.  Otherwise, the new position is returned.

See Also:  

!*/
{
   xdllink *curr;

   CHECKUSE;
   if (xdllist_curr->head == NULL)	/* nothing in the list ? */
     goto error_return;		/* no, error */
   curr = (xdllink *) pos;	/* convert to link struct */
	/* make sure its a legal position */
   /* an empty or deleted position has curr->next and curr->prev set to -1 */
   /* if curr->prev == HEADID, that means its a head entry */
   if ( curr->next == -1 && curr->prev == -1)	/* no prev,no next, not head */
        goto error_return;	/* return error code if going to empty node */
   xdllist_curr->curr = (xdllink *) pos;
   return (pos);

error_return:
    return (NULL);	/* means out of memory or id not opened */
}


/*!

xdll_clear()	     Clear out a linked list.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_clear(int flag,void (*userfunc)())

/*!
Return Value:  0 (FUNCOK) if OK, -1 (FUNCBAD) if no linked list in use.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		flag      ;/*  I  true - clear entire linked list */
		           /*     false - clear only from head to tail */
void	     (*userfunc)();/*  I  User function that is called for every */
			   /*     link cleared with the pointer to the *
			   /*	  user's structure;  NULL if no user function */
#endif

/*!
Description:

Clears all of the cells in a linked list for the currently link list is
use (from xdll_use()).  Also sets head and curr pointers to NULL to indicate
that the list is clear.  If `flag` is set, all the links everywhere in the
entire list are cleared.  If `flag` is false, on the links in the current
connected list from xdll_head() to xdll_tail() are cleared (deleted).  This
does nothing to the user's portion of the structure; only the link info.  To
clear the user info, the user must walk through their own structure or use
the `userfunc` parameter.   The `userfunc` is called every entry that a link
is released for.  The address of the user's structure is called as a parameter
to this function (the parameter is a `void *` type and must be promoted to the
user's structure by the user routine).

See Also:

!*/
{
   xdllink *curr;

   CHECKUSE;
   if (flag)	/* if true, clear entire linked list structure out */
      clear_links(xdllist_curr->beg,xdllist_curr->nlinks,userfunc);
   else	/* just clear from head to tail and null out head */
   {
     for(curr = (xdllink *) xdll_head();
	 curr != NULL;
	 curr = (xdllink *) xdll_head() )
     {
        if (userfunc != NULL)
	   (*userfunc)((void *) curr);	/* call user func to clear */
        xdll_delete(0);		/* delete link.  Repo after */
     }
   }
   xdllist_curr->curr = xdllist_curr->head = NULL;
					/* this says its all empty */
   return(FUNCOK);

error_return:
   return (FUNCBAD);            /* none there, exit */
}



/*!

xdll_links_left         Returns number of free links left in the list.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_links_left ()	

/*!
Return Value:  Number of links if OK, -1 if no list in use.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

This utility routine returns the number of free links left in the list.  If
it is zero, the list is full.

See Also:  

!*/
{
   int num;

   CHECKUSE;
   for (num=0;	/* set to start search at top of the link structure */
        next_free_link(num==0) >= 0;   /* go until none left anymore */
	num++);		/* set to search from current pos, incr num */
   return (num);
error_return:
    return (FUNCBAD);	/* means out of memory or id not opened */
}


/*!

xdll_links_num		Returns the number of links in the current link list.

Summary:

#include <ComUtil/comutil.h>
*/

int xdll_links_num ()	

/*!
Return Value:  Number of links if OK, -1 if no list in use.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Returns the maximum number of links possible in a tree.  Its the same as
`xdllist_curr->nlinks`.  The number of links used can be found as
`xdll_links_num() - xdll_links_left()'.


See Also:  

!*/
{
   CHECKUSE;
   return (xdllist_curr->nlinks);
error_return:
    return (FUNCBAD);	/* means out of memory or id not opened */
}

/*!

next_free_link		Returns position of next free link.

Summary:

#include <ComUtil/comutil.h>
*/

static  int next_free_link(int flag)

/*!
Return Value:  Offset of next free link relative to beginning of link list,
	       -1 if none left.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int		flag      ;/* I   If true, start at top of link structure */
			   /*     If false, continue from last position */
#endif

/*!
Description:

This internal routine returns the address of the next free position in the
link structure.  It searches from the start of the structure if flag is true or
continues from the last position if the flag is false.  The flag is normally
set to true when looking for a place to add a node.  It is set to false when
counting the number of remaining cells.

See Also:  

!*/
{
   static xdllink *curr = NULL;	/* static to hold its value */
   static int i;	/* link number */
   xdllink *tpntr;

   if (flag || curr == NULL)
   {
     curr = xdllist_curr->beg;	/* start of the table */
     i = 0;	/* set to start at top of list */
   }

   while (i++ <= xdllist_curr->nlinks)
   {
     tpntr = curr;	/* current position */
     curr = (xdllink *) (((char *)curr) + xdllist_curr->size); /* preincr */
   /* an empty or deleted position has tpntr->next and tpntr->prev set to -1 */
   /* if tpntr->prev == HEADID, that means its a head entry */
     if ( tpntr->next == -1 && tpntr->prev == -1)
	return (SUBBEG(tpntr));
   }  
/* if you leave while, there are no remaining links */
   curr = NULL;
   return (-1);		/* none there, exit */
}


/*!

clear_links		Clear from position in link list a spec'd # of links

Summary:

#include <ComUtil/comutil.h>
*/

static int clear_links(xdllink * curr,int nlinks,void (*userfunc)())

/*!
Return Value:  0 always.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
xdllink        *curr      ;/* I   Position to start clearing links for */
int		nlinks    ;/* I   Number of links to clear */
void       (*userfunc)()  ;/* I   User function to call to clear their own */
			   /*     structure elements; NULL if none */
#endif

/*!
Description:

This internal routine clears the links in a user structure for `nlinks` 
records starting at `curr` position in the link structure.  It is used
to clear open'd and expanding re-opened link areas.

See Also:  

Examples:

To clear the entire current link list:
	clear_links(xdllist_curr->beg,xdllist_curr->nlinks);

!*/
{
   while (nlinks-- > 0)
   {
     curr->prev = curr->next = -1;	/* clear the links */
     if (userfunc != NULL)
	(*userfunc)((void *) curr);	/* call user func to clear */
     curr = (xdllink *) (((char *)curr) + xdllist_curr->size); /* preincr */
   }  /* if you leave while, there are no remaining links */
   return(FUNCOK);
}

