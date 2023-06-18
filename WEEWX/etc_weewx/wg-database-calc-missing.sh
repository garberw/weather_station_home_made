#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

process_station() {
    echo
    echo "THIS MAY TAKE A LONG TIME; maybe five minutes or more ..."
    echo "press 'y' when prompted"
    echo -n "enter non-blank to do wee_database_$1 --calc-missing for $1 > "
    read answer
    echo
    if [[ "x$answer" == "x" ]]; then
	return
    fi
    echo "wee_database_$1 --calc-missing"
    wee_database_$1 --calc-missing
    echo
}

process_station atlas
process_station wmod
process_station green



# eee eof
