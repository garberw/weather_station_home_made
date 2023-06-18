#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

DATE=`date -Is`

# -R 40 == AcuriteAtlas
# -R 78 == Fine Offset WH32B

DEVICE="-R 40"

LOG="$SCRIPT_HOME/LOG/run4_${DATE}.log"

/usr/bin/rtl_433 $DEVICE 2>&1 | tee "$LOG"

# eee eof
