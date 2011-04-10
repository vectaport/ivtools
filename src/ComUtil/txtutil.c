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
txtutil.c        TeXT storage UTILities

Externals:       unsigned int txtstore()
                 unsigned int txtread()
                 unsigned int txtopenclose()
                 unsigned int txtkwsrch()
                 unsigned int txtprint()

Summary:         Routines to manage the text storage table.

Author:          Rob Graber   7-89
*/

#include <stdio.h>
#include <stdlib.h>

#include "comutil.ci"

#define TXT_STORE_PATH "$comtxt$.$$$"

/* Local globals */
static int Currid = -1;
static int Lastid = -1;
static FILE *fd   = NULL;
static char tmpstr[256];


/*!

txtstore          STORE TeXT in text table


Summary:

*/

unsigned int txtstore(int new_entry,char * txtstr)


/*!
Return Value:  -1 = Error, # = Text ID


Parameters:

Type      Name               IO    Description
----      ----               --    -----------                 */
#ifdef DOC
int       new_entry     ;/*  I     New entry flag.
                                   1 = Start new entry
                                   0 = Continue previous entry */
char *    txtstr        ;/*  I     String to write.            */
#endif


/*!
Description:

`txtstore` stores a text string in the text table and returns an
ID for it.  If the `new_entry` flag is set a new ID is created and
returned, otherwise `txtstr` is appended to the previous entry.

!*/

#undef     TITLE
#define    TITLE  "txtstore"

{
    int index;

    /* Check for file open */
    if (fd == NULL)
    {
        /* Text storage file not open */
        COMERR_SET(ERR_TXTUT_FNOTOPEN);
        return(-1);
    }

    /* Check for string beginning with internal delimeter */
    if ( !strncmp(txtstr, ".!#ID#",6))
    {
        /* String begins with internal delimeter string */
        COMERR_SET(ERR_TXTUT_DELIM);
        return(-1);
    }

    /* Check for new entry */
    if (new_entry)
    {
        Lastid++;
        fprintf(fd,".!#ID#%d\n",Lastid);
    }

    /* Check for multiple \n's */
    index = strlen(txtstr) - 1;
    if (txtstr[index] == '\n') txtstr[index] = '\0';

    /* Write text string */
    fprintf(fd,"%s\n",txtstr);

    /* Return */
    return(Lastid);
}

/*!

txtread          READ TeXT from text table


Summary:

*/

unsigned int txtread( id, txtstr )


/*!
Return Value:  0 = OK, -1 = ERROR


Parameters:

Type       Name               IO    Description
----       ----               --    -----------*/
unsigned   id            ;/*  I     Text ID           */
char *     txtstr        ;/*  I     String to write.  */


/*!
Description:
`txtread` reads a text string from the text table.
If `id` is the same as the previous `id` no seek is
done and the next line from the file is returned (unless it
is the end of the text segment).  A -1 value for ID will cause
the current file position to be reset.

!*/

#undef     TITLE
#define    TITLE  "txtread"

{
    char *tst, cmpstr[15];

    /* Check for file open */
    if (fd == NULL)
    {
        /* Text storage file not open */
        COMERR_SET(ERR_TXTUT_FNOTOPEN);
        return(-1);
    }

    /* Check for file reset */
    if (id == -1)
    {
        Currid = -1;
        return(0);
    }

    /* Check for new id */
    if (id != Currid)
    {
        /* Reset current id pointer and seek to beginning of file */
        Currid = id;
        fseek(fd,0L,SEEK_SET);

        /* Search for .!#ID#id delimeter */
        sprintf( cmpstr, ".!#ID#%d\n",id);
        while( (tst = fgets(txtstr, 256, fd)) != NULL )
            if (!strcmp(txtstr, cmpstr)) break;

        /* Check for id not found */
        if (tst == NULL)
        {
            Currid = -1;
            return(-1);
        }
    }

    /* Read next line of file */
    tst = fgets(txtstr, 256, fd);

    /* Check for end of text entry (or EOF) */
    if ( (!strncmp(txtstr, ".!#ID#",6)) || (tst == NULL) )
    {
        Currid = -1;
        txtstr[0] = '\0';
        return(-1);
    }

    /* Return */
    return(0);
}


/*!

txtopenclose          TeXT table file OPEN and CLOSE


Summary:

*/

unsigned int txtopenclose(int openclose)


/*!
Return Value:  0 = OK, -1 = ERROR


Parameters:

Type      Name               IO    Description
----      ----               --    -----------               */
#ifdef DOC
int       openclose     ;/*  I     Txt file open close flag.
                                   0 = open
                                   1 = close                 */
#endif


/*!
Description:

`txtopenclose` opens or closes the txt file based on the `openclose` flag.

!*/

#undef     TITLE
#define    TITLE  "txtopenclose"

{
    /* Check for open/close */
    if (openclose == 0)
    {
        /* If file is not already open - open it */
        if (fd == NULL)
        {
            fd = fopen(TXT_STORE_PATH,"w+");
            if (fd == NULL)
            {
                /* Error opening text storage file */
                COMERR_SET(ERR_TXTUT_FOPEN);
                return(-1);
            }
        }
    }
    else
        /* Close file and remove */
        if (fd != NULL) 
        {
            fclose(fd);
            unlink(TXT_STORE_PATH);
        }

   return(0);
}



/*!

txtkwsrch          TeXT Key Word SeaRCH


Summary:

*/

unsigned int txtkwsrch(char * keyword,int bol,char * rdstr)


/*!
Return Value:  -1 = ERROR, # = text ID


Parameters:

Type      Name             IO    Description
----      ----             --    -----------                         */
#ifdef DOC
char *    keyword     ;/*  I     String to search table for          */
int       bol         ;/*  I     Search flag
                                    0 = whole line 
                                    1 = beginning only               */
char *    rdstr       ;/*  IO    String to return value in. 
                                 Must be alloced in calling program.
                                 If rdstr is NULL an internal string
                                 is used.                            */
#endif

/*!
Description:

`txtkwsrch` searches the text table and returns the first next ID
containing `keyword`. Use a NULL keyword to reset the file.

NOTE: The file position can be changed by reading or writing using txtstore
and txtread.  The file needs to be opened using txtopenclose prior to calling
kwsearch.

!*/

#undef     TITLE
#define    TITLE  "txtkwsrch"

{
    char *tst, *rdptr;
    int id, index;

    /* Check for file open */
    if (fd == NULL)
    {
        /* Text storage file not open */
        COMERR_SET(ERR_TXTUT_FNOTOPEN);
        return(-1);
    }

    /* Check for file reset */
    if (keyword == NULL)
    {
        fseek(fd,0L,SEEK_SET);
        return(0);
    }

    /* Set which read buffer to use */
    if (rdstr == NULL)
       rdptr = tmpstr;
    else
       rdptr = rdstr;

    id = -1;
    while( (tst = fgets(rdptr, 256, fd)) != NULL )
    {

        /* Check for .!#ID#id delimeter */
        if (!strncmp(rdptr, ".!#ID#",6)) 
            id = atoi(&rdptr[6]);
        else
        {
            if (bol)
            {
                /* One check only */  
                if (!strncmp(rdptr, keyword, strlen(keyword)))
                if (id != -1) return(id);
            }
            else
            {
                /* Search line for string */
                for (index = 0; index < strlen(rdptr); index++)
                    if (!strncmp(&rdptr[index], keyword, strlen(keyword)))
                        if (id != -1) return(id);
            }
        }
    }
    return(-1);
}

#if 0
/*!

txtprint          TeXT PRINT


Summary:

*/

unsigned int txtprint(unsigned id,char * ignorestr,unsigned pause,
		      unsigned * nlines)


/*!
Return Value:  -1 if not found, 0 = OK


Parameters:

Type      Name           IO    Description
----      ----           --    -----------                        */
#ifdef DOC
unsigned  id        ;/*  I     ID to print                        */
char *    ignorestr ;/*  I     Strings beginning with ignorestr 
                               are not printed                    */
unsigned  pause     ;/*  I     Pause afeter 22 lines flag. 
                               0 = no pause                       */
unsigned  *nlines   ;/*  I/0   Number of lines printed prior to
                               this call of txtrpint. Updated on
                               output                             */
#endif

/*!
Description:

`txtprint` searchs the text table and prints out entry `id`.
Any string beginning with `ignorestr` are NOT printed.  This is
useful for user defined delimeters within a block of text, specifically
the .!#KW# identifiers for COMTERP.
!*/

#undef     TITLE
#define    TITLE  "txtprint"

{
    int i, xsize, ysize;

    /* Illegal ID value */
    if (id == -1) return(-1);

    /* Constants */
    crt_size(&xsize, &ysize);

    /* Find first line */
    txtread(-1,tmpstr);
    if (txtread(id, tmpstr) == -1) return(-1);
    txtread(-1,tmpstr);

    /* Print until end of id */
    for (i = *nlines; txtread(id, tmpstr) != -1; i++)
    {
        if (strncmp(ignorestr, tmpstr, strlen(ignorestr)))
        {
            printf("%s",tmpstr);
            *nlines = *nlines+1;
            if (pause)
                if ( (i%ysize == 0) && (i != 0) )
                    key_to_continue();
        }
    }
    return(0);
}

#endif
