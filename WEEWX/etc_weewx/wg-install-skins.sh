#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

DEFAULT="/etc/weewx/skins/"
SRC="$SCRIPT_HOME/skins/"
DST="/usr/share/weewx/user/skins/"

echo "install default skins ------------------------------------------"

rsync -av --delete "$DEFAULT" "$DST"

echo "install customization to skins ---------------------------------"

# ____NOTE____ no --delete; do an overlay;
rsync -av "$SRC" "$DST"


# eee eof
