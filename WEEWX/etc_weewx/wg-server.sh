#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

ALL=no

request_all() {
    echo -n "enter a to restart all services > "
    read ALL
}

restart_server() {
    echo
    if [[ "x$ALL" != "xa" ]]; then
	echo -n "enter non-blank to restart $1 > "
	read answer
	if [[ "x$answer" == "x" ]]; then
	    return
	fi
    fi
    echo
    echo "stop $1"
    systemctl stop "$1"
    echo
    echo "start $1"
    systemctl start "$1"
    echo
    echo "status $1"
    systemctl status "$1"
    echo
}

request_all
restart_server weewx_atlas.service
restart_server weewx_wmod.service
restart_server weewx_green.service


# eee eof
