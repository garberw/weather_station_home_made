#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

ALL=no

request_all() {
    echo -n "enter a to do all reports > "
    read ALL
}

request_report() {
    echo
    if [[ "x$ALL" != "xa" ]]; then
	echo -n "enter non-blank to do reports for $1 > "
	read answer
	echo
	if [[ "x$answer" == "x" ]]; then
	    return
	fi
    fi
    echo "removing /var/www/html/weewx/weewx_$1/"'*'
    rm -rf /var/www/html/weewx/weewx_$1/*
    echo
    echo "wee_reports_$1"
    wee_reports_$1
    echo
}

request_all
request_report atlas
request_report wmod
request_report green
request_report merge


# eee eof
