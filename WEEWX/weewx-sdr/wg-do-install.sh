#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cd $SCRIPT_HOME/garberw_version

zip -r weewx-sdr.zip weewx-sdr-master
# tar czf weewx-sdr.tar.gz weewx-sdr-master

wee_extension_atlas --install weewx-sdr.zip

wee_config_atlas --reconfigure --driver=user.sdr --no-prompt

# eee eof
