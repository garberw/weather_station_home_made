#!/usr/bin/env python3

import time
import raspi_temp
import numpy

def main():
    ttt = raspi_temp.raspi_temp()
    rrr = ttt.rts_pin
    sss = ttt.serial
    ddd = bytearray([0x02, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x38])
    count = 0
    empty = 0
    while True:
        rrr.transmit_set()
        sss.write(ddd)
        sss.flush()
        ttt.delayMicroseconds(ttt.T35_usec)
        ttt.delayMilliseconds(50)
        rrr.receive_set()
        mode = 1
        reply = numpy.zeros(20, dtype=int)
        jjj = 0
        if mode == 1:
            reply = sss.read(9)
            jjj = len(reply)
        elif mode == 2:
            while sss.in_waiting != 0:
                bbb = sss.read()
                B = bbb[0]
                reply[jjj] = B
                jjj += 1
                ttt.delayMicroseconds(ttt.T15_usec)
        reply = reply[:jjj]
        if jjj == 0:
            count = count + 1
        elif jjj == 1 and reply[0] == 0:
            empty = empty + 1
        else:
            print("len(reply) = {} blanks = {} empty = {}".format(len(reply), count, empty))
            print("reply = {}".format(reply))
            count = 0
            empty = 0


if __name__ == '__main__':
    main()

# eee eof
