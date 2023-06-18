#!/bin/bash

cp weatherg.service  /etc/systemd/system/
chmod 644 /etc/systemd/system/weatherg.service
# for redhat
# /sbin/restorecon -v /etc/systemd/system/weatherg.service 
systemctl daemon-reload
systemctl enable --now weatherg.service
systemctl status weatherg.service

# eee eof
