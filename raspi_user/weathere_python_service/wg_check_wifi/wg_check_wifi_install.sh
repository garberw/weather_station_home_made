#!/bin/bash

cp wg_check_wifi.sh /usr/local/bin

chown root.root /usr/local/bin/wg_check_wifi.sh

chmod 755 /usr/local/bin/wg_check_wifi.sh

# also add this line with "crontab -e"
# */5 * * * * /usr/bin/sudo -H /usr/local/bin/checkwifi.sh >> /dev/null 2>&1

# eee eof
