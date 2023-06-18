#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

cd $SCRIPT_HOME/garberw_version

# zip -r weewx-sdr.zip weewx-sdr
tar czf weewx_wmod.tar.gz weewx_wmod/

wee_extension_wmod --install weewx_wmod.tar.gz

wee_config_wmod --reconfigure --driver=user.weewx_wmod --no-prompt

# eee eof
