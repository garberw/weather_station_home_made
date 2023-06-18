#!/usr/bin/env python
"""
the previous weewx_atlas.py adafruit version was a
modified weewx-sdr driver with AsyncReader with a queue
which merged stdout from rtl_433 over usb and wpa arduino over usb;
deprecated 4/28/23 at advice of GJR (Gary) on weewx blog;
the original unmodified driver is now used and my stuff was moved to this new service;
the new service has same name as original driver weewx_atlas.py (this file);

the new custom service is AddAtlasInside; see [Engine][[Services]] section at end under
         data_services = user.weewx_atlas.AddAtlasInside,

AddAtlasInside processes NEW_LOOP_PACKET events only;
(A) get_wpa_arduino_data();
    read line from usb serial port and json.loads();
    return packeti which is a json dict of inside weather;
    does not need to contain time or usunits;
(B) new_loop_packet();
    packeto = event.packet.copy();
    packeti = get_wpa_arduino_data();
    a few edits to packeti; 
    event.packet.update(packeti);
    (update is a standard dict() member that merges two dicts);
(C) WeeWX engine accumulates (averages) packets when running callback for NEW_ARCHIVE_RECORD;

GJR comments;

It is important that the function download_total_power() does not delay very long
because it will sit right in the main loop of the WeeWX engine.
If it is going to cause a delay of more than a couple seconds
you might want to put it in a separate thread and
feed the results to AddElectricity through a queue.

Why not run a standard sdr driver to feed WeeWX with loop packets from the Atlas
with a simple, non-threaded data service bound to the new loop packet arrival
to read the indoor data (pressure, temperature and humidity) and augment the loop packet.
Far more modular and easier to test/develop (and get help),
you will be running a standard sdr driver,
and since you are already getting your hands dirty modifying the sdr driver,
writing a small data service to handle the Arduino input should be a walk in the park.
If the Arduino is serially connected to the RPi
you should not have great latency in accessing data,
so a suitably short timeout on the serial reads should provide you with your
indoor data without blocking the main WeeWX thread.
Once proved, the serial read could be moved to a thread if you really had the need.
"""

from __future__ import with_statement
import threading
import select
import signal
import sys
import time
from datetime import datetime
import argparse
import logging
import logging.handlers
import serial

import weewx.units
import weecfg
import weeutil.logger
import weeutil.weeutil
from weeutil.weeutil import to_int
from weewx.engine import StdService, StdEngine

try:
    import cjson as json
    setattr(json, 'dumps', json.encode)
    setattr(json, 'loads', json.decode)
except (ImportError, AttributeError):
    try:
        import simplejson as json
    except ImportError:
        import json

SERVICE_VERSION = '0.00'
DEFAULT_CONFIG='/etc/weewx/weewx_atlas.conf'
# thp stands for indoors; temp; humidity; pressure;
# DEFAULT_FNAME_USB_THP = '/dev/ttyACM0'
DEFAULT_FNAME_USB_THP = '/dev/ttyACMwa'
DEFAULT_DEBUG = 1
READ_ATTEMPT_MAX = 140

log = logging.getLogger(__name__)

class AsyncReader(threading.Thread):
    """
    read all input from wpa (weather port Atlas) arduino;
    reject lines that do not begin with '{' i.e. keep only json data;
    keep only self._last_line == most recent line of json data;
    """
    def __init__(self, fd, label):
        threading.Thread.__init__(self)
        self._fd = None
        self._last_line = None
        self._lock_last_line = threading.Lock()
        self._partial = ''
        self._attempt = 0
        self._running = False
        self.setDaemon(True)
        self.setName(label)
        self._exited = False

    def read_all_with_error(self):
        """read whole input buffer; return empty if error or timeout;"""
        # block until one more char or timeout
        tmp = self._fd.read(1).decode()
        if len(tmp) != 1:
            log.info('read_all_with_error() timeout;')
            return None
        # small block to collect data; buffer should not overflow in this time;
        # zero works okay
        # time.sleep(0.100)
        # time.sleep(0.010)
        nwait = self._fd.inWaiting()
        tmp += self._fd.read(nwait).decode()
        if len(tmp) != 1 + nwait:
            log.info('read_all_with_error() timeout;')
            return None
        return tmp

    def get_last_line_of_json_data(self):
        """
        main iteration;
        read entire chunk of all data waiting in input buffer received from arduino;
        blocking with timeout of serial port; only blocks on reading first char;
        split and loop over lines;
        store last line which begins with '{' in self._last_line;
        if final line in split list does not end with newline it is a partial line
        so save for next iteration to be concatenated with rest of line;
        """
        buf = self.read_all_with_error()
        if not buf:
            self._partial = ''
            return
        line_list = buf.splitlines(keepends = True)
        if not line_list:
            self._partial = ''
            return
        # concatenate partial last line on previous call + line_list[0]
        line_list[0] = self._partial + line_list[0]
        # find line == last line beginning with '{' and ending with newline;
        line = None
        self._partial = ''
        for jjj, tmp in enumerate(line_list):
            if tmp[0] == '{':
                if tmp[-1] == '\n':
                    # found a valid line; keep;
                    line = tmp
                elif jjj == len(line_list) - 1:
                    # found partial last line;
                    self._partial = tmp
        if line:
            log.debug('after attempt = %d  line = %s', self._attempt, line)
            with self._lock_last_line:
                self._last_line = line
            self._attempt = 0
            return
        self._attempt += 1

    def get_last_line_of_json_data1(self):
        """this may be all you need;"""
        timeout = 12
        # select(rlist,wlist,xlist,timeout); wait until ready for reading (rlist);
        ready,_,_ = select.select([self._fd], [], [], timeout)
        if not ready:
            return
        line = self._fd.readline().rstrip().decode()
        log.debug('read line = <<%s>>', line)
        if not line:
            return
        if not line.startswith('{'):
            return
        self._last_line = line

    def get_last_line(self):
        """return last line found; lock checks it is not currently being saved;"""
        with self._lock_last_line:
            # strings are immutable so you do not need a copy
            line = self._last_line
        return line

    def set_fd(self, fd):
        self._fd = fd

    def run(self):
        self._exited = False
        """iterate while self._running;"""
        log.info('start AsyncReader for %s', self.getName())
        self._running = True
        while self._running:
            if self._fd:
                self.get_last_line_of_json_data()
            else:
                time.sleep(0.5)
        self._exited = True

    def complete(self):
        return self._exited

    def stop_running(self):
        """will halt thread at end of newx get_last_line_of_json_data;"""
        self._running = False



class AddAtlasInside(StdService):
    """
    service that processes NEW_LOOP_PACKET events;
    (1) get_wpa_arduino_data() returns partial packet of indoor data packeti;
    (2) new_loop_packet() input is packeto (outdoor data packet); update (merge) with packeti;
    """

    def __init__(self, engine, config_dict):
        super().__init__(engine, config_dict)
        # Bind to any weewx.NEW_LOOP_PACKET events
        self.bind(weewx.NEW_LOOP_PACKET, self.new_loop_packet)

        archive_dict = config_dict.get('StdArchive', {})
        self._archive_interval = to_int(archive_dict.get('archive_interval', 300))
        log.info('_archive_interval = %d', self._archive_interval)

        # e.g. serial usb port == ttyACM0 or ttyACMwa
        sdr_dict = config_dict.get('SDR', {})
        self._fname_usb_thp = sdr_dict.get('fname_usb_thp', DEFAULT_FNAME_USB_THP)
        log.info('_fname_usb_thp = %s', self._fname_usb_thp)

        # open serial port; start async reader;
        self._fptr_usb_thp = None
        self.wpa_reader = None
        try:

            # delay opening port until service runs new_loop_packet;
            # wee_report calls init;
            # wee_report may be run while weewx has port open;
            # can not open port twice;
            self._fptr_usb_thp = None

            self.wpa_reader = AsyncReader(self._fptr_usb_thp, 'wpa-reader-thread')
            self.wpa_reader.start()

        except (OSError, ValueError) as exc:
            log.error('startup failed')
            log.error('exc= %s', str(exc))
            raise weewx.WeeWxIOError(f'failed to start AddAtlasInside; {exc}')

        log.info('AddAtlasInside startup passed')


    def open(self):
        """
        open port if it is not already open;
        unfortunately we do not get to find out if this passes until new_loop_packet
        arrives on other RTL-SDR usb port;
        """
        if self._fptr_usb_thp and self._fptr_usb_thp.is_open:
            return
        try:
            # timeout = 14
            # timeout = 10
            timeout = 5
            log.info('open serial port')
            self._fptr_usb_thp = serial.Serial(port=self._fname_usb_thp,
                                               baudrate=115200,
                                               timeout=timeout)
            self.wpa_reader.set_fd(self._fptr_usb_thp)
        except (OSError, ValueError) as exc:
            log.error('open port %s failed', self._fptr_usb_thp)
            log.error('exc= %s', str(exc))
            raise weewx.WeeWxIOError(f'failed to open port AddAtlasInside; {exc}')

    def close(self):
        """
        close port; 
        wpa_reader uses _fptr_usb_thp so join thread before closing _fptr_usb_thp;
        """
        self._fptr_usb_thp.close()
        self._fptr_usb_thp = None

    def shutDown(self):
        """close _fptr_usb_thp ttyACM* serial port; join wpa_reader thread;"""
        log.info('AddAtlasInside.shutDown()')
        self.wpa_reader.stop_running()
        while not self.wpa_reader.complete():
            time.sleep(0.1)
        self.close()
        # wpa_reader uses _fptr_usb_thp so join thread before closing _fptr_usb_thp;
        self.wpa_reader.join(0.5)
        if self.wpa_reader.is_alive():
            log.error('timed out waiting for %s', self.wpa_reader.getName())
        else:
            log.info('completed %s', self.wpa_reader.getName())
        self.wpa_reader = None
        log.info('shutdown complete')

    def new_loop_packet(self, event):
        """
        callback bound to NEW_LOOP_PACKET events; this should be VERY FAST;
        (1) packeto = just a copy of event.packet for reference; this is outside weather data;
        (2) line = wpa_reader.get_last_line() instantly returns last line of json data or None;
        (3) packeti = json.loads(line); do nothing on fail; this is inside weather data;
        (4) make minor changes to packeti;
        (5) event.packet.update(packeti) merges outside data with packeti;

        NEW_LOOP_PACKET event.packet should already have these two (key: value) pairs;
            'dateTime': 12345678.12345, 'usUnits': 1,
        """
        log.debug('new_loop_packet() begin ===============================================')
        # open port if it is not already open
        self.open()
        time_packet1 = event.packet['dateTime']
        time_packet2 = datetime.utcfromtimestamp(time_packet1).strftime('%Y-%m-%d %H:%M:%S')
        log.debug('time_packet1=%d time_packet2=%s', time_packet1, time_packet2)
        interval_t1 = weeutil.weeutil.startOfInterval(time_packet1, self._archive_interval)
        interval_t2 = interval_t1 + self._archive_interval
        log.debug('interval_t1=%d interval_t2=%d', interval_t1, interval_t2)

        # inside; this should be instant;
        line = self.wpa_reader.get_last_line()
        if not line:
            return
        try:
            packeti = json.loads(line)
        except json.JSONDecodeError as exc:
            log.error('new_loop_packet() json.loads failed    line = %s', line)
            log.error('new_loop_packet() JSONDecodeError       exc = %s', str(exc))
            return
        # outside
        packeto = event.packet.copy()

        # minor changes to packeti
        # for this specific weather station we assign arbitrary caseTemp1 caseHumid1 cpuTemp1;
        if 'outTemp' in packeto.keys():
            packeti['caseTemp1' ] = packeto['outTemp']
            packeti['cpuTemp1'  ] = packeto['outTemp']
        if 'outHumidity' in packeto.keys():
            packeti['caseHumid1'] = packeto['outHumidity']
        # remove aaa_millis which was just for debugging;
        if 'aaa_millis' in packeti.keys():
            del packeti['aaa_millis']

        # event.packet = merge packeti with packeto
        event.packet.update(packeti)

        log.debug('new_loop_packet(); packet inside;        packeti=%s', packeti)
        log.debug('new_loop_packet(); packet outside;       packeto=%s', packeto)
        log.debug('new_loop_packet(); final augmented; event.packet=%s', event.packet)


    # fake test packet
FAKE_PACKET = {
    'dateTime'    : int(time.time()),
    'usUnits'     : weewx.US,
    'windSpeed'   : 50.0,
    'windDir'     : 50.0,
    'outTemp'     : 50.0,
    'outHumidity' : 50.0,
    'lux'         : 50.0,
    'UV'          : 50.0,
    'rain_total'  : 50.0,
    'caseTemp1'   : 50.0,
    'caseHumid1'  : 50.0,
    'cpuTemp1'    : 50.0,
    'signal2'     : 50.0,
    'signal3'     : 50.0,
    'signal4'     : 50.0,
    'signal5'     : 50.0,
    'signal6'     : 50.0,
    'signal7'     : 50.0,
}

def process_options_get_config_dict():
    """
    parse command args and 
    (1) handle simplest options then exit or
    (2) return config_dict and continue
    """

    usage = """weewx_atlas [--debug] [--help] [--version]"""
    log.info('argparse.ArgumentParser')
    parser = argparse.ArgumentParser(description=usage)
    parser.add_argument('--version',
                        dest='version',
                        action='store_true',
                        help='display driver version')
    parser.add_argument('--debug',
                        dest='debug',
                        default=DEFAULT_DEBUG,
                        action='store_true',
                        help='display diagnostic information while running')
    parser.add_argument('--config',
                        default=DEFAULT_CONFIG,
                        action='store',
                        help='configuration file with sensor map')

    log.info('parser.parse_args')
    options = parser.parse_args()

    if options.version:
        log.info('weewx_atlas version %s', SERVICE_VERSION)
        sys.exit(1)

    if options.debug:
        log.info('log.setLevel(logging.DEBUG)')
        log.setLevel(logging.DEBUG)

    log.debug('options.config = %s', str(options.config))
    log.debug('reading config_dict')
    _, config_dict = weecfg.read_config(options.config)

    archive_interval_demo_use = True
    archive_interval_demo = 30     # seconds

    archive_dict = config_dict.get('StdArchive', {})
    archive_interval_in = archive_dict.get('archive_interval', None)
    if archive_interval_demo_use:
        log.debug('using archive_interval_demo; alter config;')
        archive_dict['archive_interval'] = str(archive_interval_demo)
        log.debug('archive_interval_in  = %s', str(archive_interval_in))
        log.debug('archive_interval_out = %s', str(archive_interval_demo))
    else:
        log.debug('using archive interval from options.config')
        log.debug('archive_interval_out = %s', str(archive_interval_in))
    return config_dict


def main():
    """
    test service; not used during weewx run; to run test do;
    PYTHONPATH=$PYTHONPATH:/usr/share/weewx python /usr/share/weewx/user/weewx_atlas.py [OPTIONS]
    """

    # signal handling
    # usb port close is probably already done by station destructor; just in case;
    svc = None

    def testing_handle_interrupt(signum, stack):
        """catch keyboard interrupts; clean up; close usb port;"""
        log.error('Handling interrupt.  Closing port.')
        log.error('signum = %d', signum)
        log.error('stack = %s', str(stack))
        svc.shutDown()
        log.error('closed port.  exiting.')
        sys.exit(0)

    signal.signal(signal.SIGTERM, testing_handle_interrupt)
    signal.signal(signal.SIGINT, testing_handle_interrupt)

    log.addHandler(logging.StreamHandler(sys.stdout))
    log.addHandler(logging.handlers.SysLogHandler(address='/dev/log'))
    print('log.setLevel INFO')
    log.setLevel(logging.INFO)

    config_dict = process_options_get_config_dict()

    log.debug('StdEngine constructor')
    eng = StdEngine(config_dict)

    log.debug('AddAtlasInside service constructor')
    svc = AddAtlasInside(eng, config_dict)

    while True:
        log.debug('main loop begin %s', 'M' * 70)
        # log.debug('sleeping archive_interval= %d', archive_interval)
        # time.sleep(archive_interval)
        event = weewx.Event(weewx.NEW_LOOP_PACKET)
        event.packet = FAKE_PACKET.copy()
        event.packet['dateTime'] = int(time.time())
        log.debug('begin event.packet = %s', str(event.packet))
        svc.new_loop_packet(event)
        log.debug('end   event.packet = %s', str(event.packet))
        log.debug('main loop end   %s', 'M' * 70)


if __name__ == '__main__':
    main()

# eee eof
