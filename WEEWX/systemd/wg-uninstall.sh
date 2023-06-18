#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

systemctl stop weewx_atlas.service
systemctl stop weewx_wmod.service
systemctl stop weewx_green.service
systemctl stop weewx_merge.service
systemctl stop weewx_timer.timer

systemctl disable weewx_atlas.service
systemctl disable weewx_wmod.service
systemctl disable weewx_green.service
systemctl disable weewx_merge.service
systemctl disable weewx_timer.timer

rm /etc/systemd/system/weewx_atlas.service # and symlinks that might be related
rm /etc/systemd/system/weewx_wmod.service # and symlinks that might be related
rm /etc/systemd/system/weewx_green.service # and symlinks that might be related
rm /etc/systemd/system/weewx_merge.service # and symlinks that might be related
rm /etc/systemd/system/weewx_timer.timer # and symlinks that might be related

# rm /usr/lib/systemd/system/weewx_atlas.service  # and symlinks that might be related
# rm /usr/lib/systemd/system/weewx_wmod.service  # and symlinks that might be related
# rm /usr/lib/systemd/system/weewx_green.service  # and symlinks that might be related
# rm /usr/lib/systemd/system/weewx_merge.service  # and symlinks that might be related
# rm /usr/lib/systemd/system/weewx_timer.timer  # and symlinks that might be related

systemctl daemon-reload
systemctl reset-failed

## systemctl revert weatherwms.service    # does the whole thing

# for redhat; does not do anything on raspbian
# /sbin/restorecon -v /etc/systemd/system/weewx_atlas.service
# /sbin/restorecon -v /etc/systemd/system/weewx_wmod.service
# /sbin/restorecon -v /etc/systemd/system/weewx_green.service
# /sbin/restorecon -v /etc/systemd/system/weewx_merge.service
# /sbin/restorecon -v /etc/systemd/system/weewx_timer.timer

# eee eof
