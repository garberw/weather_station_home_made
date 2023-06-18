#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

DATE=`date -Is`

EXEC="/usr/share/weewx/user/sdr.py"

LOG="$SCRIPT_HOME/LOG/run5_${DATE}.log"

CMD="rtl_433 -M utc -F json"

# PYTHONPATH=$PYTHONPATH:/usr/share/weewx/ python "$EXEC" --cmd="$CMD" 2>&1 | tee "$LOG"

#HIDE=--hide=out,parsed,unparsed,mapped,unmapped,empty
HIDE=
#HIDE=--hide=out

# could omit --cmd 
PYTHONPATH=$PYTHONPATH:/usr/share/weewx/ python "$EXEC" --cmd="$CMD" $HIDE --config=/etc/weewx/weewx_atlas.conf 2>&1 | tee "$LOG"


# eee eof
