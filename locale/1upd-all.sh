#!/bin/sh

echo --- Generating new template .pot
echo

../bin/RelWithDebInfo/sr-translator
#./sr-translator

#echo  --  OLD  todo  --
#echo --- Uploading new template

#./tx push -s

#echo
#echo --- Pulling .po translations back

#./tx pull -a --minimum-perc=10

#echo
#echo --- Converting .po to .xml

#./xml_po_all.py

#echo
echo "Press Enter to end ..."
read REPLY
#sleep 5
echo End.
