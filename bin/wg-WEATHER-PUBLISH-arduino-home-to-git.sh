#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "$PROMPT warning this will push $ARDUINO_HOME to overwrite $GIT_HOME/Arduino"
echo "$PROMPT this is usually what you want"
echo "$PROMPT since development is done in $ARDUINO_HOME (usually ~/Arduino)"
echo "$PROMPT and pushed to git"
echo "$PROMPT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo -n "$PROMPT type yes to proceed > "
read answer

if [[ "x$answer" != "xyes" ]]
then
   exit 0
fi

mkdir -p $GIT_HOME/Arduino/libraries

# Arduino program files (not libraries) end in .ino
function copy_similar_program_dirs() {
    ppp="$1"
    for ddd in $ARDUINO_HOME/${ppp}*
    do
        nnn=`basename $ddd`
        # trailing slashes
        rsync -av --delete "$ARDUINO_HOME/$nnn/" "$GIT_HOME/Arduino/$nnn/"
    done
}

function copy_similar_library_dirs() {
    ppp="$1"
    for ddd in $ARDUINO_HOME/libraries/${ppp}*
    do
        nnn=`basename $ddd`
        # trailing slashes
        rsync -av --delete "$ARDUINO_HOME/libraries/$nnn/" "$GIT_HOME/Arduino/libraries/$nnn/"
    done
}

cp $ARDUINO_HOME/LICENSE_WSHM* $GIT_HOME/Arduino/
cp $ARDUINO_HOME/README* $GIT_HOME/Arduino/

copy_similar_program_dirs test
copy_similar_program_dirs utility
copy_similar_program_dirs weather_dri

copy_similar_library_dirs weather_lib
copy_similar_library_dirs weather_wra
   
# eee eof
