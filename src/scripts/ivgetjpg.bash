#!/bin/bash
# ** requires w3c from w3c.org and djpeg from jpeg.org as well **
#
# ivgetjpeg
#
# bash script to download a jpeg file and import to a local drawing editor
#
# Parameters:
#     $1 URL 
#     $2 import port on drawing editor
url=$1
importport=$2
echo import $url to port $importport
tempfile=`tmpnam`
cmapfile=`tmpnam`
stdcmapppm >$cmapfile
w3c $url >$tempfile; djpeg -map $cmapfile -dither fs -pnm $tempfile | comterp telcat localhost $importport
rm $tempfile $cmapfile
