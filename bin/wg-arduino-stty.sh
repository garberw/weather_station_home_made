#!/bin/bash

# BAUD=9600
BAUD=115200

SSSLIST="c l r w a g"

echo
/bin/ls -l /dev/ttyACM*
echo
for DDD in $SSSLIST
do
    SSS=/dev/ttyACMw$DDD
    sss="$(/usr/bin/readlink -f $SSS)"
    if [ -e "$sss" ]
    then
        echo "SSS=$SSS resolved to $sss"
        echo "stty -F $SSS $BAUD speed raw"
        stty -F $SSS $BAUD speed raw
    else
        echo "SSS=$SSS not resolved"
    fi
done

#SSSc=/dev/ttyACMwc
#SSSl=/dev/ttyACMwl
#SSSr=/dev/ttyACMwr
#SSSw=/dev/ttyACMww
#SSSa=/dev/ttyACMwa
#SSSg=/dev/ttyACMwg

#sssc=$(/usr/bin/readlink -f $SSSc)
#sssl=$(/usr/bin/readlink -f $SSSl)
#sssr=$(/usr/bin/readlink -f $SSSr)
#sssw=$(/usr/bin/readlink -f $SSSw)
#sssa=$(/usr/bin/readlink -f $SSSa)
#sssg=$(/usr/bin/readlink -f $SSSg)

#echo
#echo "diagnostics ..."
#echo "SSSc = $SSSc   sssc = $sssc"
#echo "SSSl = $SSSl   sssl = $sssl"
#echo "SSSr = $SSSr   sssr = $sssr"
#echo "SSSw = $SSSw   sssw = $sssw"
#echo "SSSa = $SSSa   sssa = $sssa"
#echo "SSSg = $SSSg   sssg = $sssg"

#echo "stty -F /dev/ttyACM0 $BAUD speed raw"
#stty -F /dev/ttyACM0 $BAUD speed raw
#echo "stty -F /dev/ttyACM1 $BAUD speed raw"
#stty -F /dev/ttyACM1 $BAUD speed raw
#echo "stty -F /dev/ttyACM2 $BAUD speed raw"
#stty -F /dev/ttyACM2 $BAUD speed raw
#echo "stty -F /dev/ttyACM3 $BAUD speed raw"
#stty -F /dev/ttyACM3 $BAUD speed raw
#echo "stty -F /dev/ttyACM4 $BAUD speed raw"
#stty -F /dev/ttyACM4 $BAUD speed raw
#echo "stty -F /dev/ttyACM5 $BAUD speed raw"
#stty -F /dev/ttyACM5 $BAUD speed raw


## stty -F /dev/ttyACMwc cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

#if [[ $# -ne 1 ]]; then
#    echo "error:  takes one arg 'compile' or 'raw'"
#    exit 1
#elif [[ "x$1" == "xcompile" ]]; then
#    echo "setting compile mode (and upload)"
#    stty -F /dev/ttyACMwc icrnl ixon opost isig icanon
#    stty -F /dev/ttyACMwl icrnl ixon opost isig icanon
#    stty -F /dev/ttyACMwr icrnl ixon opost isig icanon
#    stty -F /dev/ttyACMww icrnl ixon opost isig icanon
#    stty -F /dev/ttyACMwa icrnl ixon opost isig icanon
#    stty -F /dev/ttyACMwg icrnl ixon opost isig icanon
#elif [[ "x$1" == "xraw" ]]; then
#    echo "setting raw mode (and see output)"
#    # or simply stty -F /dev/ttyACMw? raw
#    stty -F /dev/ttyACMwc -icrnl -ixon -opost -isig -icanon
#    stty -F /dev/ttyACMwl -icrnl -ixon -opost -isig -icanon
#    stty -F /dev/ttyACMwr -icrnl -ixon -opost -isig -icanon
#    stty -F /dev/ttyACMww -icrnl -ixon -opost -isig -icanon
#    stty -F /dev/ttyACMwa -icrnl -ixon -opost -isig -icanon
#    stty -F /dev/ttyACMwg -icrnl -ixon -opost -isig -icanon
#else
#    echo "error:  unrecognized option"
#    exit 1
#fi

# eee eof
