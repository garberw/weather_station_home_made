#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

# use wee_extension_wmod      wmod is arbitrary

cd $SCRIPT_HOME/garberw_version

tar czf weewx_schema_garberw.tar.gz weewx_schema_garberw/

wee_extension_wmod --install weewx_schema_garberw.tar.gz

# we are     adding or changing a service
# we are not adding or changing a driver
#### wee_config_wmod --reconfigure --driver=user.weewx_schema_garberw --no-prompt



# eee eof
