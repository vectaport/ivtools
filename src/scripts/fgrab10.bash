#!/bin/bash
# ** requires ivtools flipbook built with ACE ** 
# ** requires w3c from w3c.org and djpeg from jpeg.org as well **
# ** launch flipbook prior to running this script **
#
# fgrab10  URL_prefix URL_suffix
#
# bash script to download 10 sequential jpeg URL's, connecting to 
# the ivtools flipbook editor and importing them in series to newly
# created frames.   Use "Show Prev Frame" under Frame menu on flipbook to 
# see the previous raster while downloading the current raster.
#
# The default flipbook control port (20002) can be changed below if necessary
# and then overriden on flipbook with a -comdraw argument. 
#
# The default flipbook import port (20003) can be changed below if necessary
# and then overriden on flipbook with a -import argument. 
#
# Parameters:
#     $1 URL prefix
#     $2 URL suffix (i.e. .jpg or .JPG)
urlprefix=$1
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"0$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"1$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"2$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"3$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"4$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"5$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"6$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"7$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"8$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003
echo createframe | comterp client localhost 20002 
w3c "$urlprefix"9$2 | djpeg -map ~/cmap.ppm -dither fs -pnm | comterp telcat localhost 20003

