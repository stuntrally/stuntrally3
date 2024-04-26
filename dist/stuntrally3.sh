#!/bin/sh

# Stunt Rally 3  Linux launch script

# Function to find the folder this script resides in
FindPath()
{
	# Absolute path to this script, e.g. /home/user/bin/foo.sh
	SCRIPT=`readlink -f "$0"`
	# Absolute path this script is in, thus /home/user/bin
	SCRIPTPATH=`dirname "$SCRIPT"`
	echo $SCRIPTPATH
}

LIBPATH="`FindPath $0`/lib"
BINPATH="`FindPath $0`/bin/Release"

export LD_LIBRARY_PATH="$LIBPATH"
#export OGRE_PLUGIN_DIR="$LIBPATH"

"$BINPATH/stuntrally3" $*
