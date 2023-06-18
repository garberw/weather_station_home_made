#!/bin/bash

systemctl stop weathere.service
systemctl disable weathere.service
rm /etc/systemd/system/weathere.service
# rm /etc/systemd/system/weathere.service # and symlinks that might be related
# rm /usr/lib/systemd/system/weathere.service 
## rm /usr/lib/systemd/system/weathere.service # and symlinks that might be related
systemctl daemon-reload
systemctl reset-failed

## systemctl revert weathere.service    # does the whole thing

# does not do anything on raspbian
# /sbin/restorecon -v /etc/systemd/system/weathere.service 

# eee eof
