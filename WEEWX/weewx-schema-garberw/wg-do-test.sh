#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

echo "running for atlas ==========================================="
PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_schema_garberw.py --config=/etc/weewx/weewx_atlas.conf
res=$?
echo "res=$res"

echo "running for wmod ==========================================="
PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_schema_garberw.py --config=/etc/weewx/weewx_wmod.conf
res=$?
echo "res=$res"

echo "running for green ==========================================="
PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_schema_garberw.py --config=/etc/weewx/weewx_green.conf
res=$?
echo "res=$res"

echo "running for merge ==========================================="
PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_schema_garberw.py --config=/etc/weewx/weewx_merge.conf
res=$?
echo "res=$res"

# eee eof
