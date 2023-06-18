#!/bin/bash

cp weathere_timer.timer   /etc/systemd/system/
chmod 644 /etc/systemd/system/weathere_timer.timer
/sbin/restorecon -v /etc/systemd/system/weathere_timer.timer 
systemctl daemon-reload
systemctl enable --now weathere_timer.timer
systemctl status weathere_timer.timer

# eee eof
