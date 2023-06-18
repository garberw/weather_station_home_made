#!/usr/bin/env python
"""
Copyright 2014-2020 Matthew Wall
Copyright 2014 Nate Bargmann <n0nb@n0nb.us>
See the file LICENSE.txt for your rights.

Credit to and contributions from:
  Jay Nugent (WB8TKL) and KRK6 for weather-2.kr6k-V2.1
    http://server1.nuge.com/~weather/
  Steve (sesykes71) for testing the first implementations of this driver
  Garret Power for improved decoding and proper handling of negative values
  Chris Thompstone for testing the fast-read implementation

Driver for WeatherModbus weather station

This driver assumes the WeatherModbus is emitting data in Data Logger
mode format.

Communicate over ttyUSB serial port

Port parameters are 115200, 8N1, with no flow control.
"""

from __future__ import with_statement
from __future__ import absolute_import
from __future__ import print_function

import signal
import sys
import time
import argparse
import logging
import logging.handlers

import json
import serial

import weewx
import weewx.drivers
import weewx.wxformulas

DEFAULT_DEBUG = 1
DRIVER_NAME = 'WeatherModbus'
DRIVER_VERSION = '0.0'

log = logging.getLogger(__name__)

def loader(config_dict, _):
    """loader"""
    return WeatherModbusDriver(**config_dict[DRIVER_NAME])


def confeditor_loader():
    """confeditor_loader"""
    return WeatherModbusConfEditor()


def _fmt(hex_str):
    """formatted hex string"""
    return ' '.join([f'{ch:0.2X}' for ch in hex_str])


class WeatherModbusDriver(weewx.drivers.AbstractDevice):
    """weewx driver that communicates with a WeatherModbus station"""

    def __init__(self, **stn_dict):
        """
        Args:
        port(str): Serial port.
            Required. Default is '/dev/ttyACMww'

        max_tries(int): How often to retry serial communication before giving up
            Optional. Default is 5

        retry_wait (float): How long to wait before retrying an I/O operation.
            Optional. Default is 3.0

        debug_serial (int): Greater than one for additional debug information.
            Optional. Default is 0
        """
        self.model = 'WeatherModbus'
        self.port = stn_dict.get('port', Station.DEFAULT_PORT)
        self.max_tries = int(stn_dict.get('max_tries', 20))
        self.retry_wait = float(stn_dict.get('retry_wait', 3.0))
        debug_serial = int(stn_dict.get('debug_serial', 0))
        self.last_rain = None

        log.info('Driver version is %s', DRIVER_VERSION)
        log.info('Using serial port %s', self.port)
        self.station = Station(self.port, debug_serial=debug_serial)
        self.station.open()

    def closePort(self):
        """close port"""
        if self.station:
            self.station.close()
            self.station = None

    @property
    def hardware_name(self):
        """hardware_name == self.model"""
        return self.model

    # def getTime(self):
    #     """this station does not have a get_time function"""
    #     return self.station.get_time()

    # def setTime(self):
    #     """this station does not have a set_time function"""
    #     self.station.set_time(int(time.time()))

    def genLoopPackets(self):
        """loop forever yielding packets; augment packet by computing rain"""
        # tofix what is this
        # self.station.set_logger_mode()
        while True:
            packet = {'dateTime': int(time.time() + 0.5), 'usUnits': weewx.US}
            readings = self.station.get_readings_with_retry(self.max_tries, self.retry_wait)
            data = parse_readings(readings)
            packet.update(data)
            self._augment_packet(packet)
            yield packet

    def _augment_packet(self, packet):
        """compute rain from rain_total"""
        packet['rain'] = weewx.wxformulas.calculate_rain(packet['rain_total'], self.last_rain)
        self.last_rain = packet['rain_total']


class Station:
    """weewx_wmod station"""
    DEFAULT_PORT = '/dev/ttyACMww'

    def __init__(self, port, debug_serial=0):
        """set basic options for port etc"""
        self.port = port
        self._debug_serial = debug_serial
        self.baudrate = 115200
        self.timeout = 3  # seconds
        self.serial_port = None
        # tofix remove this
#        self.can_set_year = True
#        self.has_modem_mode = True

    def __enter__(self):
        """open serial port"""
        self.open()
        return self

    def __exit__(self, _, value, traceback):
        """close serial port"""
        self.close()

    def open(self):
        """open serial port"""
        log.info('Open serial port %s', self.port)
        self.serial_port = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
        self.serial_port.flushInput()

    def close(self):
        """close serial port"""
        if self.serial_port:
            log.info('Close serial port %s', self.port)
            self.serial_port.close()
            self.serial_port = None

#    def set_logger_mode(self):
#        # in logger mode, station sends logger mode records continuously
#        if self._debug_serial:
#            log.info('Set station to logger mode')
#        self.serial_port.write(b'>I\r')

#    def set_modem_mode(self):
#        # setting to modem mode should stop data logger output
#        if self.has_modem_mode:
#            if self._debug_serial:
#                log.info('Set station to modem mode')
#            self.serial_port.write(b'>\r')

    def get_readings_with_retry(self, max_tries, retry_wait):
        """loop get_readings; sleep; for max_tries; raise weewx.RetriesExceeded on fail"""
        for ntries in range(max_tries):
            try:
                line = get_readings(self.serial_port, self._debug_serial)
                return line
            except (serial.SerialException, weewx.WeeWxIOError) as exc:
                log.info('Failed attempt %d of %d to get readings: %s',
                         ntries + 1, max_tries, str(exc))
                time.sleep(retry_wait)
        msg = f'Max retries ({max_tries}) exceeded for readings'
        log.error(msg)
        raise weewx.RetriesExceeded(msg)


# ##############################################################33
#                 Utilities
# ##############################################################33

def get_readings(serial_port, debug_serial):
    """Read an WeatherModbus sentence from a serial port.

    Args:
        serial_port (serial.Serial): An open port
        debug_serial (int): Set to greater than zero for extra debug information.

    Returns:
        string:  A string containing the sentence.
    """

    # Search for the character '{', which marks the beginning of a sentence:
    while True:
        ch_in = serial_port.read(1)
        if ch_in == b'{':
            break
    # Save the first '{' then read until we get to a '\r' or '\n'
    buf = bytearray(ch_in)
    term = set([ b'\n', b'\r'])
    while True:
        ch_in = serial_port.read(1)
        if ch_in in term:
            # We found a carriage return or newline, so we have the complete sentence.
            # NB: Because WeatherModbus terminates a sentence with a '\r\n', this will
            # leave a newline in the buffer. We don't care: it will get skipped over when
            # we search for the next sentence.
            break
        buf += ch_in
    line = bytes(buf).decode()
    if debug_serial:
        log.debug('Station said: %s', line)
    return line


def parse_readings(line):
    """
    line is input from usb port converted to string;
    attempt packet = json.loads(line) and catch exception on decode error;
    keys of packet are in format of arduino class weather_lib_data
    which is already in final form used by weewx as observation types
    and names of fields in database with a few exceptions;
    keys beginning with 'ex_' are not in weewx schema but a few need to be retained
    after converting name;
    edit packet for these few keys then delete remaining keys starting with 'ex_'
    then return finished packet (a dict());

    wind speed          (mi/hr         )
    wind direction      (degrees       )
    wind speed max      (mi/hr         )
    wind rating         (no unit; 0 to 17)
    outdoor temperature (F             )
    outdoor humidity    (RH %          )
    luminosity full     (W/m2          )
    luminosity infrared (W/m2          )
    luminosity visible  (W/m2          )
    lux                 (W/m2          )
    lux2                (W/m2          )
    rho                 (W/m2          )
    UV                  (UV index      )
    long term rain      (inch          )
    rain bucket tips    (# bucket_tips )
    rain bucket change  (# bucket_tips )
    rain bucket prev    (# bucket_tips )
    rain reset          (bool          )
    temperat client amb (F             )
    humidity client amb (RH %          )
    temperat client cpu (F             )
    temperat light  amb (F             )
    humidity light  amb (RH %          )
    temperat light  cpu (F             )
    temperat rain   amb (F             )
    humidity rain   amb (RH %          )
    temperat rain   cpu (F             )
    indoor temperature  (F             )
    indoor humidity     (RH %          )
    pressure            (inHg          )
    gas resistance      (kOhm          )
    altitude            (ft            )
    iaq                 (IAQ index     )  index air qual 0 to 500
    co2                 (ppm           )
    iaq_acc             (bool          )  iaq auto calibration done when value = 3

    Args:
        line: A string containing the sentence.

    Returns
        dict: A dictionary containing the data.

    """
    try:
        packet = json.loads(line)
        key_list = list(packet.keys())
        packet['radiation'] = packet['ex_lux']
        packet['rain_total'] = packet['ex_rain_total']
        packet['altitude'] = packet['ex_altitude']
        del packet['signal6']
        # delete all keys beginning with 'ex_' for extra
        for key in key_list:
            if key[0:3] == 'ex_':
                del packet[key]
        return packet
    except json.JSONDecodeError as exc:
        log.error('parse json error %s', str(exc))
        return {}


class WeatherModbusConfEditor(weewx.drivers.AbstractConfEditor):
    """standard weewx configuration editor"""

    @property
    def default_stanza(self):
        return f"""
[WeatherModbus]
    # This section is for the WeatherModbus weather station.

    # Serial port such as /dev/ttyS0, /dev/ttyUSB0, /dev/cua0, /dev/ttyACM0 etc...
    port = {Station.DEFAULT_PORT}

    # The driver to use:
    driver = weewx.drivers.wmod_driver
"""

    def prompt_for_settings(self):
        """prompt for port setting"""
        print('Specify the serial port on which the station is connected,')
        print('for example: /dev/ttyACM0 or /dev/ttyUSB0 or /dev/ttyS0 or /dev/cua0.')
        port = self._prompt('port', Station.DEFAULT_PORT)
        return {'port': port}


def process_options():
    """
    parse command args and
    handle simplest options then exit
    """

    # usage = """%prog [options] [--help]"""
    usage = """weewx_wmod.py [options] [--help]"""
    log.info('argparse.ArgumentParser')
    parser = argparse.ArgumentParser(usage=usage)
    parser.add_argument('--version',
                        dest='version',
                        action='store_true',
                        help='display driver version')
    parser.add_argument('--debug',
                        dest='debug',
                        default=DEFAULT_DEBUG,
                        action='store_true',
                        help='display diagnostic information while running')
    parser.add_argument('--port',
                        dest='port',
                        metavar='PORT',
                        help='serial port to which the station is connected',
                        default=Station.DEFAULT_PORT)

    log.info('parser.parse_args')
    options = parser.parse_args()

    if options.version:
        log.info('wmod driver version %s', DRIVER_VERSION)
        sys.exit(1)

    if options.debug:
        weewx.debug = 1
        log.info('log.setLevel(logging.DEBUG)')
        log.setLevel(logging.DEBUG)

    return options


def main():
    """
    define a main entry point for basic testing of the station without weewx
    engine and service overhead.  invoke this as follows from the weewx root dir:
    PYTHONPATH=bin python bin/weewx/drivers/ultimeter.py
    """

    # signal handling
    # port close is probably already done by station destructor; just in case;
    
    def testing_handle_interrupt(signum, stack):
        """catch CONTROL-C for example; cleanup; close port etc.;"""
        log.error('Handling interrupt.  Closing port.')
        log.error('signum = %d', signum)
        log.error('stack  = %s', str(stack))
        station.close()
        log.error('closed port.  exiting.')
        sys.exit(0)

    signal.signal(signal.SIGTERM, testing_handle_interrupt)
    signal.signal(signal.SIGINT, testing_handle_interrupt)

    log.addHandler(logging.StreamHandler(sys.stdout))
    log.addHandler(logging.handlers.SysLogHandler(address='/dev/log'))
    print('log.setLevel INFO')
    log.setLevel(logging.INFO)

    options = process_options()

    # weeutil.logger.setup('wmod', {})

    with Station(options.port, debug_serial=options.debug) as station:
        # station.set_logger_mode()
        while True:
            # max_tries  = 5 is default
            # retry_wait = 5 is default
            readings1 = station.get_readings_with_retry(max_tries = 5, retry_wait = 5)
            log.info('%d %s', time.time(), str(readings1))
            data1 = parse_readings(readings1)
            log.info('%d %s', time.time(), str(data1))


if __name__ == '__main__':
    main()

# eee eof
