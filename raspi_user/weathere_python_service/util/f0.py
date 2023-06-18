#!/usr/bin/env python3

import time
import raspi_temp

def main():
    rrr = raspi_temp.raspi_temp()
    sss = rrr.serial
    FFF = True
    ZZZ = True
    zzz = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    fff = bytearray([0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff])
    if FFF:
        print(fff)
        for w in range(0, 750):
            sss.write(fff)
            sss.flush()
            # print("sleep 5")
            # time.sleep(5)
    if ZZZ:
        print(zzz)
        for w in range(0, 750):
            sss.write(zzz)
            sss.flush()
            # print("sleep 5")
            # time.sleep(5)


if __name__ == '__main__':
    main()

# eee eof
