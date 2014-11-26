#!/bin/bash

MYPIN_PATH="../../tool/pin-2.12-58423-gcc.4.4.7-linux/source/tools/AxC-PINTOOL"

g++ *.c -o simple
cd $MYPIN_PATH; make clean; make
