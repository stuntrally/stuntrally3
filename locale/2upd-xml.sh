#!/bin/sh

echo --- Converting .po to .xml
echo

./xml_po_all.py


echo "Press Enter to end ..."
read REPLY
#sleep 5
echo End.
