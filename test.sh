#!/bin/sh

./chkugenids -f /dev/ugen1.00 -i -q | grep "1fd3:0608" > /dev/null
retVal=$?
if [ $retVal -ne 0 ]; then
	echo ">>> NO match"
else
	echo ">>> Match!"
fi

./chkugenids -f /dev/ugen1.00 -l -q | cut -d ":" -f 4 | grep -i "logo" > /dev/null
if [ $? -eq 0 ]; then
	echo ">>> Match!"
else
	echo ">>> NO match"
fi

