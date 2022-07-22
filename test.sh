#!/bin/sh

./chkugenids /dev/ugen1.00 1fd3 0608

retVal=$?
if [ $retVal -ne 0 ]; then
	echo ">>> NO match"
else
	echo ">>> Match!"
fi

