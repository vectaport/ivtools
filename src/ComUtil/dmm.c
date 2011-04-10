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
DMM.C           Dynamic Memory Management Routines

Externals:
                See NUTIL.ARG

Summary:        These function support dynamic memory management.  They
                allow memory to be allocated and reallocated in a
                deterministic manner by supporting automatic memory
                packing and allocated memory pointer updating.  This
                differs from malloc() and realloc() routines which are
                not guaranteed to be able to take maximum efficiency of
                all memory and may fail to be able to allocate
                memory since they are unable to move existing allocated
                areas to new areas of memory (i.e., can't pack).

                More details are in the dmm_mblock_alloc() function.

History:        Written by Robert C. Fitch, March 1989
		3 Apr '89, R.Fitch  Made dmm_update more general.
			Add function dmm_movrecs().
		21 Apr '89 R.Fitch  >> operator does not work in OS9.
			Replaced >>1 usage with /2 division.  Works OK.
*/

/**     DMM_TEST should be set to decrease values for easier testing **/
#if 0
#define DMM_TEST
#endif

#include        <stdio.h>

#include        "comutil.ci"

/* comutil.ci includes of ComUtil/comutil.h, util.h, dos.h, string.h, stdlib.h, malloc.h */

#ifndef DMM_OFF
/* ========================================== */

typedef char huge * HPNTR;      /* huge pointer to character */

typedef struct _dmmpntr huge * PNTRPNTR;
typedef struct _dmmpntr dmmpntr;  /* allocation Table  structure */
struct _dmmpntr {
                   xdllink link;        /* link list support */
                   short int allocflag; /* 1 if points into alloc'd array */
                                        /* 0 if not in alloc'd area */
                   void huge **user;    /* user's duplicate pointer address */
                   unsigned long nrecs; /* number of records if pntr array  */
                                        /* >1 an array, ==1 a scalar pointer */
                   unsigned nsize;      /* bytes to next pointer if nrecs>1 */
                                        /* set to 0 if scalar pointer */
                   int (*cond)();       /* checks condition in user */
                 };
/* cond() is called with a pointer to each alloc'd structures elements */
/* it is a user function that returns true to update or false to skip */
/** A real waste for scalar pointers; add another struct for scalars someday */

/* ========================================== */

/* the doubly-linked allocation table */
/* linked to avoid moving memory to add and delete blocks; just the */
/* links have to be changed.  Saves time */

typedef struct _dmmalloc huge * ALLOCPNTR;
typedef struct _dmmalloc dmmalloc;  /* allocation Table  structure */
struct _dmmalloc {
                xdllink link;    /* for link list support (XDLL.C) */
                /* these are for the memory allocated */
                void **user;  /* pointer to users declared pointer */
                HPNTR mentry;/* address of alloc'd area in mblock */
                unsigned nsize; /* size of each alloc'd area in bytes */
                unsigned long nrecs;    /* number of records allocated */
                unsigned long tsize; /* total bytes alloc`d for the area */
                };

/* ========================================== */

static int fatal_error = 0;     /* fatal error indicator */
                /* not fully implmented yet */

static HPNTR mblock = NULL;   /* must be huge type in Microsoft C */
static HPNTR mblock_beg;       /* first usable address on page boundary */
static HPNTR mblock_end;       /* last usable location in mblock */
static unsigned long mblock_size;  /* number of bytes in mblock */

static int alloc_id;    /* id for alloc link table */
static ALLOCPNTR alloc_beg; /* address of entry into alloc table */

static int pntr_id;     /* for pntr link table usage */
static PNTRPNTR pntr_beg;  /* start of memory for pntr lists */

/* statics used between dmm_gap_*() static functions */

static HPNTR gapbeg;            /* address of beginning of gap */
static unsigned long gap_size;  /* bytes in a gap */
static ALLOCPNTR gap_alloc;     /* allocation table entry prior to a gap */

/* Used to indicate that a realloc has occured; used in dmm_mblock_pack() */
static int realloc_flag;

/* static used to communicate new alloc position between realloc and malloc */
static ALLOCPNTR new_curr;

/* Used by dmm_ichk() to sum up bytes; used by dmm_mblock_stats() */
static unsigned long used_bytes;        /* total bytes used in mblock */
static unsigned long free_bytes;        /* total free bytes in mblock */
static unsigned long system_bytes;      /* bytes used by system in mblock */

/* Static Functions: */

static long dmm_sub_pntrs ();
static HPNTR dmm_add_offset ();
static int dmm_update ();
static ALLOCPNTR dmm_find_alloc();
static PNTRPNTR dmm_find_pntr();
static ALLOCPNTR dmm_find_pntr_to_alloc();
static int  dmm_memcpy();
static int  dmm_memset();
static unsigned long dmm_gap_size();
static HPNTR dmm_gap_search();
static void dmm_update_pntr();
static int dmm_not_portable();
static void dmm_pack_range();

/* DMM MACROS */

/* DMM_PAGE defines the boundaries that alloc'd areas must start on */
/* for MSDOS, it must be on 16 byte boundaries */
/* for OS9 and others, integer boundaries is OK */

/* define alloc page boundaries */
#if defined(MSDOS)
#define  DMM_PAGE  16   /* every 16 bytes in Microsoft C and Intel x86's */
#else
#define  DMM_PAGE sizeof(int)  /* on integer boundaries in most machines */
#endif

/* ROUNDUP does a ceiling function on lsize to the next DMM_PAGE */
/* This way everything will always end up starting on a page boundary and */
/* will always be an integral number of pages in size */
#define ROUNDUP(num)     ( ((num)+(DMM_PAGE-1)) & ( ~(DMM_PAGE-1) )  )


/* these save and restore the currently used linked-list in xdll_*() */
typedef struct _savestruct savestruct;
struct _savestruct {    /* this structure used to save current dll state */
                int id; /* identifier used by linked list routines */
                void *curr;  /* current position */
                  };
/* These two just save the id */
#define SAVE_CURR_LINK_ID(save) { \
        save.id = xdll_in_use(); }
#define RESTORE_CURR_LINK_ID(save) { \
        xdll_use(save.id); }
/* These two save id and current position */
#define SAVE_CURR_LINK_POS(save) { \
        save.id = xdll_in_use();\
        save.curr = xdll_curr();  }
#define RESTORE_CURR_LINK_POS(save) { \
        xdll_use(save.id); \
        xdll_goto(save.curr);  }

/* This checks to see if the fatal_error flag is true */
#define FATALCHECK  {if(fatal_error) goto error_return;}

/* this is the shift increment for tables to alloc */
#ifdef DMM_TEST
#define DMM_SHIFT       ((unsigned) 1)
#else
#define DMM_SHIFT       ((unsigned) 6)
#endif

#define DMM_INCR        ((unsigned long) (1L << DMM_SHIFT))


/* This is a portable max unsigned int */
#define DMM_MAXUNSIGNED ( (unsigned int) (~(unsigned) 0) )

/* I'm using a half unsigned number to keep away from limits of unsigned for */
/* memory moves which can only xfer 1 less than 64 K bytes */
/* This is half a maximum unsigned integer plus 1 */
/* its 0x8000 for 16-bit machines , 0x80000000 for 32-bit machines */
/* Could use a bigger number, but this moves on nice even boundaries */
/* I originally used '>> 1' in this, but it did not work in OS9, so */
/* now I use a '/2' instead and that works OK */
#ifdef DMM_TEST
#define DMM_MAXCOUNT    16
#else
#define DMM_MAXCOUNT    ( (DMM_MAXUNSIGNED/2) + 1 )
#endif

/* This determines how many pages of maximum unsigned ints are */
/* in a unsigned long.  Also the remainder.  Machine portable */
/* Uses DMM_MAXCOUNT to determine maximum size of a page */
#define DMM_PAGES(ulong,pages,remains) {  \
  pages =   (unsigned) ((unsigned long)(ulong)/ (unsigned long) DMM_MAXCOUNT);\
  remains = (unsigned) ((unsigned long)(ulong)% (unsigned long) DMM_MAXCOUNT);\
                                }

/* Used to add a byte offset to a pointer and subtract two pointers to */
/* generate a byte offset.  This works for both int and long size  */
/* byte offsets (even microsoft allows both with huge pointers! */
/* C_terp doesn't support huge, so here's another kludge */
#if defined(MSDOS)
#define ADDHOFFSET(hpntr,offset) dmm_add_offset((HPNTR)(hpntr),(long)(offset))
#else
#define ADDHOFFSET(hpntr,offset)  ((HPNTR) (hpntr) + (long) (offset))
#endif
/* This macro returns a LONG ! */
/* must use the first one with Microsoft, but second one is OK with OSK */
/* Second may be used if Microsoft changes return to long instead of int */
#if defined(MSDOS)
#define SUBHPNTRS(pntr1,pntr2)  dmm_sub_pntrs((HPNTR)(pntr1),(HPNTR)(pntr2))
#else
#define SUBHPNTRS(pntr1,pntr2) ( (long) (((HPNTR)(pntr1)) - ((HPNTR)(pntr2))) )
#endif

/* Macro to align an address relative to a unsigned offset base address */
/* This uses normal pointer adding of positive offsets; */
/* This is currently needed for xdll_*() routines which do not support */
/* huge arithmetic on pointers */
#define BASEALIGN(base,offset) ((HPNTR)((char *)(base)+(unsigned int)(offset)))


/* Microsoft 5.0 requires structure sizes to be a power of 2 if the */
/* size of an allocated area exceeds 64K.  This is a pain in the butt */
/* So, this kludge is needed to limit the user from allocating more than */
/* 64K since I don't feel like figuring out if the sizeof rec is a power of 2 */
/* Note that this essentially does nothing for 32-bit machines and OS-9 */
#if defined(MSDOS)
#define CHECKSIZE(size) {\
        if ((unsigned long) (size) > 65535L ) goto error_return;  }
#else   /* nothing for other systems like OS-9 and 32-bit ones */
#define CHECKSIZE(size)
#endif

#endif /* #ifndef DMM_OFF */

/* ========================================== */

/*!

dmm_mblock_alloc        Allocate the dynamic memory block.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_mblock_alloc (unsigned long nbytes)

/*!
Return Value:  0 if OK, -1 if insufficient memory or mblock already exists.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned long   nbytes    ;/* I   Total number of bytes of memory to alloc */
#endif


#ifdef DOC
/* look for this in dmm1.doc */
#endif /* DOC */

{
#ifdef DMM_OFF
   return 0;
#else

   ABSPNTR abs;
   savestruct save;

    SAVE_CURR_LINK_ID(save);      /* save current link list in use */
/* should be at least enough for DMM_INCR alloc's, pntrs and dups */
    if (mblock != NULL ||       /* if already allocated */
        nbytes < DMM_INCR*(sizeof(dmmalloc)+sizeof(dmmpntr))  ||
        dmm_not_portable()   )
       goto error_return;

    fatal_error = 0;            /* reset fatal error indicator */
    alloc_id = pntr_id = -1;    /* set these for proper error_return */
    mblock_size = ROUNDUP(nbytes);  /* round to next multiple of page size */
    /* allocate memory with halloc(); halloc is calloc in 32-bit systems */
    /* grab an extra page for page offsetting the start of mblock */
    if ( (mblock = (HPNTR)halloc(mblock_size+DMM_PAGE,1))  == NULL)
        goto error_return;      /* insufficient memory */
#ifndef C_terp	/* Inform C-terp this memory can be written to */
    bl_alloc((char *)mblock,mblock_size+DMM_PAGE);
#endif
    /* memory is cleared by halloc() */
    /* make sure mblock starts on a page boundary */
    PNTR_TO_ABS(mblock,abs);    /* convert to an absolute */
    mblock_beg = ADDHOFFSET(mblock,(ROUNDUP(abs) - abs)); /* round address up */
    mblock_end = ADDHOFFSET(mblock_beg,(mblock_size - 1));

    /* allocate allocation table at the start of mblock */
    /* bootstrap first entry to point to the alloc table itself */
    alloc_beg = (ALLOCPNTR)mblock_beg;  /* for now, the start of mblock */

    /* create linked list for DMM_INCR elements. */
    /* also create reopen type entries for dup's and pntr's */
    if ( (alloc_id =
          xdll_open((void *)alloc_beg,(unsigned)DMM_INCR,sizeof(dmmalloc))) < 0
       )
        goto error_return;      /* xdll blew it */

    xdll_use (alloc_id);        /* use this one */
    xdll_insert(0);      /* insert the head of the list; guaranteed to fit */
    /* now add data for head  */
    xdll_head();        /* shouldn't need this */
    alloc_beg->user = (void **) (&alloc_beg); /* user pntr to update */
    alloc_beg->mentry = (HPNTR) alloc_beg;    /* start of table */
    alloc_beg->nrecs = DMM_INCR;  /* grab this many records for now */
    alloc_beg->nsize  =  sizeof(dmmalloc);    /* size of an entry */
    alloc_beg->tsize =  ROUNDUP(alloc_beg->nrecs * alloc_beg->nsize);
                                /* put on page boundary */
/* if size is always on page boundaries, then memory will always start on */
/* page boundaries; other routines rely on this happening */

/* Alloc table is bootstrapped.  OK to use dmm_malloc(), etc. now */

/* Create the link lists for pointers */

    if ( dmm_calloc((void**)&pntr_beg,DMM_INCR,sizeof(dmmpntr)) != 0)
           goto error_return;   /* insufficient memory */

    ADDRALIGN(pntr_beg);        /* align address; address still pnts to same */
    if((pntr_id = xdll_open(pntr_beg,(unsigned)DMM_INCR,sizeof(dmmpntr))) < 0)
           goto error_return;

    RESTORE_CURR_LINK_ID(save); /* restore current link list id */
    return(FUNCOK);

error_return:           /* return an error code */
    dmm_mblock_free();  /* free what's been done so far */
    RESTORE_CURR_LINK_ID(save); /* restore current link list id */
    return (FUNCBAD);
#endif
}

/*!

dmm_mblock_free         Free the entire memory block.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_mblock_free ()

/*!
Return Value:  0 if OK, -1 if no mem block has been allocated or already free.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Free the memory block created by dmm_mblock_alloc().


See also:  dmm_mblock_alloc()


Example:

!*/
{
#ifdef DMM_OFF
     return 0;
#else
     savestruct save;

     if (mblock == NULL)  /* error return if block has not been alloc */
        goto error_return;

     /* close link lists used by this routine */
     SAVE_CURR_LINK_ID(save);      /* save current link list in use */
     xdll_use(alloc_id);
     xdll_close(0);        /* release linked list definitions */
     xdll_use(pntr_id);
     xdll_close(0);        /* release linked list definitions */
     RESTORE_CURR_LINK_ID(save);        /* restore current link list id */

     /* free block */
     hfree (mblock);    /* free the memory and everything allocated in it */
#ifndef C_terp	/* Inform C-terp this memory has been freed */
    bl_free((char *)mblock);
#endif
     mblock = NULL;     /* set so you know its a goner */

     return (FUNCOK);

error_return:
     return (FUNCBAD);  /* error return */
#endif
}


/*!

dmm_mblock_pack         Pack the entire memory block so there are not gaps.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_mblock_pack (int ipackflag)

/*!
Return Value:  0 if Pack done OK, <0 if no memory block has been allocated,
                                  >0 if incremental pack is completed
                                        (only if ipackflag is true).

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             ipackflag  ;/* I  true - do an incremental pack */
                            /*    false - pack entire memory */
#endif
/*!
Description:


Pack the memory block so that there are no gaps between allocated
memory blocks.  This will maximize memory usage efficiency.  The pack
can be either incremental or entire depending on the condition of the
boolean flag `ipackflag`.  An incremental pack will eliminate one gap
at a time everytime a call is made while the entire pack will eliminate
all gaps.  Any reallocate will restart an incremental pack and a return
value of >0 means the incremental pack is done.

Notes:  `realloc_flag` is a trigger that indicates a pack is needed or
an incremental pack should be restarted.  `packed` indicates that the
memory is entirely packed and does not have to be packed again until another
true `realloc_flag`.  `realloc_flag` is reset after an entire pack and
after an incremental pack restarts.  `realloc_flag` is set anytime there
is a memory reallocation, allocation or free.

See also:


Example:


!*/
{
#ifdef DMM_OFF
     return 0;
#else
     static int apos;           /* current allocation table entry number */
     static int packed=1;       /* flag indicating memory is packed */
     ALLOCPNTR beg,end,next;
     int i;
     savestruct save;
     int retval = -1;   /* error code */

     SAVE_CURR_LINK_ID(save);   /* save current link list in use */
     FATALCHECK;

     if (mblock == NULL)  /* error return if block has not been alloc */
       goto error_return;

     xdll_use(alloc_id);
     if (!ipackflag && realloc_flag)    /* if not an incremental pack */
     {  /* no pack needed unless reallocation has occured */
       dmm_pack_range (NULL,NULL,0);    /* pack entire memory down */
       packed = 1;  /* set pack flag true */
       retval = realloc_flag = 0;       /* reset reallocation flag */
     }
     else /* incremental pack */
     {
       if (realloc_flag)        /* set true everytime an allocation occurs */
       {        /* you have to restart if any reallocation has occured */
          beg = NULL;           /* restart the incremental pack */
          apos = packed = 0;   /* indicate not packed */
          next = (ALLOCPNTR) xdll_head();       /* next is head */
       }
       else if (!packed)        /* continue from current position */
       {
        /* search for next entry */
          for (i=0; i<apos; i++)        /* apos == 0 means head */
          {
            beg = (i==0) ? (ALLOCPNTR) xdll_head() :
                           (ALLOCPNTR) xdll_next();
          }
          next = (ALLOCPNTR) xdll_next();
       }
       if (!packed && next != NULL) /* NULL next means incr pack is complete */
       {
          end = (ALLOCPNTR) xdll_next();        /* end is one after next */
          dmm_pack_range(beg,end,0);    /* pack one cell down */
          retval = realloc_flag = 0;    /* set to not restart next time */
          apos++;               /* set next begin to this next */
       }
       else
          packed = retval = 1;  /* incremental pack is done */
     }
error_return:
     RESTORE_CURR_LINK_ID(save);
     return (retval);  /* error return */
#endif
}

/*!

dmm_mblock_stats        Return sizes of memory used.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_mblock_stats (void ** mpntr, unsigned long * total,
		      unsigned long * used, unsigned long * free,
		      unsigned long * system)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated or
                                allocation tables are corrupt.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void       **   mpntr      ;/* I  Beginning address of memory block */
unsigned long   *total     ;/* I  Total number of bytes in memory block */
unsigned long   *used      ;/* I  Number of bytes the user has allocated */
unsigned long   *free      ;/* I  Num of bytes free */
unsigned long   *system    ;/* I  Num of bytes used by the alloc system */
#endif


/*!
Description:

Returns the total number of bytes available in memory `total`, the
number of bytes already used by the user (`used`), the number of
bytes used by the dmm_*() system (`system`) and the number of bytes
that are free (`free`).  The relation between the two are:

        *total = *used + *system + *free

If any of the pointer arguments are NULL, no value will be returned for
those which are.  So to just get the total number of bytes in memory,
do a `dmm_mblock_stats(NULL,&total,NULL,NULL,NULL,NULL)`.

Some key measures can be derived from these numbers:

        System Overhead :       *system / *used
        Memory Utilization:     *used / *total

These are returned in the arguments, so DON'T FORGET THE `&` in front
of the variables.  The beginning address of the memory block is
returned by this routine also in the `mpntr` parameter.


See also:


Example:

!*/
{
#ifdef DMM_OFF
  return 0;
#else
  savestruct save;
  int retval = FUNCBAD;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  if (mblock == NULL)           /* not block, error */
     goto error_return;

  if( dmm_ichk() != FUNCOK)     /* exit if tables corrupt */
    goto error_return;
  if (used != NULL)
    *used = used_bytes;                 /* used_bytes set by dmm_ichk() */
  if (free != NULL)
    *free = free_bytes;                   /* free_bytes set by dmm_ichk() */
  if (system != NULL)
    *system = system_bytes;             /* system_bytes set by dmm_ichk() */
  if (total != NULL)
    *total = used_bytes + free_bytes;     /* total is used plus free */
  if (mpntr != NULL)
    *mpntr = (void *) mblock_beg;               /* return pointer also */
  retval = FUNCOK;

error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);  /* error return */
#endif
}


/*!

dmm_malloc      Allocate  an area of memory.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_malloc (void ** pntr,unsigned long nrecs,unsigned nsize)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr    ;/*  I  Address of location to put start of area */
                           /*     allocated.   */
unsigned long   nrecs     ;/*  I  Number of records to be allocated */
unsigned        nsize     ;/*  I  Size of each record in bytes   */
#endif

/*!
Description:


Allocates an area in the memory space generated by dmm_mblock_alloc().  Unlike
`calloc()` which returns a pointer to the allocated area, this routine returns
the pointer in the location pointed to by the `pntr` argument.  This `pntr`
should not be an autovariable since the stack is volatile; it should be
in a static, global or allocated area.  The definitions of `nrecs` and
`nsize` are the same as for `calloc()`.

In addition to allocating the memory requested, the address of `pntr` are
remembered so that it can be automatically updated if this memory block
must move.  For pointers inside the memory block record allocated, you must
declare them as pointers-to-be-updated using the `dmm_pntr()` function.
Only pointers which reference memory locations within areas created by
`dmm_calloc()` must be declared this way, but if a pointer is declared that
points outside these areas, it will cause no problem.

The basic algorithm used to allocated is as follows:

- Look for a gap large enough to hold the memory requested.  If found,
  insert a zero size memory block on the end.
- If no gap is large enough, search for any gap and use that one and again
  insert a zero size memory block there.
- Call dmm_realloc() to resize this zero size memory block to have nrecs
  records of nsize size.


See also:  dmm_mblock_alloc(), dmm_pntr(), dmm_free(), dmm_realloc().


Example:   See example in dmm_mblock_alloc()

!*/
{
#ifdef DMM_OFF
  *pntr = malloc(nrecs*nsize);
  return *pntr ? 0 : -1;
#else
  savestruct save;
  ALLOCPNTR curr,curr1;
  HPNTR gap;
  ABSPNTR agap;
  int retval = FUNCBAD;
  unsigned long size;
  int before;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  FATALCHECK;

  /* now see if pntr already in the allocation table or params out of range */
  if ( mblock == NULL || pntr == NULL || nrecs < 1 ||
       nsize < 1 || dmm_find_alloc(pntr) != NULL)
     goto error_return;
  size = nrecs * nsize; /* size needed */

  CHECKSIZE(size);      /* size check for MSDOS */

  /* note: there is always guaranteed to be 1 entry in the alloc table */
  xdll_use(alloc_id);   /* use alloc table */
  if (xdll_links_left() == 0)   /* if no more links, then */
      goto error_return;        /* you are out of memory and you are screwed */
  /* Could realloc here but pntr could be pointing into the memory block and */
  /* I'm too lazy to figure out where it is so I'm allocating afterwards */

  /* first search for a gap large enough for the entire block */
  if ( (gap = dmm_gap_search(size)) == NULL)    /* find one ? */
  {     /* no, just look for anywhere to put it */
        /* look for any gap you can find */
    if ( (gap = dmm_gap_search((long)DMM_PAGE)) == NULL)  /* if no gaps anywhere */
       goto error_return;       /* sorry, must be out of memory */
  }
  before = (gap_alloc == NULL);
  if (before)           /* is it a gap at head of list */
    xdll_head();                /* move to first entry */
  else
    xdll_goto((void *)gap_alloc);   /* move to entry before gap */
  curr = (ALLOCPNTR) xdll_insert(before);       /* insert new entry */
  curr->nrecs = curr->tsize = 0L;      /* make it a zero size memory */
  curr->nsize = nsize;                 /* for reallocating to new size */
  curr->user = pntr;                   /* remember where user's pointer */
  ADDRALIGN(gap);               /* align gap address */
  *(curr->user)=(void *)(curr->mentry=gap); /* set to gap for now; user too */

  if (dmm_realloc(pntr,nrecs) != 0)     /* could it realloc ? */
  {     /* if it fails, nothing will have been updated at all */
    xdll_goto( (void *)curr);   /* return to current position in alloc table */
    *(curr->user) = NULL;       /* set users pointer back to NULL */
    xdll_delete(0);             /* and delete this partially completed entry */
    goto error_return;
  }
  curr = new_curr;              /* this is where curr before is now */
  /* after this point, the contents of `pntr` and curr are no longer valid */
  /* now expand the table if adding one more will overflow */
  if (xdll_links_left() == 0)   /* if no more links, then */
  {  /* realloc some more before you really need them */
    curr1 = dmm_find_alloc(&alloc_beg); /* set curr to pntr_beg */
    if (dmm_realloc((void**)&alloc_beg,(nrecs=curr1->nrecs+DMM_INCR)) != 0)
    { /* big one won't fit, add a single sized one */
        if (dmm_realloc((void**)&alloc_beg,(nrecs=curr1->nrecs+1L)) != 0  )
        {  /* out of memory, quitting time */  /* nothing moved or changed */
        /* first try for DMM_INCR and then just 1 and then give up */
        xdll_goto((void *)curr);/* return to current position in alloc table */
        *(curr->user) = NULL;   /* set users pointer back to NULL */
        xdll_delete(0);         /* and delete this partially completed entry */
        goto error_return;      /* you are out of memory and you are screwed */
        }
    }
    /* reopen alloc_id and change size */
    xdll_reopen (0,(void *) alloc_beg,(unsigned) nrecs,-1);
  }
  retval = FUNCOK;
error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);  /* error return */
#endif
}


/*!

dmm_calloc      Allocate  and clear an area of memory.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_calloc (void ** pntr,unsigned long nrecs,unsigned nsize)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr    ;/*  I  Address of location to put start of area */
                           /*     allocated.   */
unsigned long   nrecs     ;/*  I  Number of records to allocated */
unsigned        nsize     ;/*  I  Size of each record in bytes   */
#endif

/*!
Description:

Same as dmm_malloc() except it also clears memory.  It calls dmm_malloc()
followed by a dmm_clear() to clear the allocated memory.

See also:  dmm_malloc(), dmm_clear().

Example:

!*/
{
#ifdef DMM_OFF
  *pntr = calloc(nrecs,nsize);
  return *pntr ? 0 : -1;
#else
     if (dmm_malloc(pntr,nrecs,nsize) == FUNCOK &&
         dmm_clear(pntr) == FUNCOK )
        return(FUNCOK);
     return (FUNCBAD);  /* error return */
#endif
}

#ifdef DMM_OFF
static int _dmm_realloc_size;
int dmm_realloc_size(int size) { 
    int oldsize = _dmm_realloc_size; 
    _dmm_realloc_size=size; 
    return oldsize;
}
#endif

/*!

dmm_realloc     Re-Allocate an area of memory.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_realloc (void ** pntr,unsigned long nrecs)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr    ;/*  I  Address of location to put start of area */
                           /*     allocated.   */
unsigned long   nrecs     ;/*  I  Number of records to be allocated */
#endif

/*!
Description:


Re-Allocates an area in the memory space generated by dmm_mblock_alloc().
Same as `dmm_calloc()` except `pntr` must be for an existing allocated
array.  Also, only the number of records can be changed at present.  The
new number of records to have is in `nrecs`.  On exit, the static global
`new_curr` will have the position of the new current allocation table
entry.


The basic algorithm is:

- Find area in allocation table that references `&pntr`.  If not, error.
- If shrinking in size, just reduce the size and exit.
- First see if there is enough room immediately after
  to expand.  If so, do it and leave.
- Otherwise, memory must be moved.
- First see if there is a gap large enough to move the new memory size.
  if so, move it and exit.
- If not, search for strings of gaps that go around the region you are
  trying to reallocate.  Choose the one that has the minimum cost in
  memory movement and push memory regions around to create a gap
  that the desired memory can move into.

See also:  dmm_calloc(), dmm_free().


Example:   See example in dmm_mblock_alloc()

!*/
{
#ifdef DMM_OFF
  *pntr = realloc(*pntr, nrecs*_dmm_realloc_size);
  return *pntr ? 0 : -1;
#else
  unsigned long old_size,new_size;
  PNTRPNTR ppntr;
  HPNTR gap,beg,end;
  ALLOCPNTR min_beg,min_end,curr,old_curr,begcurr,next;
  long nbytes, delta;
  unsigned long move_cost,min_move_cost,tsize,gap_nbytes;
  unsigned long offset,goffset,offset1,oldnrecs;
  int passed_old_curr,found,before,is_old_curr;
  int retval = FUNCBAD;
  savestruct save;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  FATALCHECK;

  /* now find pntr in the allocation table */
  if ( mblock == NULL || pntr == NULL || nrecs < 1 ||
              (old_curr = dmm_find_alloc(pntr)) == NULL)
     goto error_return;

  offset = SUBHPNTRS(old_curr,alloc_beg);
  old_size = (oldnrecs = old_curr->nrecs) * old_curr->nsize; 
             /* zero if malloc'ing */
  new_size = nrecs * old_curr->nsize;
  CHECKSIZE(new_size);  /* size check for MSDOS */
  delta = new_size - old_size;
  /* if shrinking or following gap large enough, just reduce size and exit */
  if (delta > 0L && dmm_gap_size(0,old_curr) < delta )
  { /* new one is bigger and insufficient gap afterwards */

    /* look for a new area to move it to */
    if ( (gap = dmm_gap_search(new_size)) != NULL)
    { /* found one, but I have to move old_curr memory to it */
      /* move existing memory block */
      /* determine #bytes to move, update pointers and move the memory */
      nbytes = SUBHPNTRS(gap,old_curr->mentry);
      goffset = SUBHPNTRS(gap_alloc,alloc_beg); /* save this offset to gap */
      before = (gap_alloc == NULL);
      dmm_update (1,old_curr,nbytes,-1L,-1L); /* update pntrs and move memory */

   /* now relocate old_curr to gap region */
      old_curr = (ALLOCPNTR) BASEALIGN(alloc_beg,offset);
                        /* align to the base address */
      xdll_goto((void *) old_curr);     /* go to this entry again */
      xdll_delete(0);           /* delete it */
        /* note: user data in old_curr is still valid until I wreck it */
      if (before)    /* is it a gap at head of list */
        xdll_head();            /* move to first entry */
      else
      {
        gap_alloc = (ALLOCPNTR) BASEALIGN(alloc_beg,goffset);
                                /* recover gap_alloc */
        xdll_goto((void *) gap_alloc);   /* move to entry before gap */
      }
      curr = (ALLOCPNTR) xdll_insert(before);  /* insert new entry */
        /* this is guarenteed to be inserted without any memory problem */
        /* the following works since deletes do not destroy user contents */
        /* note that curr and old_curr could be the same sometimes ,so */
        /* do it if they are equal */
      if (SUBHPNTRS(curr,old_curr) != 0L)  /* if old_curr to be vacated */
      { /* move values and clear old_curr pointers */
        curr->user = old_curr->user;      /* move old values */
        curr->mentry = old_curr->mentry;
        curr->nsize = old_curr->nsize;
      }
      new_curr = curr;          /* new_curr is a global */
    }
    else  /* uh oh, you got to pack memory */
    {
      min_beg = NULL;           /* null means there is none yet */
      min_move_cost = mblock_size+1;    /* max movement */
      delta = new_size - old_curr->tsize;  /* amount of bytes needed */
      for (begcurr = old_curr, found=0,xdll_goto((void *)old_curr);
           ;            /* see end of loop for break condition */
           xdll_goto((void *)begcurr), begcurr = (ALLOCPNTR)xdll_prev() )
      {                 /* processing begcurr == NULL is required */
        for (curr = begcurr, move_cost=gap_nbytes=0L, passed_old_curr=0;
             ;          /* see end of loop for break condition */
             curr = next )
        {       /* count gap sizes from curr */
          gap_nbytes += dmm_gap_size(1,curr);   /* total gap size */
          if (curr == NULL)
          {
            is_old_curr = 0;    /* false: old_curr can't be NULL */
            next = (ALLOCPNTR) xdll_head();     /* next is head if NULL */
          }
          else
          {
            is_old_curr = (SUBHPNTRS(curr,old_curr) == 0L);
            if (is_old_curr)
                passed_old_curr = 1;  /* set this as you pass old_curr */
            next = (ALLOCPNTR) xdll_next();     /* go to next */
          }
          /* the cost of the gap immediately after old_curr costs nothing */
          /* cost is based on size of next memory after the gap and if */
          /* next is NULL, there is not next memory to cost you */
          if (!is_old_curr && next != NULL)
            move_cost += next->tsize;  /* cost is size of memory to move */
          if( gap_nbytes >= delta)   /* has total of gaps exceeded needed */
          { /* yes, if past or up to old_curr, accum cost and break loop */
            if (passed_old_curr ||
                (next != NULL && SUBHPNTRS(next,old_curr) == 0L)  )
            {   /* no break until you get past or up to old_curr */
              if(min_move_cost > move_cost) /* accum one with min cost */
              { /* you are less than min cost so far; remember it */
                found = 1;      /* I found one big enough to fit */
                min_move_cost = move_cost;
                min_beg = begcurr;      /* save range to pack over */
                min_end = next;
              }
              break;    /* bread for (curr.. */
            }
          }     /* end if */
          if (next == NULL)
            break;      /* bread if next one is NULL */
        }  /* end for (curr...) */

        if (begcurr == NULL ||  /* already processed head */
           (found && !passed_old_curr) )  /* or found what we need */
          break;        /* break if old_curr not in the sum anymore */
      }  /* end for (begcurr...) */
      if (!found)       /* couldn't find one big enough ?*/
        goto error_return; /* that's bad news; completely out of mem */

      /* now pack the memory down and up around old_curr */
      xdll_goto( (void *)old_curr);     /* go to this position */
      next = (ALLOCPNTR) xdll_next();  /* go to next one */
      offset1 = SUBHPNTRS(min_end,alloc_beg);   /* save this offset */
      dmm_pack_range(min_beg,next,0); /* pack down including old_curr*/
      old_curr = (ALLOCPNTR) ADDHOFFSET(alloc_beg,offset);
                  /* restore new old_curr */
      min_end =  (ALLOCPNTR) ADDHOFFSET(alloc_beg,offset1);
                        /* restore min_end */
      dmm_pack_range(old_curr,min_end,1); /* pack up, not old_curr */
      /* now the gap after old_curr is large enough to expand memory size */
      new_curr = (ALLOCPNTR) ADDHOFFSET(alloc_beg,offset);
                    /* restore new old_curr */
    }  /* else uh oh */
  }
  else
    new_curr = old_curr;                /* its the same */

  new_curr->nrecs = nrecs;            /* just change records and */
  tsize = new_curr->tsize = ROUNDUP(new_size);  /* put on a page boundary */
  /* if size of memory expanded, clear the new end of memory */
  if ( tsize > old_size)
    dmm_memset(ADDHOFFSET(new_curr->mentry,old_size),0,tsize-old_size);
                /* clear trailing nbytes including unused ones */
  if (oldnrecs > nrecs)		/* if removing records */
  {	/* you have to free the records that are being removed */
     dmm_update(0,new_curr,0L,nrecs,oldnrecs-1L); /* free them */
  }
/* Now update sizes of pointer arrays that point to the realloc'd area */
  /* reallocation automatically adjusts records entry for pointers */
  /* that are arrays in allocated areas */
  beg = new_curr->mentry;
  end = ADDHOFFSET(beg,tsize - 1L);
  xdll_use(pntr_id);    /* update makes sure this still works after move */
  for (ppntr = (PNTRPNTR) xdll_head();  /* go through all pntrs in table */
       ppntr != NULL;
       ppntr = (PNTRPNTR) xdll_next()  )
  { /* search through all pointers to this */
    if (ppntr->allocflag != (short int) 0 &&   /* only if array alloc'd area */
        SUBHPNTRS(ppntr->user,beg) >= 0L &&
        SUBHPNTRS(ppntr->user,end) <= 0L   )
          ppntr->nrecs = nrecs; /* set to the new number of records */
  }
  realloc_flag = 1;     /* set to indicate that a reallocation occured */
  retval = FUNCOK;
error_return:
     RESTORE_CURR_LINK_ID(save);
     return (retval);  /* error return */
#endif
}


/*!

dmm_free        Free an allocated memory area.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_free (void ** pntr)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr     ;/*  I  Address of location of alloc'd area. */
#endif

/*!
Description:

This frees an allocated area of memory and all associated duplicate
pointers and pointer references.  It also sets any other pointer references
in the allocation table to NULL if they are pointing to the area being
free'd.

See also:  dmm_calloc().


Example:   See example in dmm_mblock_alloc()

!*/
{
#ifdef DMM_OFF
    free(*pntr);
    return 0;
#else
    savestruct save;
    ALLOCPNTR curr;
    int retval = FUNCBAD;

    SAVE_CURR_LINK_ID(save);       /* save current link list in use */
    FATALCHECK;
    if ( mblock == NULL || pntr == NULL ||
            (curr = dmm_find_alloc(pntr)) == NULL)
       goto error_return;
    /* curr now points at one you are freeing */
    dmm_update(0,curr,0L,-1L,-1L); /* this frees all memory and pointers */
                /* also NULL's any pointers into the memory area being freed */
                /* nothing will move in memory as a result of this call */
                /* but all pointers to area being freed will be set to NULL */
    xdll_goto((void *) curr);   /* return to the entry found above */
    xdll_delete(0);             /* delete the entry from alloc table */
    /* now release all entries which have NULL user entries */
    /* NULL user means it was stored in an area that was free'd */
    while (dmm_find_pntr(NULL) != 0)
    {   /* returns positioned on NULL entry, so just delete it */
      xdll_delete(0);   /* remove it from the list its hopeless */
    }
    while (dmm_find_alloc(NULL) != 0)
    {   /* returns positioned on NULL entry, so just delete it */
      xdll_delete(0);   /* remove it from the list its hopeless */
    }
        /* put some table shrink stuff in here someday */
    realloc_flag = 1;   /* set to indicate that memory reallocation */
                        /* has occurred */
    retval = FUNCOK;
error_return:
     RESTORE_CURR_LINK_ID(save);
     return (retval);  /* error return */
#endif
}

#ifndef DMM_OFF

/*!

dmm_clear       Clear an allocated memory area.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_clear (void ** pntr)

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr     ;/*  I  Address of location of alloc'd area. */
#endif

/*!
Description:

This zeroes out the allocated memory block.  It uses the somewhat
general routine `dmm_memset()` to do so.


See also:  dmm_calloc(), dmm_memset().


Example:   See example in dmm_mblock_alloc()

!*/
{
  savestruct save;
  ALLOCPNTR curr;
  int retval = FUNCBAD;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  FATALCHECK;
  /* now find pntr in the allocation table */
  if ( mblock == NULL || pntr == NULL ||
            (curr = dmm_find_alloc(pntr)) == NULL)
     goto error_return;
  dmm_memset(curr->mentry,0,curr->tsize);       /* call set routine */
  retval = FUNCOK;
error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);  /* error return */
}


/*!

dmm_pntr    Declare a pointer to be automatically updated.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_pntr (int ptype,void ** pntr,unsigned long nrecs,
	      unsigned nsize,int (*cond)())

/*!
Return Value:  0 if OK, -1 if no memory block has been allocated
                           or insufficient memory.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             ptype     ;/*  I  0 - single scalar pointer */
                           /*     >0 - array of pointers outside mem block */
                           /*     <0 - array of pointers in alloc'd area */
void            **pntr    ;/*  I  Address of pointer */
unsigned long   nrecs     ;/*  I  Number of pointers; 1L if single */
                           /*    which means use alloc'd regions nrecs,nsize */
unsigned        nsize     ;/*  I  Step between pointers if nrecs > 1L */
int            (*cond)()  ;/*  I  Function that returns:    */
                           /*      0 - to update pointer, */
                           /*      1 - to not update pointer */
                           /*     If cond NULL, then always update */
#endif
#ifdef DOC
/* look for this dmm2.doc */
#endif /* DOC */
{
#ifdef DMM_OFF
  return 0;
#else
  
  PNTRPNTR ppntr;
  ALLOCPNTR curr;
  savestruct save;
  int retval = FUNCBAD;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  FATALCHECK;

  if (mblock == NULL || pntr == NULL || /* no NULLs allowed */
      dmm_find_alloc(pntr) != NULL ||   /* already used as alloc'd pntr */
      dmm_find_pntr(pntr) != NULL)      /* or already defined */
    goto error_return;          /* the big beeper */

  /* now see if pointer is stored in an allocated region */
  /* if so, and its an array and you must use the alloc regions sizes,records */
  if ( ptype < 0)
  {     /* must be an array that is allocated */
     if ( (curr = dmm_find_pntr_to_alloc(pntr)) != NULL)
     {  /* its to an allocated area; use its values */
       if (SUBHPNTRS(pntr,curr->mentry) >= (nsize = curr->nsize) )
          goto error_return;    /* must point into first structure element */
       nrecs = curr->nrecs;
     }
     else
       goto error_return;
  }
  if (ptype == 0)       /* if scalar specified */
     nrecs = 1L;        /* set for scalar (nsize will not be used ever) */
  else if ( nrecs == 0L || nsize < 1 )  /* don't check for scalar */
    goto error_return;
  /* it is guaranteed by design that another pntr will fit in the list */
  xdll_use (pntr_id);   /* switch to pointer link list */
  if (xdll_links_left() == 0)   /* if no more links, then */
     goto error_return;
  /* can't try realloc because pntr may be in memory that moves and I'll */
  /* lose it.  This could be changed eventually, but probably not a big deal */
  xdll_tail();          /* go to tail; OK if NULL list */
  ppntr = (PNTRPNTR) xdll_insert(0);    /* insert it this time for sure */

  ppntr->allocflag = (ptype < 0) ? (short int) 1 : (short int) 0;
  ppntr->user = pntr;
  ppntr->nrecs = nrecs;
  ppntr->nsize = (nrecs == 0L) ? 0L : nsize;	/* zero for scalar */
  ppntr->cond = cond;           /* save function pointer */

  /* now expand the table if adding one more will overflow */
  if (xdll_links_left() == 0)   /* if no more links, then */
  {  /* realloc some more before you really need them */
    curr = dmm_find_alloc(&pntr_beg);   /* set curr to pntr_beg */
    xdll_use(pntr_id);  /* dmm_find_alloc corrupts table in use */
    if (dmm_realloc((void**)&pntr_beg,(nrecs=curr->nrecs+DMM_INCR)) != 0)
    { /* try a smaller increment if this one doesn't fit */
      if(dmm_realloc((void**)&pntr_beg,(nrecs=curr->nrecs+1)) != 0  )
         goto error_return;     /* you are out of memory and you are screwed */
    }
    xdll_reopen (0,(void *) pntr_beg,(unsigned) nrecs,-1);
  }
  retval = FUNCOK;
error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);
#endif
}


/*!

dmm_free_pntr   Remove a pointer refernce.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_free_pntr (void ** pntr)

/*!
Return Value:  0 if OK, -1 no memory block has been allocated
                           or pntr does not exist in allocation table.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr    ;/*  I  Address of location of pntr reference */
#endif

/*!
Description:

Remove a pointer reference previously specified by dmm_pntr() function.
Duplicate pointers take memory.  They should be removed when you
are finished with them otherwise they will tend to accumulate.


See also:  dmm_calloc(), dmm_pntr().


Example:

!*/
{
  savestruct save;
  int retval = FUNCBAD;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  FATALCHECK;
  if (mblock == NULL || pntr == NULL)
        /* error return if block has not been alloc or pntr NULL */
    goto error_return;

  /* now go after the one you really want to free */
  if (dmm_find_pntr(pntr) != NULL)  /* if found it */
  {
    xdll_delete(0);             /* delete this entry */
    retval = FUNCOK;
  }
/* PUT IN TABLE SHRINK STUFF HERE SOMEDAY */
error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);  /* error return */
}


/*!

dmm_ichk        Do an integrity check on the allocation tables.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_ichk ()

/*!
Return Value:  0 if OK, -1 if the table is corrupted.


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Check to see if the allocation table has been corrupted for any reason.  It
does this by doing a `dmm_walk()` through the table and checking status
returns. This routine also sums up the used and free bytes and puts these
sums in statics `used_bytes`,`free_bytes` and `system_bytes' for the
`dmm_mblock_stats()` routine to used.


See also:  dmm_mblock_alloc(), dmm_walk()


Example:

!*/
{
  int retval = FUNCBAD;
  dmmwalk ret;
  int n;

  if (mblock == NULL)   /* no memory; i.e., no table */
     goto error_return;

  system_bytes = used_bytes = free_bytes = 0L;
  ret.mentry = NULL;   /* set to start at beginning of allocation table */
  while ( (n=dmm_walk(&ret)) != DMM_WALK_END)
  {
    if (n != DMM_WALK_OK)       /* if not OK or END, its corrupted */
      goto error_return;        /* tables blown */
    if (ret.useflag)            /* sum up bytes used and free */
      used_bytes += ret.nbytes; /* total bytes used */
    else
      free_bytes += ret.nbytes; /* total bytes free */
    /* now see if its either alloc_beg or pntr_beg's entry */
    if (SUBHPNTRS(alloc_beg,ret.mentry) == 0L ||        /* alloc table */
        SUBHPNTRS(pntr_beg,ret.mentry)  == 0L   )       /* or pntr_beg */
      system_bytes += ret.nbytes;       /* total bytes used by system */
  }
  retval = FUNCOK;
error_return:
  return (retval);  /* error return */
}

/*!

dmm_walk        Walk through the allocation tables and return information.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_walk (dmmwalk ret)

/*!
Return Value:  DMM_WALK_OK if OK,
               DMM_WALK_BAD if table corrupted,
               DMM_WALK_EMPTY if table empty,
               DMM_WALK_END if at end of table.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
dmmwalk         *ret       ;/* O  Val returned in structure pointed to by ret */
#endif

#ifdef DOC
/*!
Description:

This is similar to the Microsoft `heapwalk` routine, but has a slightly
different structure return.  If `ret->mentry` is NULL, it starts at the
beginning of the table.  If it is not NULL, it continues from the last position.
Returned are information for either allocated areas or free areas (free areas
are also called gaps sometimes).  Free areas are the number of bytes between
allocated areas since the table entries are always in the order of memory.
Since only non-NULL returns are legal for `ret->mentry`, subsequent calls
with the same ret strucuture will continue through the table.

Returns information in the `dmmwalk` structure as follows:

typedef struct _dmmwalk dmmwalk;
struct _dmmwalk {
                int useflag;    /* false (0) is FREE, true (1) means USED */
                void *mentry;    /* entry point address for area allocated */
                unsigned long nbytes;  /* total bytes in the alloc'd area */
                unsigned long nrecs;   /* actual records in the area */
                unsigned int  nsize;   /* actual size of each record */
                        };

See also:  dmm_mblock_alloc().


Example:

!*/
#endif /* DOC */
{
  static ALLOCPNTR curr=NULL;   /* current position; NULL means end */
  static int gapflag;   /* true if checking on a gap size (i.e., free mem) */
  savestruct save;
  HPNTR pntr;
  unsigned long gapsize;
  int flag;
  int retval = DMM_WALK_BAD;

  SAVE_CURR_LINK_ID(save); /* save current link list in use */

  xdll_use(alloc_id);
  if (mblock == NULL || xdll_head() == NULL)
  { /* no memory or no table in memory */
     retval = DMM_WALK_EMPTY;
     goto error_return;
  }
  /* table is guaranteed to have at least one entry if mblock allocated */
  flag = (ret->mentry == NULL); /* true if starting at beginning of list */
  if (flag)
  {
    gapflag = 1;        /* set to check start gap */
    gapsize = dmm_gap_size(1,NULL);  /* get gap at the head of memory */
  }
  else if (curr != NULL)
  {
    xdll_goto((void *) curr);   /* go to this position */
    gapsize = dmm_gap_size(1,curr);
  }
  else          /* curr == NULL */
  {
     retval = DMM_WALK_END;
     goto error_return;
  }

  if (gapflag)  /* if to check the gap */
  {     /* skip them if they are zero size */
     if (gapsize == 0L)         /* if there is no gap */
       gapflag = 0;     /* set so that it picks up next entry */
     else
     {
       ret->useflag = 0;        /* free */
       ret->mentry = (void *) gapbeg;     /* start of this memory block */
       ret->nrecs = ret->nbytes = gapsize;      /* set to gap size */
       ret->nsize = 1;  /* byte size records */
     }
     curr = (ALLOCPNTR) ((flag) ? xdll_head() : xdll_next() );
        /* if start of list, next is head, else next is next */
        /* curr of NULL means hit end of the list */
  }
  if (!gapflag) /* this is not a gap */
  {  /* check eot again since gapflag may have changed since last check */
     if (curr == NULL)  /* hit eot? */
     {
       retval = DMM_WALK_END;
       goto error_return;
     }
     ret->useflag = 1;  /* used */
     pntr = curr->mentry;
     ret->mentry   = (void *) pntr;     /* start of this memory block */
     ret->nrecs   = curr->nrecs;
     ret->nsize   = curr->nsize;
     ret->nbytes  = curr->tsize;
     /* do an integrity check on ->tsize and ->mentry */

     if ((curr->tsize % DMM_PAGE) != 0L  ||  /* must be zero or error */
          curr->nrecs == 0L || curr->nsize == 0L || /* zero sizes */
          (curr->nrecs * curr->nsize) > curr->tsize ||  /* illegal size */
          pntr == NULL ||       /* NULL memory address */
          SUBHPNTRS(pntr,mblock_beg) < 0L ||  /* do range check on mblock */
          SUBHPNTRS(pntr,mblock_end) > 0L )
     {   /* returns != 0 if &pntr, pntr NULL or pntr not pointing to mblock */
        retval = DMM_WALK_BAD;  /* out of range, table is goofed */
        ret->mentry = NULL;      /* set NULL for eot and restart */
        fatal_error = 1;        /* sorry, this is fatal */
        goto error_return;
     }
  }
  gapflag = !gapflag;   /* toggle gap flag */
  retval = DMM_WALK_OK;
error_return:
  RESTORE_CURR_LINK_ID(save);
  return (retval);  /* error return */
}


/*!

dmm_movrecs	Move a block of records in an allocated memory area.

Summary:

#include <ComUtil/comutil.h>
*/

int dmm_movrecs(void ** pntr,unsigned long dstrec,unsigned long srcrec,
                unsigned long nrecs)

/*!
Return Value:  0 if OK, -1 no memory block has been allocated
                           or pntr does not exist in allocation table.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr    ;/*  I  Address of location of alloc'd area */
unsigned long	dstrec    ;/*  I  Destination record number */
unsigned long	srcrec    ;/*  I  Source record number  */
unsigned long	nrecs     ;/*  I  Number of records to move */
#endif

/*!
Description:

Moves a consecutive block of records in an existing allocated memory block
to another location in the same block.  This would typically be used when
inserting a new records in an existing record stream or packing records
in memory.  This routine will update all pointer references in the records
and those that point to the records being moved.

The `pntr` parameter is the one specified in either the
`dmm_malloc` or `dmm_calloc` function calls and must have been allocated.
The destination record number is given by `dstrec` and the source record
number is given by `srcrec`.  The number of records to move is `nrecs`.  Note
all record numbers are `longs`.


See also:  dmm_calloc(), dmm_malloc().


Example:

!*/
{
  int retval = FUNCBAD;
  long nbytes,delta;
  unsigned long start,end;
  ALLOCPNTR curr;

  FATALCHECK;
  if (mblock == NULL || pntr == NULL || /* no NULLs allowed */
      (curr = dmm_find_alloc(pntr)) == NULL ||	/* must exist */
      (srcrec+nrecs) > curr->nrecs || 	/* out of range */
      (dstrec+nrecs) > curr->nrecs ) 	/* out of range */
     goto error_return;
  delta = dstrec - srcrec;	/* difference; could be pos or neg */
	/* pos if dst above src and negative is dst below src */
  if (delta != 0L && nrecs > 0L)/* don't do anything unless somethings moving */
  { /* zero delta means not moving; zero nrecs means no records to move */
    nbytes = curr->nsize * delta; /* bytes of delta for dmm_update */
    /* now update and move the records */
    if(dmm_update(1,curr,nbytes,srcrec,srcrec+nrecs-1) )
      goto error_return;
    /* now free evacuated records */
    /* there are 4 cases to consider related to direction and overlapping */
    if (delta > 0)	/* dst is above src in memory */
    {
       start = srcrec; /* free from start of srcrec */
       /* end depends on whether src overlaps dst or not */
       /* only free up to the beginning of dst unless src doesn't hit it */
       end = srcrec + ((nrecs <= delta) ? (nrecs-1L) : (delta-1L) );
    }
    else	/* dst is below src in memory */
    {
       end  = srcrec + (nrecs - 1L);	/* free up to the end of srcrec */
	/* start depends on whether dst overlaps src */
	/* only free up to the end of dst */
       start = (nrecs <= -delta) ? srcrec : (end + delta + 1L);
    }
    if(dmm_update(0,curr,0L,start,end))	/* free records from start to end */
      goto error_return;
  }
  retval = FUNCOK;
error_return:
  return (retval);  /* error return */
}


/* ==========  Static Functions ============= */


/*!

dmm_update      Update all pointers in the table for an area being moved.

Summary:

#include <ComUtil/comutil.h>
*/

static int dmm_update (int flag,ALLOCPNTR apntr,long nbytes,long srec,long erec)

/*!
Return Value:  0 if OK, -1 if apntr->nrecs was zero.


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int             flag     ;/*   I  >  0 - Update pointers by adding nbytes */
			  /*      <  0 - Update pointer but don't move memory */
                          /*      == 0 - Free pointers and set to NULL */
ALLOCPNTR       apntr    ;/*   I  Pointer to area being moved */
long            nbytes   ;/*   I  How many bytes you are moving the */
                          /*      *apntr memory entry */
                          /*      can be pos or neg depending on direction */
long            srec     ;/*   I  Starting record number (<= 0 is start). */
long            erec     ;/*   I  Ending record num (< 0 if to last record) */
#endif


/*!
Description:

To be used when moving the block identified to 'apntr' to another
area in the mblock memory.  It is also used for releasing pointers that
point to apntr when the memory for apntr is being free'd by dmm_free().

A range of records to move or free can be specified by a starting records 
`srec` and an ending record `erec`.  If `srec` is <= 0, it starts at the 
beginning of the allocated area.  If `erec` is <0, it end at the last record in
the allocated area (so to move the entire array, set `srec` to <= 0 and
`erec` to -1L).  Currently, if `srec` and `erec` are specified as non-zero, 
it is assumed that the records are being copied to another area in memory or
being partially free'd (such as in a shrink realloc) and 
the current allocated area identified by `apntr` is left in position meaning
the apntr area is not being free'd, just pointers to the area identified
by the `srec` and `erec`.

The number of bytes to move is given by `nbytes` and can be either
positive or negative. If `flag` is 1, the pointers are updated by `nbytes` for 
a memory area being moved.  If `flag` is -1, pointers are updated but the 
no memory movement is done (movement will be done elsewhere or not at all).  
If `flag` is 0, it is assumed that apntr's memory
is being free'd and all pointers to apntr's memory are set to NULL.  This
includes all duplicates for apntr also.  In addition to NULL'ing out
pointer values, the links for the duplicates and pointers for the
entry being free'd are also deleted.

This will update all the pointers in the memory block for the entry being
moved and selected by the entry identified by `apntr`.  The number of
bytes it is moving is given by `nbytes`.  It can be either positive
or negative.  It is positive if moving to higher address and negative if
moving to lower addresses.

The technique used is as follows:
- update or free the apntr->mentry pointer value (unless a record
  range was specified, then leave it alone).
- update user entry for apntr.
- update all dmmpntr references by searching the entire allocation table.

If apntr->nrecs is zero, it is assumed that this memory has not been
allocated as yet and is not being pointed to.  Normally a zero size memory
is not allowed, so the only case is when dmm_malloc() is making a new
one by reallocating a zero size one.  In this case, no update is made.

See also:


Example:

!*/
{
  ALLOCPNTR curr;
  PNTRPNTR  ppntr;
  savestruct save;
  HPNTR abeg,aend,hpntr,dest,base;
  unsigned long nrecs, movesize;
  unsigned int nsize;
  long delta;
  int updateall;
  int (*cond)();        /* users cond function in pointer list entry */
  HPNTR *cpntr; /* this is a huge char ** type deal */
  int retval = -1;
  long tempoff;

  SAVE_CURR_LINK_ID(save);      /* save current link list in use */
  if ((nrecs = apntr->nrecs) == 0L ||     /* zero means its being allocated */
      apntr->mentry == NULL ||
      apntr->tsize == 0L  )
    goto error_return;         /* so nothing is pointing to it anywhere */

  nsize = apntr->nsize;
  if (srec < 0L) srec = 0L;	/* set to start */
  if (erec < 0L) erec = nrecs - 1L;	/* end of array */
  updateall = (srec == 0L && erec == (nrecs-1L) ); /* true if copying all */
  /* range check */
  if (!updateall)
  {	/* check legality of srec and erec if not copying all */
	/* also limits move to be within existing allocated block */
    delta = nbytes / (long) nsize;  /* records to move by: must be an integer */
    if (srec > erec || erec >= nrecs || 
        (nbytes % (long) nsize) != 0 ||  /* must be a int of rec size */
        (srec+delta) < 0L || (erec+delta) >= nrecs  )
      goto error_return;
  }
  abeg = ADDHOFFSET(apntr->mentry,(srec * (long)nsize));
  tempoff = (erec == nrecs-1L) ? (apntr->tsize - 1L) : ((erec+1L)*(long)nsize - 1L);
  aend = ADDHOFFSET(apntr->mentry,tempoff);
  /* now adjust the base of area being moved; this is destination addr */
  /* don't do it unless you are copying the entire memory block */
  if (updateall)
  {
    dmm_update_pntr(flag,(HPNTR *)&apntr->mentry,nbytes,NULL);
    ADDRALIGN(apntr->mentry);     /* align address; usually a NOP */
    dest = apntr->mentry;	/* this was offset by nbytes */
    movesize = apntr->tsize;	/* entire block */
  }
  else
  {	/* move is in place, don't touch apntr->mentry */
    dest = ADDHOFFSET(abeg,nbytes);	/* if moving in place */
    movesize = (erec-srec+1)*(long)nsize; /* bytes to move */
  }
  base = apntr->mentry;		/* address to align offset to */
  /* Update user pointers in the allocation table structures */
  xdll_use(alloc_id);
  for (curr = (ALLOCPNTR) xdll_head();
       curr != NULL;    /* go through all entries in the table */
       curr = (ALLOCPNTR) xdll_next()  )
  {
    /* first check what the user pointer is pointing at */
    /* don't update user's alloc pointer unless updateall true */
    /* assumed to always point at the beginning of alloc'd area */
    if (updateall)
    {
      hpntr = *((HPNTR *)curr->user);
      if (SUBHPNTRS(hpntr,abeg) >= 0L && SUBHPNTRS(hpntr,aend) <= 0L)
      { /* only update if pointing into area being moved or free'd */
        dmm_update_pntr(flag,(HPNTR *)curr->user,nbytes,base);
      }
    }
    /* repeat for where the pointer itself is */
    /* updates if users pointer is in the area being moved or free'd */
    hpntr = (HPNTR)curr->user;
    if (SUBHPNTRS(hpntr,abeg) >= 0L && SUBHPNTRS(hpntr,aend) <= 0L)
    { /* only update if pointing into area being moved or free'd */
      dmm_update_pntr(flag,(HPNTR *)&curr->user,nbytes,base);
    }
                                /* free or update pointer to new area */
  } /* for curr */

  /* update all pointer arrays  */
  xdll_use(pntr_id);            /* use pointer linked-list */
  for (ppntr = (PNTRPNTR) xdll_head();  /* go to head */
       ppntr != NULL;
       ppntr = (PNTRPNTR) xdll_next() )
  {
      nrecs = ppntr->nrecs;     /* number of records in the structure */
      nsize = ppntr->nsize;  /* sizeof structure */
      cond = ppntr->cond;       /* user's function */
      /* go through all pointers in structure and update their value */
      for (cpntr = (HPNTR *) ppntr->user; /* starting byte */
          /* cpntr now is address of start of a pointer to update */
           nrecs-- > 0L;        /* for all records in the structure */
           cpntr = (HPNTR *) ADDHOFFSET(cpntr,nsize) )
                        /* next offset element in the structure */
      {
         /* check what the structure is pointing too first */
         hpntr = *((HPNTR *)cpntr);     /* hpntr is contents of pointer */
                /* convert pointer to an absolute address */
         if (SUBHPNTRS(hpntr,abeg) >= 0L && SUBHPNTRS(hpntr,aend) <= 0L)
         { /* only update if pointing into area being moved or free'd */
         /* if user cond func specified, update if it returns true */
         /* if user cond func NULL, do update automatically */
             if (cond == NULL || (*cond)(flag,(void **)cpntr) )
                dmm_update_pntr(flag,(HPNTR *)cpntr,nbytes,base);
         }
         if (flag == 0)		/* special case when freeing */
	 {	/* see if you are pointing to area being free'd */
		/* cpntr's can't be updated directly */
            hpntr = (HPNTR)cpntr;     
                /* convert pointer to an absolute address */
            if (SUBHPNTRS(hpntr,abeg) >= 0L && SUBHPNTRS(hpntr,aend) <= 0L)
		*((HPNTR *)cpntr) = NULL;	/* set to NULL contents */		
         }
      }  /* for cpntr */
     /* repeat for the pointer itself is */
     /* this will update if the user's pointer is in an area being moved */
     /* only update this is freeing or copying the entire alloc'd area */
     /* or the pointer is a scalar (single record) pointer */
     if (ppntr->nsize == 0L ||		/* always do it if scalar */
         updateall	)		/* or if copying the entire array */
     {
        hpntr = (HPNTR)ppntr->user;
        if (SUBHPNTRS(hpntr,abeg) >= 0L && SUBHPNTRS(hpntr,aend) <= 0L)
        { /* only update if pointing into area being moved or free'd */
          dmm_update_pntr(flag,(HPNTR *)&ppntr->user,nbytes,base);
        }
     }
  } /* for ppntr */

  /* now move memory if not freeing */
  if (flag > 0)     /* if updating for a memory move */
      dmm_memcpy(dest,abeg,movesize);

  /* reopen the link lists that may have moved */
  /* just change entry points; sizes stay the same */
  xdll_use(alloc_id);           /* use pointer linked-list */
  xdll_reopen (0,(void *) alloc_beg,0,-1);
  xdll_use(pntr_id);            /* use pointer linked-list */
  ADDRALIGN(pntr_beg);          /* align address */
  xdll_reopen (0,(void *)pntr_beg,0,-1);        /* re-open in case it moved */
  retval = 0;
/* all pointers updated and memory moved */
error_return:
  RESTORE_CURR_LINK_ID(save);
  return(retval);
}


/*!

dmm_update_pntr         Add a value to a pointer if it points into mblock.

Summary:

#include <ComUtil/comutil.h>
*/

static void dmm_update_pntr (int flag, HPNTR * pntr,long ldiff,HPNTR apntr)

/*!
Return Value:  none

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------        */
#ifdef DOC
int             flag     ;/*   I  true - add ldiff to pointer */
                          /*      false - NULL out pointer */
HPNTR *         pntr     ;/*   I  Pointer to pointer to add to */
long            ldiff    ;/*   I  Value to add to it */
HPNTR           apntr    ;/*   I  Address to offset *pntr relative to */
                          /*      Ignored if NULL */
#endif

/*!
Description:

Add a constant `ldiff` to a pointer `pntr`.  Will not add it if `pntr` is
NULL. Note that `ldiff` can be either positive or negative.
If `flag` is false, the pointer is just set to NULL.
The `apntr` parameter is the base address of the area that
`*pntr` is pointing to.  This routine makes sure `*pntr` is equal to
`apntr` plus the offset to `*pntr` so that is is aligned relative to this
base address.  This latter feature is only needed in wierd segment/offset
type machines like Intel 386, etc., and normally is a total waste of time.
It is assumed that `apntr` has already been corrected by `ldiff` before
entering this routine.

See also:


Example:

!*/
{
  unsigned long offset;
  if (pntr != NULL && *pntr != NULL)    /* NULL's are out of there */
  {
    if (flag)
    { /* update pointer; make sure its pointing inside mem block */
         *pntr=ADDHOFFSET(*pntr,ldiff); /* point to new place */
         if (apntr != NULL)     /* align relative to apntr if not NULL */
         {
           offset = SUBHPNTRS(*pntr,apntr);  /* offset from aligned base */
                /* offset is always >= 0 or something very wrong */
           *pntr = (offset <= DMM_MAXUNSIGNED) ?        /* base align if */
                   BASEALIGN(apntr,offset) :  /* less than unsigned */
                  ADDHOFFSET(apntr,offset); /* align relative to base */
         }
    }
    else
      *pntr = NULL;     /* just set contents to NULL to indicate free */
  }
}

/*!

dmm_find_alloc          Find an allocated memory area.

Summary:

#include <ComUtil/comutil.h>
*/

static ALLOCPNTR dmm_find_alloc (void ** pntr)

/*!
Return Value:  The address of alloc'd areas structure.  NULL if not found.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr     ;/*  I  Address of location of alloc'd area. */
#endif

/*!
Description:

Searches through the allocation table for a `user` entry that matches
`pntr`.   It does this by searching the table.  I considered having
a current state so that mulitple calls with the same pntr would not have
to research, but I don't see it being of much purpose and its a pain in
the but to maintain.  NOTE: this routine changes the current xdll table
in use to `alloc_id` and does not restore the one used on entry, so be
careful.


See also:  dmm_calloc().


Example:   See example in dmm_mblock_alloc()

!*/
{
    ALLOCPNTR curr;

    xdll_use (alloc_id);        /* use allocation table link list */
    for(curr = (ALLOCPNTR) xdll_head(); /* go to the head */
        curr != NULL;                   /* while not at end or no list */
        curr = (ALLOCPNTR) xdll_next()   )      /* go to next one */
    {
       if ((pntr == NULL && curr->user == NULL) ||
            SUBHPNTRS(pntr,curr->user) == 0L)
       {        /* diff of zero means equal of course */
          return(curr); /* found it, return its address */
       }
    }
    return (NULL);  /* error return */
}


/*!

dmm_find_pntr    Find a reference pointer.

Summary:

#include <ComUtil/comutil.h>
*/

static PNTRPNTR dmm_find_pntr (void ** pntr)

/*!
Return Value:  The address of pointer link-list structure.  NULL if not found.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr   ;/*  I  Address of pointer reference to find. */
#endif

/*!
Description:

Searches through the pointer tables for a match with between `pntr` and the
`user` entry. It does this by a linear search until it finds it or hits the
end.  Note that if `pntr` is NULL, it will find entries that point to no
where (garbage).  Returns positioned on the entry if found or NULL if not
found.  NOTE: This routine changes the current table in use to `pntr_id`.

See also:


Example:   See example in dmm_mblock_alloc()

!*/
{
  PNTRPNTR ppntr;
  PNTRPNTR retval=NULL;
  ABSPNTR abspntr,absbeg,absend;        /* no doubt unsigned longs */

  PNTR_TO_ABS(pntr,abspntr);    /* used later */
  xdll_use(pntr_id);    /* use dup table */
  for (ppntr = (PNTRPNTR) xdll_head();   /* through all dups */
       ppntr != NULL;
       ppntr = (PNTRPNTR) xdll_next() )
  {
   /* first see if pntr and ppntr->user are NULL (garbage entry) */
     if (pntr == NULL && ppntr->user == NULL)
     {  /* looks for first NULL entry if requested */
        retval = ppntr;
        break;          /* first NULL, break for loop and exit */
     }
   /* make sure pntr is not equal to  (ppntr->user) + i   where */
   /*                                   i = 0,1,2,...,ppntr->nrecs-1 */
   /* As you know, you cannot use modulo operators on pointers, so */
   /* I am forced to use the PNTR_TO_ABS() macros */
   /* the alternative is brute force search, which would be a dog */
    PNTR_TO_ABS(ppntr->user,absbeg);    /* beginning of memory area */
    if (abspntr == absbeg)      /* if equal to start, just exit */
    {
       retval = ppntr;
       break;
    }
    if (ppntr->nsize > 0)       /* if an array (i.e., not a scalar) */
    {
      if (abspntr < absbeg ||       /* if before this memory block */
          abspntr > (absend=absbeg+(ppntr->nrecs-1)*ppntr->nsize)  ) /*last*/
         continue;              /* outside this memory section */
      /* at this point you know its inside the address space of the array */
      /* now comes the modulo operator; to find out if its in first element */
      if ( (((unsigned long) (abspntr - absbeg)) % ppntr->nsize ) == 0L)
      { /* this means you found one of them in the pointer array */
         retval = ppntr;        /* found it, return links address */
         break; /* break for loop, found it */
      }
    }
  }
  return (retval);
}

/*!

dmm_find_pntr_to_alloc   See if a pntr is into the alloc table memory space.

Summary:

#include <ComUtil/comutil.h>
*/

static ALLOCPNTR dmm_find_pntr_to_alloc (void ** pntr)

/*!
Return Value:  The address of alloc link-list structure.  NULL if not found.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
void            **pntr   ;/*  I  Address of pointer reference to find. */
#endif

/*!
Description:

Searches through the alloc table to see if `pntr` is in the memory space
of an allocation table entry.  If so, the address of the alloc table
entry is returned.  If not, a NULL is returned to indicate it does not
point into this.  This is handy for determining the number of records and
size of the structure that a pointer points to.

See also:


Example:   See example in dmm_mblock_alloc()

!*/
{
  ALLOCPNTR curr;
  HPNTR beg,end;

  if (SUBHPNTRS(pntr,mblock_beg) >= 0L &&
      SUBHPNTRS(pntr,mblock_end) <= 0L  )  /* if in mblock */
  { /* find out which curr entry it is in */
     xdll_use(alloc_id);        /* use alloc table */
     for (curr = (ALLOCPNTR) xdll_head();   /* through all alloc entries */
          curr != NULL;
          curr = (ALLOCPNTR) xdll_next() )
     {
        beg = curr->mentry;
        end = ADDHOFFSET(beg,curr->tsize-1L);
        if (SUBHPNTRS(pntr,beg) >= 0L &&
            SUBHPNTRS(pntr,end) <= 0L  )  /* if in alloced area */
                 return(curr);          /* return it */
     }
  }
  return (NULL);
}

/*!

dmm_gap_search     Search for a gap >= size requested on a page boundary.

Summary:

#include <ComUtil/comutil.h>
*/

static HPNTR dmm_gap_search (unsigned long size)

/*!
Return Value:  Address of start of the gap if found, NULL if not found.
                (puts gap size in gap_size and preceeding alloc table
                        entry in gap_alloc global statics ).

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
unsigned long   size     ;/*   I  Number of bytes to look for */
#endif

/*!
Description:

Searches through the allocation table for a gap that is as large as
or exceed the gap size requested.  Leaves the alloc table positioned
on the current gap.  The gap must start on a page boundary.

See also:


Example:

!*/
{
  ALLOCPNTR curr;
  savestruct save;
  HPNTR retval = NULL;

/* static globals: unsigned long gap_size;   ALLOCPNTR gap_alloc; */

  SAVE_CURR_LINK_POS(save);     /* save current link list in use */
  xdll_use (alloc_id);  /* use allocation table link list */
  curr = NULL;          /* start at beginning of memory */
  do
  {
     if ((gap_size = dmm_gap_size(1,curr)) >= size)  /* save gap_size */
     {  /* diff of zero means equal of course */
        gap_alloc = curr; /* alloc'd area just before the gap;NULL beg mem */
        retval = gapbeg;        /* found one, return it */
        break;  /* break do {..} while */
     }                  /* dmm_gap_size() sets gapbeg */
     curr = (curr == NULL) ?    /* first time through, set to head */
       (ALLOCPNTR) xdll_head()  :
       (ALLOCPNTR) xdll_next(); /* otherwise set to next */
  } while (curr != NULL);

  RESTORE_CURR_LINK_POS(save);
  return (retval);  /* return not found indicator */
}

/*!

dmm_gap_size    Find size of memory gap after alloc_id alloc table entry.

Summary:

#include <ComUtil/comutil.h>
*/

static unsigned long dmm_gap_size(int flag,ALLOCPNTR curr)

/*!
Return Value:  Number of bytes in the gap.  0 if no gap.
                (beginning of gap left in gapbeg static )
Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
int              flag    ;/*   I  true - find size of gap on next page */
                          /*              boundary after curr */
                          /*      false - find gap from end of user memory */
                          /*              in curr */
ALLOCPNTR        curr    ;/*   I  Alloc table entry to check gap after */
                          /*      NULL means check gap at head of memory */
#endif

/*!
Description:

Finds the size of the memory gap after the current allocation table
entry identified by the `curr` allocation table pointer.  If `flag` is
true, the beginning of the gap is on the first page after the end of `curr`.
This is found by adding `curr->tsize` to the `curr->mentry`.  If `flag` is
false (0), then the gap is measured from the end of the user memory or
`curr->mentry` + `curr->nrecs * curr->nsize`.  A false value for `flag` is
only used for walking through memory (actual user available memory) and for
measuring the gap on the end of a memory to check if it can be expanded in
size in place.  Otherwise, gaps are almost always measure relative to a
page boundary start (note that curr->tsize is always a multiple of the
page size DMM_PAGE and that curr->mentry always starts on the boundary of
at page; thus the gap size is always a multiple of the number of pages).

See also:


Example:

!*/
{
  ALLOCPNTR next;
  HPNTR gapend;
  unsigned long size;
/* static global:  HPNTR gapbeg */
  savestruct save;

  SAVE_CURR_LINK_POS(save);     /* save current link list in use */

  /* Look through ALL of the alloc entries dup links for duppntr */
  xdll_use (alloc_id);  /* use allocation table link list */

  if (curr == NULL)     /* head of memory indicator ? */
  {
    gapbeg = mblock_beg;  /* start of memory block */
    next = (ALLOCPNTR) xdll_head();  /* head is next */
  }
  else  /* there is a block there */
  {
    xdll_goto ((void *) curr);  /* goto this location */
    size = (flag) ? curr->tsize : (curr->nrecs * curr->nsize);
    gapbeg = ADDHOFFSET(curr->mentry, size);
                     /* start of next gap */
    next = (ALLOCPNTR) xdll_next();  /* goto next */
  }
  if ( next == NULL)
  { /* end of memory, use end of mblock */
    gapend = mblock_end;        /* end of gap address */
  }
  else  /* there is a next entry */
  {
    gapend = ADDHOFFSET(next->mentry,-1L);      /* set to end of gap address */
  }
/* even though curr->tsize is the size of the allocated area, the */
/* real used area is size and the remaining bytes from size to tsize */
/* are considered part of a gap since they can be expanded into for reallocs */
/* but since this delta is less than a page size, it won't be used for alloc */

  RESTORE_CURR_LINK_POS(save);
  return ((unsigned long) SUBHPNTRS(gapend,gapbeg) + 1 );
}


/*!

dmm_memcpy   Copy any size block of potentially overlapping blocks of memory.

Summary:

#include <ComUtil/comutil.h>
*/

static int dmm_memcpy (HPNTR dest,HPNTR src,unsigned long count)

/*!
Return Value:  0 if OK, -1 if illegal parameters.


Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
HPNTR           dest      ;/* I  Destination Address  */
HPNTR           src       ;/* I  Source Address       */
unsigned long   count     ;/* I  Number of bytes to move */
#endif

/*!
Description:

Copies any size block of memory from the `src` address to `dest`.  The
number of bytes to move is specified by `count`.

Handles overlapping areas as well.  Moving blocks up to size unsigned is
handled by the MEMCPY macro, but sizes larger than unsigned (as is the case
with MSDOS, but not OS9), you have to do fragmented block moves.  The memory
must be broken and moved in pages.  Each page covering the range of an
unsigned (or less as is the case with this implementation).  This is no problem
if the src and dest memory areas are not overlapping, but when they
overlap, you have to move them is a way so that memory is not corrupted.  There
are two cases to consider:

(1) src before dest in memory, overlapping,
(2) dest before src in memory, overlapping.

Case 1 must move pages in reverse from the end of src to the beginning of src.
Case 2 is OK since the src moves up and won't corrupt.

Note:   These assume that pointer boolean and long add routines work for
        Microsoft-C huge pointers.

See also:


Example:

!*/
{
  unsigned int pages,remains;
  HPNTR srcend,destend;
  unsigned maxcount = DMM_MAXCOUNT;

  if (dest == NULL || src == NULL)      /* get those NULL's out of here */
     goto error_return;

  if (count > 0 && SUBHPNTRS(src,dest) != 0) /* not very interesting if equal */
  { /* determine number of pages and remainder */
    srcend = ADDHOFFSET(src, (count - 1)); /* calculate ending addresses */
    DMM_PAGES(count,pages,remains);
    /* check for case where there are pages and src is before dest and */
    /* they overlap each other */
    if (SUBHPNTRS(src,dest) < 0L && /* if src begin is before dest begin */
        SUBHPNTRS(srcend,dest) >= 0L)    /* and src end overlaps dest begin */
    { /* move all full pages in reverse */
      for ( srcend=ADDHOFFSET(srcend,1L),       /* one past the end */
            destend = ADDHOFFSET(dest,count);     /* also one past the end */
            pages > 0;          /* for all pages */
            pages--           )
      {
         srcend=ADDHOFFSET(srcend,-maxcount);/* back up to start of block */
         destend=ADDHOFFSET(destend,-maxcount);
         MEMCPY(destend,srcend,maxcount);
      }
    }
    else  /* move all full pages forward */
    { /* this is for non overlapping case and src above dest */
      for ( ;   /* src and dest point to start of move area */
            pages > 0;          /* for all pages */
            pages--,src=ADDHOFFSET(src,maxcount),
                    dest=ADDHOFFSET(dest,maxcount))   /* to next one */
      {
         MEMCPY(dest,src,maxcount);
      }
    }
    /* src and dest point at start of what remains to be moved */
    /* all pages moved, now move remains */
    if (remains > 0)    /* shouldn't need this */
       MEMCPY(dest,src,remains);
  } /* end if src != dest */

  return(FUNCOK);

error_return:
     return (FUNCBAD);  /* error return */
}

/*!

dmm_memset      Set memory to value for huge pointers and long counts.

Summary:

#include <ComUtil/comutil.h>
*/

static int dmm_memset (HPNTR dest,int c,unsigned long count)

/*!
Return Value:  0 if OK, -1 if illegal parameters.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
HPNTR           dest      ;/* I  Destination Address  */
int             c         ;/* I  Char to set it to */
unsigned long   count     ;/* I  Number of bytes to set */
#endif

/*!
Description:

This sets a `count` bytes of a memory block starting at `dest` to a value `c`.
It supports an unsigned long sized count, which `memset()` does not in
Microsoft.  It uses a paging method for machines which have less than
32-bit ints.  Note that this is essentially a memset() function if the
sizeof(long) is the same as sizeof(int).

See also:

Example:

!*/
{
  unsigned int pages,remains;

  if (dest == NULL)
    goto error_return;
  /* clear all count bytes */
  /* do it in pages for pitiful 16-bit machines */
  DMM_PAGES(count,pages,remains);
  /* clear all whole pages */
  for (;
       pages > 0;
       dest=ADDHOFFSET(dest,DMM_MAXCOUNT),--pages )
    MEMSET(dest,c,DMM_MAXCOUNT);  /* set all bytes to zero */
  /* clear remainder of last page */
  MEMSET(dest,c,remains);       /* set all bytes to zero */
  return(FUNCOK);
error_return:
  return (FUNCBAD);  /* error return */
}


/*!

dmm_pack_range          Pack a range of the allocation table up or down.

Summary:

#include <ComUtil/comutil.h>
*/

static void dmm_pack_range (ALLOCPNTR beg,ALLOCPNTR end,int upflag)

/*!
Return Value:  None.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
ALLOCPNTR        beg     ;/*   I  Starting allocation table position */
                          /*      NULL means from start of memory block */
ALLOCPNTR        end     ;/*   I  Ending allocation table position */
                          /*      NULL means to the end of memory block */
int             upflag   ;/*   I  false - pack memory down in memory */
                          /*      true  - pack memory up in memory */
#endif
/*!
Description:

Packs a range of memory bounded by the allocation table entries `beg` and
`end`.  It will eliminate all the gaps between these two allocation regions.
The `upflag` when true will pack memory up against the start of the `end`
memory, while if false will pack memory down agains the end of the `beg`
memory.

This routine is broken into two symmetric parts, one for each state of
upflag.  They are quite similar and maybe can be merged into one loop
sometime by someone more clever than me.

Improvements:

- Add a total amount to pack parameter since the entire last gap on
  a pack does not necessarily have to be moved;  this current
  implementation is pretty inefficient for large block packing.


See also:


Example: To pack all memory down, do a dmm_pack_range(NULL,NULL,0)

!*/
{
  HPNTR abeg,aend;
  unsigned long offset;
  long nbytes;
  int error=0;
  savestruct save;

  SAVE_CURR_LINK_ID(save); /* save current link list in use */

  xdll_use (alloc_id);  /* use allocation table */
  if (!upflag)          /* if packing down */
  {
    aend = (end == NULL) ?    /* NULL means go to the end of memory */
       ADDHOFFSET(mblock_end,1L) :       /* 1 past the end; on a page */
       end->mentry;    /* go to beginning of end entry */

    while (!error)      /* can only get out by breaking out */
    {
      if (beg != NULL)
        xdll_goto((void *)beg);           /* get back where you were */
      nbytes = dmm_gap_size(1,beg);       /* get gap between me and next guy */
      beg = (beg == NULL) ?  /* NULL means start at beginning of memory */
        /* this can only be done once in this while loop */
         (ALLOCPNTR) xdll_head() :        /* this is next entry */
         (ALLOCPNTR) xdll_next();        /* move past end of gap */

      /* if beg->mentry is >= end->mentry, break the while */
      if (beg == NULL)  /* nothing after gap, you're done */
        break;
      if (SUBHPNTRS(beg->mentry,aend) >= 0L) /* you went past end marker,done */
        break;
      offset = SUBHPNTRS(beg,alloc_beg);  /* remember in case alloc moves */
      dmm_update (1,beg,-nbytes,-1L,-1L); /* update the entry you are moving */
      beg = (ALLOCPNTR) BASEALIGN(alloc_beg,offset);       /* recover */
    }  /* end while () */
  }  /* end of if (!upflag) */
  else  /* upflag is true; pack memory up */
  {
    abeg = (beg == NULL) ?   /* NULL means go to the beg of memory */
       ADDHOFFSET(mblock_beg,-1L)  :     /* one less than on a page boundary */
       beg->mentry;          /* go to end of beg entry */

    while (!error)      /* can only get out by breaking out */
    {
      if (end != NULL)
        xdll_goto((void *)end);         /* get back where you were */
      end = (end == NULL)  ? /* NULL means start at end of memory */
        /* this can only be done once in this while loop */
         (ALLOCPNTR) xdll_tail() : /* tail is next entry */
         (ALLOCPNTR) xdll_prev();  /* move to prev to calc gap */

      if (end == NULL)  /* nothing before the gap, you're done */
        break;
      if (SUBHPNTRS(end->mentry,abeg) <= 0L) /* you went past end marker,done */
        break;
      nbytes = dmm_gap_size(1,end);       /* get gap between me and next guy */
      offset = SUBHPNTRS(end,alloc_beg);
      dmm_update (1,end,nbytes,-1L,-1L); /* update the entry you are moving */
      end = (ALLOCPNTR) BASEALIGN(alloc_beg,offset);    /* recover */
    }  /* end while () */
  }  /* end of if (!upflag) */

  RESTORE_CURR_LINK_ID(save);
}


/*!

dmm_sub_pntrs          Take long difference of two huge pointers.

Summary:

#include <ComUtil/comutil.h>
*/

static long dmm_sub_pntrs (HPNTR pntr1,HPTNR pntr2)

/*!
Return Value:  The difference pntr1 - pntr2.  Also returns compare states:
               0 if pntr1 == pntr2,
              <0 if pntr1 <  pntr2,
              >0 if pntr1 >  pntr2.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
HPNTR           pntr1   ;/*   I  Pointer 1 to compare */
HPNTR           pntr2   ;/*   I  Pointer 2 to compare */
#endif

/*!
Description:

Converts pointers to absolute addresses and subtracts them and returns
the difference as a long integer.  This routine is only needed due to
Microsoft returning an int when differencing huge pointers instead of the
more logical long that it should.  Maybe new versions of their
compiler will eliminate this routine.  Currently I only use the macro
SUBHPNTRS() to difference pointers; it in turn calls this functions.



See also:


Example:

!*/
{
  ABSPNTR abs1,abs2;

  PNTR_TO_ABS(pntr1,abs1);      /* macros */
  PNTR_TO_ABS(pntr2,abs2);
  return( (long) (abs1 - abs2));
}


/*!

dmm_add_offset          Add a long offset to a huge pointer.

Summary:

#include <ComUtil/comutil.h>
*/

static HPNTR dmm_add_offset (HPNTR pntr,long offset)

/*!
Return Value:  The pointer (pntr + offset).

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */
#ifdef DOC
HPNTR           *pntr    ;/*   I  Pointer to add offset to */
long            offset   ;/*   I  Amount to add (pos or neg) */
#endif

/*!
Description:

Converts pointer to absolute address and adds a long offset.
This routine is only needed due to C-terp not handling huge pointer arith.
Use macro ADDHOFFSET(); not this direct call.


See also:


Example:

!*/
{
  ABSPNTR abs;
  HPNTR retpntr;

  PNTR_TO_ABS(pntr,abs);        /* macros */
  abs += offset;        /* add offset */
  ABS_TO_PNTR(abs,retpntr);
  return ( retpntr);
}

/*!

dmm_not_portable        Check to make sure dmm routines are portable

Summary:

#include <ComUtil/comutil.h>
*/

static int dmm_not_portable()

/*!
Return Value:  0 if portable, -1 if not portable.

Parameters:

Type            Name          IO  Description
------------    -----------   --  -----------                  */

/*!
Description:

Checks to make sure assumptions about pointer sizes and their conversions
are valid.  Should check things pretty well for C_terp and Microsoft C on
MSDOS and on OS-9.


See also:


Example:

!*/
{
#define DELTA   0x1ffffL       /* some delta */
  HPNTR pntr,pntr1,pntr2;
  ABSPNTR abs1,abs2;

  if (sizeof(ABSPNTR) < sizeof(HPNTR) ||
      sizeof(ABSPNTR) < 4  )    /* basic size checks */
    goto error_return;

  pntr1 = (HPNTR) 0x2fffffffL;  /* some fake 30 bit address */
  PNTR_TO_ABS(pntr1,abs1);
  pntr2 = ADDHOFFSET(pntr1,DELTA);        /* add a big number */
  PNTR_TO_ABS(pntr2,abs2);
  if ( SUBHPNTRS(pntr2,pntr1) !=   DELTA ||
       SUBHPNTRS(pntr1,pntr2) != -DELTA ||
       (abs2 - abs1) != DELTA || (abs1 - abs2) != -DELTA)
    goto error_return;
  ABS_TO_PNTR(abs1,pntr1);      /* convert back */
  ABS_TO_PNTR(abs2,pntr2);      /* convert back */
  if ( SUBHPNTRS(pntr2,pntr1) !=  DELTA ||
       SUBHPNTRS(pntr1,pntr2) != -DELTA )
    goto error_return;          /* should still be same diff */
  pntr = ADDHOFFSET(pntr2,-DELTA);
  PNTR_TO_ABS(pntr,abs2);       /* see if same as abs1 again */
  if (abs1 != abs2)             /* better be same absolute address or else */
     goto error_return;

  ADDRALIGN(pntr);              /* see if align function works */
  PNTR_TO_ABS(pntr,abs2);       /* does not effect pntr abs value */
  if (abs1 != abs2)             /* better be same absolute address or else */
     goto error_return;

  return(0);
error_return:
  fatal_error = 1;              /* this is a fatal error */
  return(-1);
}

#endif /* #ifndef DMM_OFF */

/* ===  END OF FILE === */

