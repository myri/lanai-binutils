#!/bin/sh

################################################################
# This script is automatically invoked if the user has . in his
# path.  It removes "." from the path and reruns make with the new path.
# This is done because some of the Gnu packages in the lanai-tools
# tree to not build properly on some Linux machines if "." is
# in the user's PATH.
################################################################

echo "@@@ ./make is removing \".\" from PATH and rerunning make @@@"
PATH=`echo ${PATH} | sed -e 's@[\.:][\.:]*@:@g' | sed -e 's@^:@@g'`
echo PATH="$PATH"
make "$@"
echo "@@@ ./make removed \".\" from PATH and rerunning make @@@"
