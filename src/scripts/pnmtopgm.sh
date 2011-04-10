#!/bin/sh
#
# pnmtopgm - derived from anytopnm
#
# Copyright 1998 Vectaport Inc.
# Copyright (C) 1991 by Jef Poskanzer.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.

if [ ! $# = 1 ] ; then
    echo "usage: $0 <file>" 1>&2
    exit 1
fi

origfile="$1"
file="$origfile"
tmpfiles=""

    filetype=`file "$file"`

    case "$filetype" in

	*PBM* )
	pbmtopgm "$file"
	exit 0
	;;

	*PGM* )
	cat "$file"
	exit 0
	;;

	 *PPM* )
	ppmtopgm "$file"
	exit 0
	;;

    esac

exit -1
