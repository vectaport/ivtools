#!/bin/bash
# ** requires plotmtv for bar plotting **
# ** (see http://www.vectaport.com/ivtools/comdraw.html for details) **
#
# cntsrcheng referrers_file
#
# bash script to count search engines URL's in a file of web site referrers,
# and plot them using the barplot command on comdraw
#
# Parameters:
#     $1 file of referring URL's
#
#!/bin/bash
echo barplot\(>/tmp/cntsrcheng.TMPFILE
echo \"Yahoo\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*yahoo" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Altavista\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*altavista" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"WebCrawler\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*webcrawler" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Excite\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*excite" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"HotBot\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*hotbot" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Infoseek\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*infoseek" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"MetaCrawler\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*metacrawler" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"DejaNews\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*dejanews" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Sal\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*sal" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"LookSmart\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*looksmart" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"NetFind\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*netfind" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Inference\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*netfind" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Lycos\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*lycos" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"DogPile\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*dogpile" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"McKinley\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*mckinley" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \"Snap\">>/tmp/cntsrcheng.TMPFILE
egrep "http://.*snap" $1 | wc -l>>/tmp/cntsrcheng.TMPFILE
echo \)>>/tmp/cntsrcheng.TMPFILE
echo quit>>/tmp/cntsrcheng.TMPFILE
comdraw < /tmp/cntsrcheng.TMPFILE 
rm /tmp/cntsrcheng.TMPFILE