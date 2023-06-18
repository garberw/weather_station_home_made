#!/bin/bash

cp weathere.service  /etc/systemd/system/
chmod 644 /etc/systemd/system/weathere.service
# for redhat
# /sbin/restorecon -v /etc/systemd/system/weathere.service 
systemctl daemon-reload
systemctl enable --now weathere.service
systemctl status weathere.service

# eee eof
