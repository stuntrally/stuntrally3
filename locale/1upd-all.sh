#!/bin/sh

echo --- Generating new template .pot
echo

../bin/Release/sr-translator
#./sr-translator


echo "Press Enter to end ..."
read REPLY
#sleep 5
echo End.
