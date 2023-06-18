#!/bin/bash


echo "will ... cd ~/Downloads"
echo -n "press enter when ready > "

read answer

set -x

cd ~/Downloads

curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh


# eee eof
