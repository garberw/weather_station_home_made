#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cp -i $SCRIPT_HOME/weewx_atlas.conf /etc/weewx/
cp -i $SCRIPT_HOME/weewx_wmod.conf /etc/weewx/
cp -i $SCRIPT_HOME/weewx_green.conf /etc/weewx/
cp -i $SCRIPT_HOME/weewx_merge.conf /etc/weewx/



# eee eof
