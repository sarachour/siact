#!/bin/bash


PTH=tool/pin-2.12-58423-gcc.4.4.7-linux

echo "are you running a 64 bit machine? (y/n)"

read CHOICE


if [ "$CHOICE" = "y" ]
then
	echo "64 bit"
	cd $PTH
	rm pin
	ln -s intel64/bin/pinbin pin
else
	echo "32 bit"
	cd $PTH
	rm pin
	ln -s ia32/bin/pinbin pin
fi
