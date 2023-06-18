#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

ALL=no

request_all() {
    echo -n "enter a to get status of all services > "
    read ALL
}

process_server() {
    echo
    if [[ "x$ALL" != "xa" ]]; then
	echo -n "enter non-blank to get status of $1 > "
	read answer
	if [[ "x$answer" == "x" ]]; then
	    return
	fi
    fi
#    echo
#    echo "stop $1"
#    systemctl stop "$1"
#    echo
#    echo "start $1"
#    systemctl start "$1"
    echo
    echo "status $1"
    systemctl status "$1"
    echo
}

request_all
process_server weewx_atlas.service
process_server weewx_wmod.service
process_server weewx_green.service
process_server weewx_timer.timer

# eee eof
