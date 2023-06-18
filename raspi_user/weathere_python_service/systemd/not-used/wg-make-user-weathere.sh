#!/bin/bash

adduser weathere --shell /sbin/nologin --no-create-home --disabled-password

usermod -a -G dialout weathere

grep weathere /etc/passwd



# eee eof
