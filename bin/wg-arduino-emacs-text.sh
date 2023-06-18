#!/bin/bash

if [[ "$#" -eq "0" ]]; then
    # start in dired mode
    TERM=xterm-256color emacs -nw .
else
    # $@ passes each arg as separate quoted string
    # $* passes all args in one quoted string
    TERM=xterm-256color emacs -nw $@
fi

# eee eof
