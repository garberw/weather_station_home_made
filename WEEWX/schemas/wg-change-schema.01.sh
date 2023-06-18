#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

echo "add atlas gas_resistance"
wee_database_atlas --add-column=gas_resistance --type=REAL

echo "add wmod gas_resistance"
wee_database_wmod --add-column=gas_resistance --type=REAL

echo "add green gas_resistance"
wee_database_green --add-column=gas_resistance --type=REAL

echo "add atlas iaq"
wee_database_atlas --add-column=iaq --type=REAL

echo "add wmod iaq"
wee_database_wmod --add-column=iaq --type=REAL

echo "add green iaq"
wee_database_green --add-column=iaq --type=REAL


# eee eof
