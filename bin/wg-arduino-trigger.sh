#!/bin/bash

set +x

udevadm control --reload-rules
echo "res=$?"

udevadm trigger
echo "res=$?"

# eee eof
