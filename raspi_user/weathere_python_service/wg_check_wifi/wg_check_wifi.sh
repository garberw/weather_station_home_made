#!/bin/bash

method1() {
    sudo /sbin/shutdown -r now
}

method2() {
    echo "No network connection, restarting wlan0"
    /sbin/ifdown 'wlan0'
    sleep 5
    /sbin/ifup --force 'wlan0'
}

ping -c4 192.168.0.1 > /dev/null
 
if [ $? != 0 ] 
then
    # method1
    method2
fi


# eee eof
