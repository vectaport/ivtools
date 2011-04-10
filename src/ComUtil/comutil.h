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

#ifndef COMUTIL_INCLUDED

#define COMUTIL_INCLUDED

#include <ComUtil/comterp.err>
#include <ComUtil/util.h>
#include <ctype.h>
#include <stdio.h>

/* ================================================================== */

/* ============== START of Macro and Definitions ==================== */

#define DMM_OFF

/* Macro to supply system-independent terminal name */
#if defined(MSDOS)
#define TERMINAL "CON"
#endif
#if defined(OSK)
#define TERMINAL "/t1"
#endif

/* This declares a pointer reference with no conditional function */

#define dmm_pntr_scalar(pntr)  dmm_pntr(0,(void **)(pntr),1L,0,NULL)
#define dmm_pntr_array(pntr,nrecs,nsize) \
  dmm_pntr(1,(void **)(pntr),(unsigned long)(nrecs),(unsigned)(nsize),NULL)
#define dmm_pntr_alloc(pntr)  dmm_pntr(-1,(void **)(pntr),0L,0,NULL)

/* Flags used as returns from dmm_walk() in DMM.C */
#define  DMM_WALK_OK      0
#define  DMM_WALK_BAD    -1
#define  DMM_WALK_EMPTY  -2
#define  DMM_WALK_END    -3

/* Token types returned from LEXSCAN.C */
#define TOK_NONE        0       /* No token found */
#define TOK_IDENTIFIER  1       /* Identifier, i.e. command name */
#define TOK_OPERATOR    2       /* Operator */
#define TOK_STRING      3       /* Character string constant */
#define TOK_CHAR        4       /* Character constant */
#define TOK_DFINT       5       /* Default integer */
#define TOK_DFUNS       6       /* Default unsigned integer */
#define TOK_LNINT       7       /* Long integer */
#define TOK_LNUNS       8       /* Long unsigned integer */
#define TOK_FLOAT       9       /* Floating point number */
#define TOK_DOUBLE      10      /* Double-size floating point number */
#define TOK_EOF         11      /* End of file */

/* Token types never returned from LEXSCAN.C */
#define TOK_WHITESPACE  12      /* Spaces, tabs, new-lines, control chars */
#define TOK_OCT         13      /* Octal token */
#define TOK_HEX         14      /* Hexadecimal token */
#define TOK_COMMENT     15      /* Comment */

/* Levels for error system output */
#define USER_LEVEL	0
#define	PROG_LEVEL	1

/* Character type checking */
#define isident( ch )   ( isalpha( ch ) || (ch) == '_' )
#define isodigit( ch )  ( (ch) >= '0' && (ch) <= '7' )
#define isquote( ch )   ( (ch) == '\'' || (ch) == '"' )
#define isrparen( ch )   ( (ch) == ')' || (ch) == ']' || (ch) == '}')


/* Error handling macros */
#define COMERR_SET( status )\
comerr_set( status, fprintf( err_fileio(), comerr_read( status )))

#define COMERR_SET1( status, val1 )\
comerr_set( status, fprintf( err_fileio(), comerr_read( status ), val1 ))

#define COMERR_SET2( status, val1, val2 )\
comerr_set( status, fprintf( err_fileio(), comerr_read( status ), val1, val2 ))

#define COMERR_SET3( status, val1, val2, val3 )\
comerr_set( status, fprintf( err_fileio(), comerr_read( status ), val1, val2, val3 ))


/* =================== END  of Macro and Definitions ================ */

/* ================================================================== */

/* ================  START of Structure Declarations ================ */

/* ===  For XDLL.C ==== */
/*  This structure is used by the linked list routines XDLL.C */
/* They use int's instead of pointers to be easily relocatable  */
/* This means max link list is 32K bytes; but that's ok for now */

typedef struct _xdllink xdllink;
struct _xdllink {	/* these are BYTE OFFSETS */
		int prev;   /* prev link in the chain; <0 is top */
		int next;   /* next link in the chain; <0 is end */
	        };

/* === structure used with dmm_walk() routine in DMM.C === */

typedef struct _dmmwalk dmmwalk;	
struct _dmmwalk {
		int useflag;	/* false (0) is FREE, true (1) means USED */
		void *mentry;	/* entry point address for area allocated */
		unsigned long nbytes;  /* total bytes in the alloc'd area */
		unsigned long nrecs;   /* actual records in the area */
		unsigned int  nsize;   /* actual size of each record */
			};

/* ======= END of Structure Declarations ======= */

/* Package function prototypes */
#if !defined(OSK)
#include <ComUtil/comutil.arg>
#endif

#if defined(OSK)
/* -- Insert definitions of any function that does not return an integer -- */
/* DMM.C */ 
/* All int functions */
int dmm_mblock_free();
/* XDLL.C */ 
void *xdll_curr();
void *xdll_head();
void *xdll_next();
void *xdll_prev();
void *xdll_tail();
void *xdll_insert();
void *xdll_goto();
/* SYMBOLS.C */
char *symbol_pntr();
/* ERRFILE.C */
char *err_readfile();
/* ERRSYS.C */
char *err_read();
FILE *err_fileio();
/* COMERR.C */
char *comerr_read();
/* FUNCPTRS */
int ffeof();
int fferror();
/* TXTUTIL */
unsigned int txtstore();
unsigned int txtread();
unsigned int txtopenclose();
unsigned int txtkwsrch();
/* unsigned int txtprint(); */
#endif

typedef char* (*infuncptr)(char*,int,void*);
typedef int (*eoffuncptr)(void*);
typedef int (*errfuncptr)(void*);
typedef int (*outfuncptr)(const char*, void*);

extern infuncptr _oneshot_infunc;  /* to inform parser of one-shot infunc */

#endif /* not COMUTIL_INCLUDED */

