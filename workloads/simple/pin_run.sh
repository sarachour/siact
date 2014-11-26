#!/bin/bash

MYPIN_PATH="../../tool/pin-2.12-58423-gcc.4.4.7-linux/source/tools"
PIN_ARGS=" -d1c 16 -d1b 32 -d1a 16 -axc_c 1 -axc_b 32 -axc_a 16 -u2c 512 -u2b 32 -u2a 16 -axc axc.config"
PROB_ERROR=0.0

pin -injection child -t $MYPIN_PATH/AxC-PINTOOL/obj-intel64/axcAnyl.so  $PIN_ARGS -p $PROB_ERROR -- $@

#pin -injection child -t $MYPIN_PATH/InsMix/obj-intel64/insmix.so  -o simple.insmix -- $@

