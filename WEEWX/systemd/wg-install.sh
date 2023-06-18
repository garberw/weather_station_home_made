#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cp $SCRIPT_HOME/weewx_atlas.service /etc/systemd/system/
cp $SCRIPT_HOME/weewx_wmod.service  /etc/systemd/system/
cp $SCRIPT_HOME/weewx_green.service /etc/systemd/system/
cp $SCRIPT_HOME/weewx_merge.service /etc/systemd/system/
cp $SCRIPT_HOME/weewx_timer.timer   /etc/systemd/system/

/sbin/restorecon -v /etc/systemd/system/weewx_atlas.service 
/sbin/restorecon -v /etc/systemd/system/weewx_wmod.service 
/sbin/restorecon -v /etc/systemd/system/weewx_green.service 
/sbin/restorecon -v /etc/systemd/system/weewx_merge.service 
/sbin/restorecon -v /etc/systemd/system/weewx_timer.timer 

systemctl daemon-reload

# eee eof
