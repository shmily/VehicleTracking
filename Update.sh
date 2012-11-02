#!/bin/sh   
F="xxxx.ftp"   
echo "open 192.168.2.149"    	 > $F   
echo "user plg plg"     		>> $F   
echo "bin"                      >> $F   
echo "cd /home/plg/"            >> $F   
echo "mput $1"                  >> $F   
echo "bye"                      >> $F   
ftp -i -in < $F   
rm -rf $F   
