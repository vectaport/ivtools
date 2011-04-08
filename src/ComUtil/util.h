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

#ifndef UTIL_INCLUDED

#define UTIL_INCLUDED

#ifdef MSDOS
#define DFINT_SCFORM "%d"
#define DFUNS_SCFORM "%u"
#define SHINT_SCFORM "%hd"
#define SHUNS_SCFORM "%hu"
#define LNINT_SCFORM "%ld"
#define LNUNS_SCFORM "%lu"
#define FLOAT_SCFORM "%f"
#define DOUBL_SCFORM "%lf"
#define DFINT_PRFORM "%d"
#define DFUNS_PRFORM "%u"
#define SHINT_PRFORM "%hd"
#define SHUNS_PRFORM "%hu"
#define LNINT_PRFORM "%ld"
#define LNUNS_PRFORM "%lu"
#define FLOAT_PRFORM "%g"
#define DOUBL_PRFORM "%g"
#endif

#ifndef MSDOS
#define DFINT_SCFORM "%d"
#define DFUNS_SCFORM "%d"
#define SHINT_SCFORM "%hd"
#define SHUNS_SCFORM "%hd"
#define LNINT_SCFORM "%d"
#define LNUNS_SCFORM "%d"
#define FLOAT_SCFORM "%f"
#define DOUBL_SCFORM "%lf"
#define DFINT_PRFORM "%d"
#define DFUNS_PRFORM "%u"
#define SHINT_PRFORM "%d"
#define SHUNS_PRFORM "%d"
#define LNINT_PRFORM "%ld"
#define LNUNS_PRFORM "%lu"
#define FLOAT_PRFORM "%g"
#define DOUBL_PRFORM "%g"
#endif

/* BOOLEAN DEFINITIONS */
#define         BOOLEAN         unsigned int
#ifndef TRUE
#define         TRUE            1
#endif
#ifndef FALSE
#define         FALSE           0
#endif

#if !defined(__cplusplus)
#ifndef true
#define         true            1
#endif
#ifndef false
#define         false           0
#endif
#ifndef boolean
#define         boolean         unsigned int
#endif
#else
#include <OS/enter-scope.h>
#endif

/* Inverse of a NULL pointer */
#define         LLUN            ((char *)-1L)

/* Extra general purpose stuff */
#ifndef nil
#define         nil             NULL
#endif

/* Handles of standard files */
#define STDIN_HANDLE  0
#define STDOUT_HANDLE 1
#define STDERR_HANDLE 2

/* fseek constants */
#ifndef MSDOS
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

extern  int   Kaput_On;
#ifndef TITLE
extern  int   TITLE;
#endif

#if !defined(min) && !defined(COMUTIL_NOMINMAXDEF) 
#define  min(a,b) (a<b?a:b)
#endif
#if !defined(max) && !defined(COMUTIL_NOMINMAXDEF)
#define  max(a,b) (a>b?a:b)
#endif

/* Return Status for functions */
#define         FUNCOK          0
#define         FUNCBAD         -1
#define         FUNCHELP        1

/* Miscellaneous Definitions */
#define         MAXLINE         511
#if defined(MSDOS)
#define         PATH_LEN        78
#define         DIR_DELIM       '\\'
#define         PATH_DELIM      ';'
#define         DEFAULT_MODE    3
#endif

#if !defined(MSDOS)
#define         PATH_LEN        256
#define		DIR_DELIM	'/'
#define		PATH_DELIM	':'
#define		DEFAULT_MODE	256
#endif

/* Standard location for rectangle coordinates */
#define XBEG 0
#define XEND 1
#define YBEG 2
#define YEND 3

/* Key codes for CRT.C */
#define KEY_EXIT        0x12D
#define KEY_HELP        0x123
#define KEY_ABORT       0x01B
#define KEY_UP          0x148
#define KEY_DN          0x150
#define KEY_RT          0x14D
#define KEY_LF          0x14B
#define KEY_RTWORD      0x174
#define KEY_LFWORD      0x173
#define KEY_BEGLINE     0x147
#define KEY_ENDLINE     0x14f
#define KEY_BEGPAGE     0x177
#define KEY_ENDPAGE     0x175
#define KEY_UPPAGE      0x149
#define KEY_DNPAGE      0x151
#define KEY_BEGFILE     0x184
#define KEY_ENDFILE     0x176
#define KEY_GOTO        0x122
#define KEY_INS         0x152
#define KEY_DEL         0x153
#define KEY_BKSP        0x008
#define KEY_DELWORD     0x193
#define KEY_BKSPWORD    0x07F
#define KEY_DELLINE     0x120
#define KEY_DELEOL      0x112
#define KEY_FINDSTR     0x121
#define KEY_NEXTSTR     0x131
#define KEY_REPLSTR     0x114
#define KEY_MARKBLK     0x125
#define KEY_CUTBLK      0x114
#define KEY_COPYBLK     0x12E
#define KEY_PASTEBLK    0x132
#define KEY_RDFILE      0x113
#define KEY_WRFILE      0x111
#define KEY_INFILE      0x117
#define KEY_OPENWIN     0x118
#define KEY_SHUTWIN     0x11F
#define KEY_JUMPWIN     0x124

/*-------------------------------------------------------------------------*/
/* DOUBLY-LINKED LIST DATA STRUCTURES                                      */

/* Generic structure for node in doubly-linked list */
typedef struct _dlnode dlnode;
struct _dlnode {
   dlnode *     next;   /* Pointer to next node in doubly-linked list,
                           NULL if tail */
   dlnode *     prev;   /* Pointer to previous node in doubly-linked list,
                           NULL if head */
   };

/* Doubly-linked list structure */
typedef struct _dllist dllist;
struct _dllist {
   dlnode *     head;   /* Pointer to head of doubly-linked list */
   dlnode *     tail;   /* Pointer to tail of doubly-linked list */
   dlnode *     curr;   /* Pointer to current position in doubly-linked list */
   unsigned     len;    /* Length of doubly-linked list */
   };

/*-------------------------------------------------------------------------*/
/* COMMAND PARSING DEFINITIONS AND MACROS                                  */

struct par_struct {
        char *          name;      /*   Parameter name.  '-' in first
                                        character indicates free format
                                        parameter.*/
        char *          defaults;  /*   Default parameter value. */
        char *          override;  /*   Override parameter value.*/
        unsigned        type;      /*    Type indicator */
        unsigned        array;     /*   Array indicator:
                                           0 -- no array
                                           1 -- arbitrary array length
                                          >1 -- required  array length*/
        char *          valptr;    /*   Pointer to storage space for results
                                        of parsing.*/
        unsigned        nbytes;    /*   Number of bytes pointed to by value.*/
        unsigned *      nunits;    /*   Pointer for returning number of units
                                        filled into an array (not returned if
                                        NULL). */
        char *          range;     /*   String that defines ranges of values */
        unsigned        version;   /*   Comparse version number.    */
        };

typedef struct par_struct PAR_STRUCT;

#define DFINTPAR                0       /* Default integer */
#define DFUNSPAR                1       /* Default unsigned integer */
#define SHINTPAR                2       /* Short integer. */
#define SHUNSPAR                3       /* Short unsigned integer. */
#define LNINTPAR                4       /* Long integer. */
#define LNUNSPAR                5       /* Long unsigned integer. */
#define FLOATPAR                6       /* Floating point. */
#define DOUBLPAR                7       /* Double floating point. */

#define CHAR_PAR                8       /* Character. */

#define STRNGPAR                9       /* Character string. */
#define BOOL_PAR               10       /* Boolean. */
#define ENUM_PAR               11       /* Enumerated type. */
#define REST_PAR               12       /* Rest of fixed format parameters */
#define REFERPAR	       13	/* Reference parameter (pointer) */
					/* can only be variables */
#define BLOCKPAR               14       /* Block of unevaluated commands */
#define FLAG_PAR               15       /* Keyword only parameter */
#define ULISTPAR               16       /* Unsigned list parameter */

#define PAR_STRUCT_SIZE( params )\
(params==NULL?0:sizeof(params)/sizeof(PAR_STRUCT))

/* define ENUM parameters */
#define PSX(name,defaults,override,type,array,valptr,nunits,consts)\
{name,defaults,override,type,array,(char*)(valptr),sizeof(valptr),nunits,consts,3}

/* normal by-value parameters */
#define PSI(name,defaults,override,type,array,valptr,nunits)\
PSX(name,defaults,override,type,array,valptr,nunits,NULL)

typedef struct {  void *base;	/* variable base address */
		  int  nels;	/* elements in the variable.  1 if scaler */
				/* > 1 if array */
		  int elsize;   /* size of an element in bytes */
		  int indx;	/* index to this entry. -1 if no index spec'd */
	       }  REFPAR;	/* reference parameter type definition */

/*-------------------------------------------------------------------------*/
/* COMMAND INTERPRETING DEFINITIONS AND MACROS                             */

struct com_struct {
        char *          name;      /* Command name. */
        int  (*         func )();  /* Function pointer. */
	char *		file;	   /* Source file for documentation */
				   /* NULL enables use of MAN-LINTARG system */ 
        unsigned        version;   /* Comterp version number.    */
        };

typedef struct com_struct COM_STRUCT;

#define COM_STRUCT_SIZE( commands )\
(commands==NULL?0:sizeof(commands)/sizeof(COM_STRUCT))

#define CSI(name,func,file)\
{name,func,file,1}

#define COMTERP(commands)\
comterp(argc, argv, commands, COM_STRUCT_SIZE(commands))

/* EXPR definitions */
struct token
  { int         tok_type;
    struct token  *next;
    union { double  value;
	    int     ft_ptr;
	    int     oper; 
            int     tlocs[2];   
	    void *pntr;		/* general pointer */
	  }
    tok_val;
    struct token *list;
  };

#define         EXPR_TYPE       struct token

/* Error handling macros */
#define TITLE_PRINT {if( TITLE ) fprintf( stderr, "%s:  ", TITLE ); }

#define breakpt() {}

#define KANRET( format_string )\
{ if(Kaput_On)\
 {TITLE_PRINT;\
  fprintf( stderr, format_string );\
  fprintf( stderr, "\n" );}\
  breakpt();}

#define KANRET1( format_string, val1 )\
{ if(Kaput_On)\
 {TITLE_PRINT;\
  fprintf( stderr, format_string, val1 );\
  fprintf( stderr, "\n" );}\
  breakpt();}

#define KANRET2( format_string, val1, val2 )\
{ if(Kaput_On)\
 {TITLE_PRINT;\
  fprintf( stderr, format_string, val1, val2 );\
  fprintf( stderr, "\n" );}\
  breakpt();}

#define KANRET3( format_string, val1, val2, val3 )\
{ if(Kaput_On)\
 {TITLE_PRINT;\
  fprintf( stderr, format_string, val1, val2, val3 );\
  fprintf( stderr, "\n" );}\
  breakpt();}

#define ERRJMP( format_string )\
{KANRET( format_string )\
goto error_return;}

#define ERRJMP1( format_string, val1 )\
{KANRET1( format_string, val1 )\
goto error_return;}

#define ERRJMP2( format_string, val1, val2 )\
{KANRET2( format_string, val1, val2 )\
goto error_return;}

#define ERRJMP3( format_string, val1, val2, val3 )\
{KANRET3( format_string, val1, val2, val3 )\
goto error_return;}

#define KAPUT( format_string )\
{KANRET( format_string )\
return -1; }

#define KAPUT1( format_string, val1 )\
{KANRET1( format_string, val1 )\
return -1; }

#define KAPUT2( format_string, val1, val2 )\
{KANRET2( format_string, val1, val2 )\
return -1; }

#define KAPUT3( format_string, val1, val2, val3 )\
{KANRET3( format_string, val1, val2, val3 )\
return -1; }

#define KANIL( format_string )\
{KANRET( format_string )\
return NULL; }

#define KANIL1( format_string, val1 )\
{KANRET1( format_string, val1 )\
return NULL; }

#define KANIL2( format_string, val1, val2 )\
{KANRET2( format_string, val1, val2 )\
return NULL; }

#define KANIL3( format_string, val1, val2, val3 )\
{KANRET3( format_string, val1, val2, val3 )\
return NULL; }

/* Macros to allocate and free memory */
#if !defined(C_terp)

#define MAKE_SPACE( pointer, type, size ) { \
   pointer = (type *) calloc( 1, size );\
   if( pointer == NULL ) { \
      KANRET( ALLOC_ERR );\
      goto error_return; \
      } \
   bl_alloc((char *) pointer, size ); \
   }

#define FREE_SPACE( pointer ) { \
   if( pointer != NULL ) {\
      bl_free((char *) pointer ); \
      free((char *) pointer ); \
      pointer = NULL;\
      } \
   }

#else

#define MAKE_SPACE( pointer, type, size ) { \
   pointer = (type *) calloc( 1, size );\
   if( pointer == NULL ) { \
      KANRET( ALLOC_ERR );\
      goto error_return; \
      }\
   }

#define FREE_SPACE( pointer ) { \
   if( pointer != NULL ) {\
      free((char *) pointer ); \
      pointer = NULL;\
      }\
   }

#endif

#define TRIMSPACE( string, newlength )\
if( string == NULL ) newlength = 0; \
else for (newlength = strlen( string );\
(isspace(*(string+newlength-1)) && newlength>0);\
newlength--);if(*(string+newlength) != '\0') *(string+newlength)='\0'

#define GETINT( string, integer, flag )\
  flag = (sscanf( string, "%hd", &integer ) == 1);\
  if (flag) while (isdigit(*string)) (string++)

/* Vowel detection */
#define isvowel( ch )\
   (ch=='a'||ch=='A'||ch=='e'||ch=='E'||ch=='i'||ch=='I'||\
   ch=='o'||ch=='O'||ch=='u'||ch=='U')

/* CURKEYS constants */
#define CUR_MOVE     1
#define CUR_RBUT     0
#define CUR_LBUT     2
#define CUR_MBUT     3
#define CUR_NONE    -1    

/* ANSI.SYS (and therefore VT-100) Escape Sequences */
#define CUH     "\033\133\110"          /* cursor home */
#define EEL     "\033\133\113"          /* erase to end-of-line */
#define CLS     "\033\133\062\112"      /* clear screen */
#define ATO     "\033\133\060\155"      /* attributes off */
#define BLD     "\033\133\061\155"      /* bold */
#define UDS     "\033\133\064\155"      /* underscore */
#define BLK     "\033\133\065\155"      /* blink */
#define RVS     "\033\133\067\155"      /* reverse image */
#define ECH     "\033\133\070\155"      /* echo off */
#define BKF     "\033\133\063\060\155"  /* black foreground */
#define RDF     "\033\133\063\061\155"  /* red foregroung */
#define GNF     "\033\133\063\062\155"  /* green foreground */
#define YWF     "\033\133\063\063\155"  /* yellow foreground */
#define BEF     "\033\133\063\064\155"  /* blue foreground */
#define MAF     "\033\133\063\065\155"  /* magnenta foreground */
#define CNF     "\033\133\063\066\155"  /* cyan foreground */
#define WEF     "\033\133\063\067\155"  /* white foreground */
#define BKB     "\033\133\064\060\155"  /* black background */
#define RDB     "\033\133\064\061\155"  /* red background */
#define GNB     "\033\133\064\062\155"  /* green background */
#define YWB     "\033\133\064\063\155"  /* yellow background */
#define BEB     "\033\133\064\064\155"  /* blue background */
#define MAB     "\033\133\064\065\155"  /* magnenta background */
#define CNB     "\033\133\064\066\155"  /* cyan background */
#define WEB     "\033\133\064\067\155"  /* white background */
#define SCU     "\033\133\163"          /* save cursor */
#define RCU     "\033\133\165"          /* restore cursor */

#ifdef MSDOS
#define         ptrtoabs(ptr)                                   \
        (((unsigned long) FP_SEG(ptr) << 4)+ (unsigned long) FP_OFF(ptr))
#endif /* MSDOS */

/* The `huge` define is needed for allocating large areas of memory */
/* in Microsoft C.  OS9 and most other machines do not need it */
/* `huge` allows blocks up to sizeof(long) instead of sizeof(int) */
#if defined(MSDOS)
/* memmove is the overlapping mem copy in Microsoft C  */
#define MEMCPY(dest,src,count)  \
		memmove((void *)(dest),(const void *)(src),(size_t)(count))
#define MEMSET(dest,c,count)    \
		memset((void *)(dest),(int)(c),(size_t)(count))
#endif

#if defined(C_terp)
/* Microsoft halloc() has some problems.  You're first alloc must be */
/* a halloc() or things will bomb out (you must be before start of heap) */
/* C_terp does not support halloc and hfree yet; Use calloc/free for test */
#define halloc(nrecs,nsize)  calloc((size_t) (nrecs),(size_t) (nsize))
#define hfree(pntr)   free((pntr))
#endif

#if !defined(MSDOS)
#if 0 /* leaved commented out if you don't need */
typedef int size_t;	/* this is some ANSI type */
#endif
#if defined(OSK)
/* this is the non-overlapping mem copy used for OS-9 */
#define MEMCPY(dest,src,count)  \
		memcpy((char *)(dest),(char *)(src),(unsigned)(count))
#else
#define MEMCPY(dest,src,count) \
                bcopy((char*)src,(char*)dest,(int)(count))
#endif
#define MEMSET(dest,c,count)    \
		memset((char *)(dest),(int)(c),(unsigned)(count))

/* huge is defined as nothing: only necessary for MS-DOS */
#define huge	
/* map the Microsoft-C huge routines over if not already defined */
#define halloc(nrecs,nsize)  calloc((unsigned)(nrecs), (unsigned)(nsize))
#define hfree(pntr)   free(pntr)
#endif  /* defined(MSDOS) */


#define COMPARSE(params,string) {\
  int status; \
  status = comparse(argc,argv,params,PAR_STRUCT_SIZE(params),string); \
  if (status==1) return FUNCOK; \
  else if(status==-1) return FUNCBAD; }

#define AF_COMPARSE(params,string) {\
  int status; \
  status = comparse(argc,argv,params,PAR_STRUCT_SIZE(params),string); \
  if (status==1) return(FUNCHELP); \
  else if(status==-1) return(FUNCBAD); }


/*---------------------------------------------------------------------------*/

/* Remove these routines if running in C-terp */
#if defined(C_terp)||!defined(MSDOS)
#define bl_alloc(p,n) 
#define bl_free(p)
#endif

/* Obsolete functions */
#define rodent(m1,m2,m3,m4)             cmousel(m1,m2,m3,m4)
#define video_state(ncols,mode,page)    crt_getmode(mode)
#define crt_mode(mode)                  crt_setmode(mode)
#define set_cursor(row,col,page)        crt_setcur(row,col)
#define cur_disable_mouse(flag)         cur_setmouse(flag)

#if 0
/* Old file pointer conventions */
#define fileerr stderr
#define fileout stdout
#endif

#ifdef MSDOS
#include <util.arg>
#endif

#ifdef OSK
EXPR_TYPE *getexpr();
EXPR_TYPE *postfix();
EXPR_TYPE *tokenize();
char *strsub();
char *findmat();
char *detab();
char *simple_path();
char *sp_gets();
char **dir();
FILE *fopen_path();
dlnode *dll_insert();
dlnode *dll_remove();
dlnode *dll_prev();
dlnode *dll_next();
dlnode *dll_head();
dlnode *dll_tail();
unsigned long TIMER_READ();
#endif

#endif /* not UTIL_INCLUDED */
