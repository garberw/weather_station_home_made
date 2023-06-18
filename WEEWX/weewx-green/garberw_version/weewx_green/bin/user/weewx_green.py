#!/usr/bin/env python
"""
Copyright 2023-2029 William Garber
See the file LICENSE.txt for your rights.

Copyright 2014-2020 Matthew Wall
Copyright 2014 Nate Bargmann <n0nb@n0nb.us>
See the file LICENSE.txt for your rights.

Credit to and contributions from:
  Jay Nugent (WB8TKL) and KRK6 for weather-2.kr6k-V2.1
    http://server1.nuge.com/~weather/
  Steve (sesykes71) for testing the first implementations of this driver
  Garret Power for improved decoding and proper handling of negative values
  Chris Thompstone for testing the fast-read implementation

Driver for Green weather station

This driver assumes the weathere is emitting data to weathere:data_filename_e.
This driver assumes the weatherg is emitting data to weatherg:data_filename_g.

This file is copied using paramiko command 'scp' (ssh secure copy).
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
import socket
from scp import SCPClient
from scp import SCPException
from paramiko import SSHClient
from paramiko import BadHostKeyException, AuthenticationException, SSHException

import weewx
import weewx.drivers
import weewx.wxformulas

DEFAULT_DEBUG = 1
STATION_E_OUTSIDE_RUNNING = True
DRIVER_NAME = 'Green'
DRIVER_VERSION = '0.0'

DEFAULT_DATA_FILENAME_E = '/tmp/weathere_final.dat'   # FNAME_FINAL
DEFAULT_DATA_FILENAME_G = '/tmp/weatherg_final.dat'   # FNAME_FINAL
IS_WEEWX_SERVER_E = False
IS_WEEWX_SERVER_G = False
SERVER_E = 'weathere'
SERVER_G = 'weatherg'
# MAIN_LOOP_SLEEP = 5 # seconds
# MAIN_LOOP_SLEEP = 60  # seconds removed 3/9/23
MAIN_LOOP_SLEEP = 10  # seconds

REQUIRED_KEYS_E = set([
    'windSpeed',    # mph
    'windDir',      # compass deg
    'outTemp',      # F
    'outHumidity',  # %
    'lux',          # W/m2
    'UV',           # UV index
    'rain_total',   # inch
    'caseTemp1',    # F
    'caseHumid1',   # %
    'cpuTemp1',     # F
    'signal2',      # W/m2
    'signal3',      # W/m2
    'signal4',      # W/m2
    'signal5',      # W/m2
    'signal6',      # W/m2
    'signal7',      # W/m2
])

REQUIRED_KEYS_G = set([
    'inTemp',           # degree_F
    'inHumidity',       # percent
    'pressure',         # inHg
    'gas_resistance',   # kOhm
    'iaq',              # IAQ
    'co2',              # ppm
    'ex_altitude',      # ft
    'ex_acc_iaq',       # converged when 3.0
])

log = logging.getLogger(__name__)

def loader(config_dict, _):
    """GreenDriver  loader"""
    return GreenDriver(**config_dict[DRIVER_NAME])


def confeditor_loader():
    """GreenConfEditor loader"""
    return GreenConfEditor()


class GreenDriver(weewx.drivers.AbstractDevice):
    """weewx driver that communicates with a Green station"""

    def __init__(self, **stn_dict):
        """
        Args:
        data_filename_e(str):
            Required. Default is '/tmp/weathere_final.dat'

        data_filename_g(str):
            Required. Default is '/tmp/weatherg_final.dat'

        max_tries(int): How often to retry communication before giving up
            Optional. Default is 5

        retry_wait (float): How long to wait before retrying an I/O operation.
            Optional. Default is 3.0
        """
        self.model = 'Green'
        self.data_filename_e = stn_dict.get('data_filename_e', DEFAULT_DATA_FILENAME_E)
        self.data_filename_g = stn_dict.get('data_filename_g', DEFAULT_DATA_FILENAME_G)
        self.max_tries = int(stn_dict.get('max_tries', 20))
        self.retry_wait = float(stn_dict.get('retry_wait', 3.0))
        self.last_rain = None

        log.info('Driver version is %s', DRIVER_VERSION)
        log.info('Using data_filename_e %s', self.data_filename_e)
        log.info('Using data_filename_g %s', self.data_filename_g)
        # wait for greene.service startup on same computer to create at least one
        # file /tmp/weatherg_final.dat
        # 5 seconds
        time.sleep(5)
        self.station = Station(self.data_filename_e, self.data_filename_g)

    @property
    def hardware_name(self):
        """get/set"""
        return self.model

#    def getTime(self):
#        """get/set"""
#        return self.station.get_time()

#    def setTime(self):
#        """get/set"""
#        self.station.set_time(int(time.time()))

    def genLoopPackets(self):
        """infinite loop running get_readings_with_retry_e and _g and yielding packet"""
        while True:
            packet = {'dateTime': int(time.time() + 0.5), 'usUnits': weewx.US}
            if STATION_E_OUTSIDE_RUNNING:
                readings_e = self.station.get_readings_with_retry_e(self.max_tries,
                                                                    self.retry_wait)
                readings_g = self.station.get_readings_with_retry_g(self.max_tries,
                                                                    self.retry_wait)
                data_e = parse_readings_e(readings_e)
                data_g = parse_readings_g(readings_g)
                data = dict(data_e, **data_g)
                packet.update(data)
                self._augment_packet(packet)
            else:
                readings_g = self.station.get_readings_with_retry_g(self.max_tries,
                                                                    self.retry_wait)
                data_g = parse_readings_g(readings_g)
                data = data_g
                packet.update(data)
                # self._augment_packet(packet)
            yield packet

    def _augment_packet(self, packet):
        """convert rain_total to rain"""
        packet['rain'] = weewx.wxformulas.calculate_rain(packet['rain_total'], self.last_rain)
        self.last_rain = packet['rain_total']


class SCPConnectionException(Exception):
    """SCPConnectionException"""

class SubStation:
    """either indoors 'g' or outdoors 'e'; two stations are very similar;"""
    def __init__(self,
                 data_filename,
                 server,
                 main_loop_sleep,
                 is_weewx_server):
        """what differentiates 'g' from 'e'"""
        self.data_filename = data_filename
        self.server = server
        self.main_loop_sleep = main_loop_sleep
        self.is_weewx_server = is_weewx_server

    def __enter__(self):
        """__enter__"""
        return self

    def __exit__(self, _, value, traceback):
        """__exit__"""

    def close_scp(self):
        """stub"""

    def get_readings_with_retry(self,  max_tries, retry_wait):
        """
        if is_weewx_server the file data_filename is already on this computer;
        otherwise set up an ssh connection from this computer to server;
        purpose is to scp data_filename to server;

        repeat for up to max_tries attempts until success;
        scp data_filename from remote to local;
        reattempt to setup_scp if connection is dead;
        read one line of Green json data from data_filename;

        Args:
        data_filename (string):  data obtained by scp from weathere or weatherg;

        Returns:
        string: line of json data; no trailing whitespace \r \n;
        """
        src = self.data_filename
        dst = self.data_filename
        for ntries in range(max_tries):
            if not self.is_weewx_server:
                try:
                    with SSHClient() as ssh:
                        log.debug('load_system_host_keys')
                        ssh.load_system_host_keys()
                        log.debug('ssh.connect server = %s', self.server)
                        ssh.connect(self.server)
                        # SCPClient takes a paramiko transport as an argument
                        log.debug('create SCPClient')
                        with SCPClient(ssh.get_transport()) as scp:
                            log.debug('create SCPClient pass')
                            # scp.get(remote, local)
                            # remote = src
                            # local  = dst
                            log.debug('scp.get(src, dst)')
                            log.debug('src = %s', src)
                            log.debug('dst = %s', dst)
                            scp.get(src, dst)
                            log.debug('scp.get() pass')
                except (SCPException, IOError, BadHostKeyException,
                        AuthenticationException, SSHException, socket.error) as exc:
                    log.error('SCPException  attempt to restart connection')
                    log.error('exc = %s', str(exc))
                    time.sleep(retry_wait)
                    continue
            try:
                log.debug('open dst = %s', dst)
                # read whole file
                log.debug('read dst = %s', dst)
                with open(dst, 'r', encoding = 'ascii') as fptr:
                    buf = fptr.read().rstrip()
                log.debug('read pass')
                log.debug('read buf = %s', str(buf))
                time.sleep(self.main_loop_sleep)
                return buf
            except (OSError, ValueError, EOFError) as exc:
                log.error('error:  read dst failed')
                log.error('dst = %s', dst)
                log.error('exc = %s', str(exc))
                log.error('Failed attempt %d of %d to get readings %s: %s',
                         ntries, max_tries, dst, exc)
                time.sleep(retry_wait)
                continue
        log.error('max_tries %d exceeded reading dst=%s', max_tries, dst)
        raise weewx.RetriesExceeded(f'retries {max_tries=} exceeded reading {dst=}')


class Station:
    """simply two SubStations; indoors 'g' for green and outndoors 'e' for external"""
    def __init__(self,
                 data_filename_e = DEFAULT_DATA_FILENAME_E,
                 data_filename_g = DEFAULT_DATA_FILENAME_G):
        self.sst_e = None
        if STATION_E_OUTSIDE_RUNNING:
            self.sst_e = SubStation(data_filename_e,
                                    SERVER_E,
                                    MAIN_LOOP_SLEEP,
                                    IS_WEEWX_SERVER_E)
        self.sst_g = SubStation(data_filename_g,
                                SERVER_G,
                                MAIN_LOOP_SLEEP,
                                IS_WEEWX_SERVER_G)
        # tofix remove this
        # self.can_set_year = True
        # self.has_modem_mode = True

    def __enter__(self):
        """stub"""
        return self

    def __exit__(self, _, value, traceback):
        """stub"""
        self.close()

    def close(self):
        """stub"""
        if STATION_E_OUTSIDE_RUNNING:
            self.sst_e.close_scp()
        self.sst_g.close_scp()

    def get_readings_with_retry_e(self, max_tries, retry_wait):
        """stub"""
        if STATION_E_OUTSIDE_RUNNING:
            return self.sst_e.get_readings_with_retry(max_tries, retry_wait)
        return None

    def get_readings_with_retry_g(self, max_tries, retry_wait):
        """stub"""
        return self.sst_g.get_readings_with_retry(max_tries, retry_wait)


# ##############################################################33
#                 Utilities
# ##############################################################33


def parse_readings_e(line):
    """
    input; line = string in json format;
    packet = json.loads(line);
    make modifications to line to create a partial packet for outside data;
    must include required keys;
    units are already converted;
    does not include datetime; dateTime; usUnits;
    returns;  packet which is a dict;
    """
    try:
        data = json.loads(line)
    except json.JSONDecodeError as exc:
        raise weewx.WeeWxIOError(f'json decode error {exc=}')
    present_keys = set(data.keys())
    if not REQUIRED_KEYS_E.issubset(present_keys):
        log.debug('REQUIRED_KEYS_E = %s', str(REQUIRED_KEYS_E))
        log.debug('present_keys    = %s', str(present_keys))
        raise weewx.WeeWxIOError('keys missing error e')
    # REQUIRED_KEYS_E
    # units already converted
    # other data included
    # windRating    # 0 to 17
    # windSpeedMax  # mph
    # rain_reset    # boolean
    # could also check all float or None
    del data['windSpeedMax']
    del data['windRating']
    del data['rain_reset']
    # could also check all float or None
    return data

def parse_readings_g(raw):
    """
    input; line = string in json format;
    packet = json.loads(line);
    make modifications to line to create a partial packet for outside data;
    must include required keys;
    units are already converted;
    does not include datetime; dateTime; usUnits;
    returns;  packet which is a dict;
    """
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise weewx.WeeWxIOError(f'json decode error {exc=}')
    # REQUIRED_KEYS_G
    # units already converted
    present_keys = set(data.keys())
    if not REQUIRED_KEYS_G.issubset(present_keys):
        log.debug('REQUIRED_KEYS_G = %s', str(REQUIRED_KEYS_G))
        log.debug('present_keys    = %s', str(present_keys))
        raise weewx.WeeWxIOError('keys missing error g')
    # we can just leave extra data there; weewx will have access if needed
    # del data['ex_altitude']
    # could also check all float or None
    return data


class GreenConfEditor(weewx.drivers.AbstractConfEditor):
    """stub"""

    @property
    def default_stanza(self):
        return f"""
[Green]
    # This section is for the Green weather station.

    data_filename_e = {DEFAULT_DATA_FILENAME_E}
    data_filename_g = {DEFAULT_DATA_FILENAME_G}

    # The driver to use:
    driver = weewx.drivers.green_driver
"""

    def prompt_for_settings(self):
        print('Specify the data_filename which the server writes to,')
        print('for example: /tmp/weathere_final.dat')
        data_filename_e = self._prompt('data_filename_e', DEFAULT_DATA_FILENAME_E)
        data_filename_g = self._prompt('data_filename_g', DEFAULT_DATA_FILENAME_G)
        return {'data_filename_e': data_filename_e, 'data_filename_g': data_filename_g}


def process_options():
    """
    parse command args and
    handle simplest options then exit
    """

    # usage = """%prog [options] [--help]"""
    usage = """weewx_green.py [options] [--help]"""
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
    parser.add_argument('--data_filename_e',
                        dest='data_filename_e',
                        metavar='DATA_FILENAME_E',
                        help='weathere server data filename; copied to client and read by weewx',
                        default=DEFAULT_DATA_FILENAME_E)
    parser.add_argument('--data_filename_g',
                        dest='data_filename_g',
                        metavar='DATA_FILENAME_G',
                        help='weatherg server data filename; copied to client and read by weewx',
                        default=DEFAULT_DATA_FILENAME_G)

    log.info('parser.parse_args')
    options = parser.parse_args()

    if options.version:
        log.info('weewx_green.py version %s', DRIVER_VERSION)
        sys.exit(0)

    if options.debug:
        log.info('log.setLevel(logging.DEBUG)')
        log.setLevel(logging.DEBUG)
        weewx.debug = 1

    return options

def main():
    """
    define a main entry point for basic testing of the station without weewx
    engine and service overhead.
    invoke this as follows from the weewx root dir:
    PYTHONPATH=/usr/share/weewx/ python /user/share/weewx/user/weewx_green.py
    """

    log.addHandler(logging.StreamHandler(sys.stdout))
    log.addHandler(logging.handlers.SysLogHandler(address='/dev/log'))
    print('log.setLevel INFO')
    log.setLevel(logging.INFO)

    options = process_options()

    with Station(options.data_filename_e, options.data_filename_g) as station:

        def testing_handle_interrupt(signum, stack):
            """catch control C when testing but not when running weewx"""
            log.error('Handling interrupt.')
            log.error('signum = %d', signum)
            log.error('stack  = %s', str(stack))
            # close scp connections
            station.close()
            sys.exit(0)

        signal.signal(signal.SIGTERM, testing_handle_interrupt)
        signal.signal(signal.SIGINT, testing_handle_interrupt)

        while True:
            if STATION_E_OUTSIDE_RUNNING:
                readings_e1 = station.get_readings_with_retry_e(max_tries = 5, retry_wait = 5)
                readings_g1 = station.get_readings_with_retry_g(max_tries = 5, retry_wait = 5)
                log.info('%d %s', time.time(), str(readings_e1))
                log.info('%d %s', time.time(), str(readings_g1))
                data_e1 = parse_readings_e(readings_e1)
                data_g1 = parse_readings_g(readings_g1)
                log.info('%d %s', time.time(), str(data_e1))
                log.info('%d %s', time.time(), str(data_g1))
            else:
                readings_g1 = station.get_readings_with_retry_g(max_tries = 5, retry_wait = 5)
                log.info('%d %s', time.time(), str(readings_g1))
                data_g1 = parse_readings_g(readings_g1)
                log.info('%d %s', time.time(), str(data_g1))
            time.sleep(5)

if __name__ == '__main__':
    main()

# eee eof
