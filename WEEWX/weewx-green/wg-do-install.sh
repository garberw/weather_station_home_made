#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cd $SCRIPT_HOME/garberw_version

# zip -r weewx-sdr.zip weewx-sdr
tar czf weewx_green.tar.gz weewx_green/

wee_extension_green --install weewx_green.tar.gz

wee_config_green --reconfigure --driver=user.weewx_green --no-prompt

# eee eof
