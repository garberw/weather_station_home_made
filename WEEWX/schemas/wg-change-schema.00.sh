#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

echo "add atlas caseTemp"
wee_database_atlas --add-column=caseTemp1 --type=REAL
wee_database_atlas --add-column=caseTemp2 --type=REAL
wee_database_atlas --add-column=caseTemp3 --type=REAL
wee_database_atlas --add-column=caseTemp4 --type=REAL
wee_database_atlas --add-column=caseTemp5 --type=REAL
wee_database_atlas --add-column=caseTemp6 --type=REAL
wee_database_atlas --add-column=caseTemp7 --type=REAL
wee_database_atlas --add-column=caseTemp8 --type=REAL

echo "add atlas caseHumid"
wee_database_atlas --add-column=caseHumid1 --type=REAL
wee_database_atlas --add-column=caseHumid2 --type=REAL
wee_database_atlas --add-column=caseHumid3 --type=REAL
wee_database_atlas --add-column=caseHumid4 --type=REAL
wee_database_atlas --add-column=caseHumid5 --type=REAL
wee_database_atlas --add-column=caseHumid6 --type=REAL
wee_database_atlas --add-column=caseHumid7 --type=REAL
wee_database_atlas --add-column=caseHumid8 --type=REAL

echo "add atlas cpuTemp"
wee_database_atlas --add-column=cpuTemp1 --type=REAL
wee_database_atlas --add-column=cpuTemp2 --type=REAL
wee_database_atlas --add-column=cpuTemp3 --type=REAL
wee_database_atlas --add-column=cpuTemp4 --type=REAL
wee_database_atlas --add-column=cpuTemp5 --type=REAL
wee_database_atlas --add-column=cpuTemp6 --type=REAL
wee_database_atlas --add-column=cpuTemp7 --type=REAL
wee_database_atlas --add-column=cpuTemp8 --type=REAL

echo "add wmod caseTemp"
wee_database_wmod --add-column=caseTemp1 --type=REAL
wee_database_wmod --add-column=caseTemp2 --type=REAL
wee_database_wmod --add-column=caseTemp3 --type=REAL
wee_database_wmod --add-column=caseTemp4 --type=REAL
wee_database_wmod --add-column=caseTemp5 --type=REAL
wee_database_wmod --add-column=caseTemp6 --type=REAL
wee_database_wmod --add-column=caseTemp7 --type=REAL
wee_database_wmod --add-column=caseTemp8 --type=REAL

echo "add wmod caseHumid"
wee_database_wmod --add-column=caseHumid1 --type=REAL
wee_database_wmod --add-column=caseHumid2 --type=REAL
wee_database_wmod --add-column=caseHumid3 --type=REAL
wee_database_wmod --add-column=caseHumid4 --type=REAL
wee_database_wmod --add-column=caseHumid5 --type=REAL
wee_database_wmod --add-column=caseHumid6 --type=REAL
wee_database_wmod --add-column=caseHumid7 --type=REAL
wee_database_wmod --add-column=caseHumid8 --type=REAL

echo "add wmod cpuTemp"
wee_database_wmod --add-column=cpuTemp1 --type=REAL
wee_database_wmod --add-column=cpuTemp2 --type=REAL
wee_database_wmod --add-column=cpuTemp3 --type=REAL
wee_database_wmod --add-column=cpuTemp4 --type=REAL
wee_database_wmod --add-column=cpuTemp5 --type=REAL
wee_database_wmod --add-column=cpuTemp6 --type=REAL
wee_database_wmod --add-column=cpuTemp7 --type=REAL
wee_database_wmod --add-column=cpuTemp8 --type=REAL

echo "add green caseTemp"
wee_database_green --add-column=caseTemp1 --type=REAL
wee_database_green --add-column=caseTemp2 --type=REAL
wee_database_green --add-column=caseTemp3 --type=REAL
wee_database_green --add-column=caseTemp4 --type=REAL
wee_database_green --add-column=caseTemp5 --type=REAL
wee_database_green --add-column=caseTemp6 --type=REAL
wee_database_green --add-column=caseTemp7 --type=REAL
wee_database_green --add-column=caseTemp8 --type=REAL

echo "add green caseHumid"
wee_database_green --add-column=caseHumid1 --type=REAL
wee_database_green --add-column=caseHumid2 --type=REAL
wee_database_green --add-column=caseHumid3 --type=REAL
wee_database_green --add-column=caseHumid4 --type=REAL
wee_database_green --add-column=caseHumid5 --type=REAL
wee_database_green --add-column=caseHumid6 --type=REAL
wee_database_green --add-column=caseHumid7 --type=REAL
wee_database_green --add-column=caseHumid8 --type=REAL

echo "add green cpuTemp"
wee_database_green --add-column=cpuTemp1 --type=REAL
wee_database_green --add-column=cpuTemp2 --type=REAL
wee_database_green --add-column=cpuTemp3 --type=REAL
wee_database_green --add-column=cpuTemp4 --type=REAL
wee_database_green --add-column=cpuTemp5 --type=REAL
wee_database_green --add-column=cpuTemp6 --type=REAL
wee_database_green --add-column=cpuTemp7 --type=REAL
wee_database_green --add-column=cpuTemp8 --type=REAL


# eee eof
