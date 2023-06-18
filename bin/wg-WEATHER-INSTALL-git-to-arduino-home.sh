#!/bin/bash

# copy
# from
#   git tree Arduino == $GIT_HOME/Arduino
# to
#   home directory Arduino == $ARDUINO_HOME    which usually equals /home/username/Arduino

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT warning this will overwrite ~/Arduino code;"
echo "$PROMPT usually that is where development is done;"
echo "$PROMPT this is for first time install"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo -n "$PROMPT type yes to proceed > "
read answer

if [[ "x$answer" != "xyes" ]]
then
   exit 0
fi

# Arduino program files (not libraries) end in .ino
function copy_similar_program_dirs() {
    ppp="$1"
    for ddd in $GIT_HOME/Arduino/${ppp}*
    do
        nnn=`basename $ddd`
        # trailing slashes
        rsync -av --delete "$ddd/" "$ARDUINO_HOME/$nnn/"
    done
}

function copy_similar_library_dirs() {
    ppp="$1"
    for ddd in $GIT_HOME/Arduino/libraries/${ppp}*
    do
        nnn=`basename $ddd`
        # trailing slashes
        rsync -av --delete "$ddd/" "$ARDUINO_HOME/libraries/$nnn/"
    done
}

copy_similar_program_dirs test
copy_similar_program_dirs utility
copy_similar_program_dirs weather_dri

copy_similar_library_dirs weather_lib
copy_similar_library_dirs weather_wra

# eee eof
