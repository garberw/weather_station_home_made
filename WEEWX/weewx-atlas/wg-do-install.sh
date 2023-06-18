#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cd $SCRIPT_HOME/garberw_version

# check .zip file owned by root

# zip -r weewx_atlas.zip weewx_atlas
tar czf weewx_atlas.tar.gz weewx_atlas/

ZIPFILE=$SCRIPT_HOME/garberw_version/weewx_atlas.tar.gz

wee_extension_atlas --install "$ZIPFILE"

# it is not a driver
# wee_config_atlas --reconfigure --driver=user.weewx_atlas --no-prompt


# eee eof
