#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

echo
echo "set debug = 1 and log_lines = 1 in /etc/weewx/weewx_atlas.conf"
echo
echo -n "press enter to proceed > "
read answer


DATE0=`date -Is`

LOG="$SCRIPT_HOME/LOG/run5_${DATE0}.log"

SERVICE="weewx_atlas.service"

DATE1=`date +'%F %T'`

function cleanup() {
    echo
    systemctl stop $SERVICE
    echo
    systemctl status $SERVICE
    echo
    exit 0
}

trap 'cleanup' INT

systemctl start "$SERVICE"

journalctl -b -u "$SERVICE" --since="$DATE1" -f | tee "$LOG"


# eee eof
