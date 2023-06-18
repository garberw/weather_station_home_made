#!/usr/bin/env python
#
# Copyright 2022 William Garber <william.garber@att.net>
# See the file LICENSE.txt for your rights.

"""weewx_calibrate
"""

WEEWX_CALIBRATE_NAME = 'weewx_calibrate.py'
WEEWX_CALIBRATE_VERSION = '0.0'
WEEWX_CALIBRATE_DEBUG = False
WEEWX_CALIBRATE_COLUMNS = [ 'inTemp', 'inHumidity', 'outTemp', 'outHumidity', 'radiation', 'UV',
                            'WindSpeed', 'WindSpeedMax', 'WindDirection', 'rain', 'pressure']
#WEEWX_CALIBRATE_DB_FNAME_ATL = '/var/lib/weewx/weewx_atlas.sdb'
#WEEWX_CALIBRATE_DB_FNAME_WMO = '/var/lib/weewx/weewx_wmod.sdb'
#WEEWX_CALIBRATE_DB_FNAME_GRE = '/var/lib/weewx/weewx_green.sdb'

WEEWX_CALIBRATE_DB_FNAME_ATL = '/var/lib/weewx/test_calibrate/weewx_atlas.sdb'
WEEWX_CALIBRATE_DB_FNAME_WMO = '/var/lib/weewx/test_calibrate/weewx_wmod.sdb'
WEEWX_CALIBRATE_DB_FNAME_GRE = '/var/lib/weewx/test_calibrate/weewx_green.sdb'

# calibration in /etc/weewx/weewx_<station>.conf
# suppose L = list for this station and variable e.g. inTemp
# e.g. calibration is intercept L[0] and slope L[1]
# inTemp = L[0] + inTemp * L[1]
CALIBRATION_CONF = { }
CALIBRATION_CONF['atlas'] = { }
CALIBRATION_CONF['wmod' ] = { }
CALIBRATION_CONF['green'] = { }
CALIBRATION_CONF['atlas']['inTemp'	  ] = [-4.0, 1.0]
CALIBRATION_CONF['atlas']['inHumidity'    ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['outTemp'       ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['outHumidity'   ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['radiation'     ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['UV'            ] = [0.0, 3.33333333]
CALIBRATION_CONF['atlas']['WindSpeed'     ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['WindSpeedMax'  ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['WindDirection' ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['rain'	  ] = [0.0, 1.0]
CALIBRATION_CONF['atlas']['pressure'      ] = [0.05, 1.0] 
CALIBRATION_CONF['wmod' ]['inTemp'	  ] = [-8.1207, 1.0]
CALIBRATION_CONF['wmod' ]['outTemp'       ] = [-4.302, 1.0]
CALIBRATION_CONF['wmod' ]['outHumidity'   ] = [2.0, 1.0]
CALIBRATION_CONF['wmod' ]['radiation'     ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['UV'            ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['WindSpeed'     ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['WindSpeedMax'  ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['WindDirection' ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['rain'	  ] = [0.0, 1.0]
CALIBRATION_CONF['wmod' ]['pressure'      ] = [0.0383319, 1.0] 
CALIBRATION_CONF['green']['inTemp'	  ] = [-5.0, 1.0]
CALIBRATION_CONF['green']['inHumidity'    ] = [0.45, 1.0]
CALIBRATION_CONF['green']['outTemp'       ] = [-4.301949, 1.0]
CALIBRATION_CONF['green']['outHumidity'   ] = [4.1, 1.0]
CALIBRATION_CONF['green']['radiation'     ] = [0.0, 1.0]
CALIBRATION_CONF['green']['UV'            ] = [0.0, 1.0]
CALIBRATION_CONF['green']['WindSpeed'     ] = [0.0, 1.0]
CALIBRATION_CONF['green']['WindSpeedMax'  ] = [0.0, 1.0]
CALIBRATION_CONF['green']['WindDirection' ] = [0.0, 1.0]
CALIBRATION_CONF['green']['rain'	  ] = [0.0, 1.0]
CALIBRATION_CONF['green']['pressure'      ] = [0.05933, 1.0] 

WEATHER_STATIONS = [ 'atlas', 'wmod', 'green' ]

CALIBRATION_FIT = {}
for station in WEATHER_STATIONS:
    ddd = {}
    CALIBRATION_FIT[station] = ddd
    for col in WEEWX_CALIBRATE_COLUMNS:
        ddd[col] = [ 0.0, 1.0 ]
            
import datetime
import sqlite3
from scipy.interpolate import interp1d
import numpy as np
import matplotlib.pyplot as plt

def time_epoch_from_iso(time_iso):
    if time_iso is None:
        return 0
    offset = datetime.datetime.now(datetime.timezone.utc).isoformat()[26:]
    tstr = datetime.datetime.fromisoformat(time_iso + offset)
    time_epoch = tstr.timestamp()
    return time_epoch
        
def time_iso_from_epoch(time_epoch):
    if time_epoch is None:
        return None
    time_iso = datetime.datetime.utcfromtimestamp(time_epoch)
    return time_iso

def get_car(vvv):
    return list(map(lambda x: x[0], vvv))

def get_cdr(vvv):
    return list(map(lambda x: x[1], vvv))

def check_sorted(name, arr):
    top = arr[0]
    for val in arr:
        if val >= top:
            top = val
        else:
            print("warning, {} not sorted".format(name))
            return False
    return True

def time_format_example():
    print("iso time format = '2011-11-04T13:05:23'")

class weewx_calibrate(object):
    def usage(self):
        print("""usage: 
{0} [--version]
{0} [--debug]
iso time format = '2011-11-04T13:05:23'
columns = {1}
""".format(WEEWX_CALIBRATE_NAME, WEEWX_CALIBRATE_COLUMNS))
        pass
    def init_args(self, options):
        self.time1_str = "2022-09-01T00:00:00"
        self.time2_str = None
        self.time3_str = None
        self.time1 = time_epoch_from_iso(self.time1_str)
        self.time2 = time_epoch_from_iso(self.time2_str)
        self.time3 = time_epoch_from_iso(self.time3_str)
        self.skip_null = None
        self.column = 'inTemp'
        self.init_column()
        self.calval = None
        self.debug = False
        self.data_in_done = False
        self.fit_done = False
    def init_column(self):
        if not (self.column in WEEWX_CALIBRATE_COLUMNS):
            print("error:  bad column; reenter")
            self.usage()
            return False
        self.skip_null = "where dateTime is not null and {} is not null".format(self.column)
        return True
    def init_databases(self):
        # open databases atlas, wmod, green
        # fixme check file exists first
        self.con_atl = sqlite3.connect(WEEWX_CALIBRATE_DB_FNAME_ATL)
        self.con_wmo = sqlite3.connect(WEEWX_CALIBRATE_DB_FNAME_WMO)
        self.con_gre = sqlite3.connect(WEEWX_CALIBRATE_DB_FNAME_GRE)
    def get_columns(self):
        # find range of dateTime column
        cur_atl = self.con_atl.cursor()
        cur_wmo = self.con_wmo.cursor()
        cur_gre = self.con_gre.cursor()
        cmd = "select dateTime, {} from archive {}".format(self.column, self.skip_null)
        res_atl = cur_atl.execute(cmd)
        res_wmo = cur_wmo.execute(cmd)
        res_gre = cur_gre.execute(cmd)
        fff_atl = res_atl.fetchall()
        fff_wmo = res_wmo.fetchall()
        fff_gre = res_gre.fetchall()
        # for interpolation just use whole range
        self.itime_atl = get_car(fff_atl)
        self.itime_wmo = get_car(fff_wmo)
        self.itime_gre = get_car(fff_gre)
        self.icolumn_atl = get_cdr(fff_atl)
        self.icolumn_wmo = get_cdr(fff_wmo)
        self.icolumn_gre = get_cdr(fff_gre)
        check_sorted("dateTime atl", self.itime_atl)
        check_sorted("dateTime wmo", self.itime_wmo)
        check_sorted("dateTime gre", self.itime_gre)
        print("len(itime_atl)   = {}".format(len(self.itime_atl)))
        print("len(itime_wmo)   = {}".format(len(self.itime_wmo)))
        print("len(itime_gre)   = {}".format(len(self.itime_gre)))
        print("len(icolumn_atl) = {}".format(len(self.icolumn_atl)))
        print("len(icolumn_wmo) = {}".format(len(self.icolumn_wmo)))
        print("len(icolumn_gre) = {}".format(len(self.icolumn_gre)))
        err = False
        if (len(self.itime_atl) != len(self.icolumn_atl)):
            print("error:  len(itime_atl) != len(icolumn_atl)")
            err = True
        if (len(self.itime_wmo) != len(self.icolumn_wmo)):
            print("error:  len(itime_wmo) != len(icolumn_wmo)")
            err = True
        if (len(self.itime_gre) != len(self.icolumn_gre)):
            print("error:  len(itime_gre) != len(icolumn_gre)")
            err = True
        if (err):
            return False
        return True

    def init_time(self):
        [min_atl, max_atl] = [min(self.itime_atl), max(self.itime_atl)]
        [min_wmo, max_wmo] = [min(self.itime_wmo), max(self.itime_wmo)]
        [min_gre, max_gre] = [min(self.itime_gre), max(self.itime_gre)]
        [min_atl_str, max_atl_str] = [time_iso_from_epoch(min_atl), time_iso_from_epoch(max_atl)]
        [min_wmo_str, max_wmo_str] = [time_iso_from_epoch(min_wmo), time_iso_from_epoch(max_wmo)]
        [min_gre_str, max_gre_str] = [time_iso_from_epoch(min_gre), time_iso_from_epoch(max_gre)]
        # represent as epoch int and print
        print("time min_atl = {}   max_atl = {}".format(min_atl, max_atl))
        print("time min_wmo = {}   max_wmo = {}".format(min_wmo, max_wmo))
        print("time min_gre = {}   max_gre = {}".format(min_gre, max_gre))
        # represent as iso string and print
        print("time min_atl = {}   max_atl = {}".format(min_atl_str, max_atl_str))
        print("time min_wmo = {}   max_wmo = {}".format(min_wmo_str, max_wmo_str))
        print("time min_gre = {}   max_gre = {}".format(min_gre_str, max_gre_str))
        # if not set already find default time1 and time2
        # for default (timea is None) set timea = max[weatherstations](min[rows](dateTime))
        # for default (timeb is None) set timeb = min[weatherstations](max[rows](dateTime))
        self.timea = max(min_atl, min_wmo, min_gre)
        self.timeb = min(max_atl, max_wmo, max_gre)
        self.timea_str = time_iso_from_epoch(self.timea)
        self.timeb_str = time_iso_from_epoch(self.timeb)
        # represent as epoch int and print
        print("timea = {}  timeb = {}".format(self.timea, self.timeb))
        # represent as iso string and print
        print("timea = {}  timeb = {}".format(self.timea_str, self.timeb_str))
        try:
            if self.time1 == 0:
                print("time1 zero; updating time1")
                self.time1 = self.timea
                self.time1_str = time_iso_from_epoch(self.time1)
            if self.time1 < self.timea:
                print("time1 out of range; updating time1")
                self.time1 = self.timea
                self.time1_str = time_iso_from_epoch(self.time1)
        except Exception as e:
            print("error:  bad time format time1; reenter")
            time_format_example()
            return False
        try:
            if self.time2 == 0:
                print("time2 zero; updating time2")
                self.time2 = self.timeb
                self.time2_str = time_iso_from_epoch(self.time2)
            if self.time2 > self.timeb:
                print("time2 out of range; updating time2")
                self.time2 = self.timeb
                self.time2_str = time_iso_from_epoch(self.time2)
        except Exception as e:
            print("error:  bad time format time2; reenter")
            time_format_example()
            return False
        try:
            if self.time3 == 0:
                print("time3 zero; updating time3")
                self.time3 = self.timeb
                self.time3_str = time_iso_from_epoch(self.time3)
            if self.time3 > self.timeb:
                print("time3 out of range; updating time3")
                self.time3 = self.timeb
                self.time3_str = time_iso_from_epoch(self.time3)
        except Exception as e:
            print("error:  bad time format time3; reenter")
            time_format_example()
            return False
            # datetime.datetime.fromtimestamp(epochtime)
            # raises OverflowError, OSError
        print("timea  = {}\t{}".format(self.timea, self.timea_str))
        print("timeb  = {}\t{}".format(self.timeb, self.timeb_str))
        print("time 1 = {}\t{}".format(self.time1, self.time1_str))
        print("time 2 = {}\t{}".format(self.time2, self.time2_str))
        print("time 3 = {}\t{}".format(self.time3, self.time3_str))
        print("calval = {}".format(self.calval))
        # validate time3 within range
        if (self.time3 < self.time1) or (self.time3 > self.time2):
            print("error:  bad time format; (time1 <= time3 <= time2); reenter")
            time_format_example()
            return False
        return True
                
    def time_window(self, ttt, ccc, TTT, CCC, tl, th):
        count = 0
        for jjj in range(0, len(ttt)):
            xxx = ttt[jjj]
            yyy = ccc[jjj]
            if (xxx < tl) or (xxx > th):
                continue
            TTT.append(xxx)
            CCC.append(yyy)
            count = count + 1
    def time_window_columns(self):
        self.time_atl   = []
        self.time_wmo   = []
        self.time_gre   = []
        self.column_atl = []
        self.column_wmo = []
        self.column_gre = []
        self.time_window(self.itime_atl,
                         self.icolumn_atl,
                         self.time_atl,
                         self.column_atl,
                         self.time1,
                         self.time2)
        self.time_window(self.itime_wmo,
                         self.icolumn_wmo,
                         self.time_wmo,
                         self.column_wmo,
                         self.time1,
                         self.time2)
        self.time_window(self.itime_gre,
                         self.icolumn_gre,
                         self.time_gre,
                         self.column_gre,
                         self.time1,
                         self.time2)
        print("len(time_atl)   = {}".format(len(self.time_atl)))
        print("len(time_wmo)   = {}".format(len(self.time_wmo)))
        print("len(time_gre)   = {}".format(len(self.time_gre)))
        print("len(column_atl) = {}".format(len(self.column_atl)))
        print("len(column_wmo) = {}".format(len(self.column_wmo)))
        print("len(column_gre) = {}".format(len(self.column_gre)))
    def __init__(self, options):
        self.options = options
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.con_atl.close()
        self.con_wmo.close()
        self.con_gre.close()
    def interpolate_columns(self):
        self.interp_func_atl = interp1d(self.itime_atl, self.icolumn_atl)
        self.interp_func_wmo = interp1d(self.itime_wmo, self.icolumn_wmo)
        self.interp_func_gre = interp1d(self.itime_gre, self.icolumn_gre)
    def least_squares(self, ttt, RRR, rrr, aaa, bbb):
        # rrr is reference
        # let calibrated aaa == original aaa + aaas
        # let calibrated bbb == original bbb + bbbs
        # find calibration constant intercept aaas and bbbs such that
        # avg(original aaa + aaas) = avg(rrr)
        # avg(original bbb + bbbs) = avg(rrr)
        tttn = len(ttt)
        RRRn = len(RRR)
        if tttn != RRRn:
            print("error: ")
            print("tttn = {}".format(tttn))
            print("RRRn = {}".format(RRRn))
            exit(1)
        aaas = 0.0
        bbbs = 0.0
        for jjj in range(0, tttn):
            tj = ttt[jjj]
            rj = RRR[jjj]
            aj = aaa(tj)
            bj = bbb(tj)
            aaas = aaas + rj - aj
            bbbs = bbbs + rj - bj
        aaas = aaas / float(tttn)
        bbbs = bbbs / float(tttn)
        self.atl_avg = 0.0
        self.wmo_avg = aaas
        self.gre_avg = bbbs
        print("atl_avg = {}".format(self.atl_avg))
        print("wmo_avg = {}".format(self.wmo_avg))
        print("gre_avg = {}".format(self.gre_avg))
        # add the same constant (calval - rrval) to each 
        if (self.time3 is None):
            print("error:  can not compute fit; time3 not defined;")
            return False
        rrrval = self.interp_func_atl(self.time3)
        if (self.calval is None):
            print("warning:  no calval; fit assuming atl(time3) is correct.")
            self.calval = rrrval
        self.atl_cal = self.atl_avg + self.calval - rrrval
        self.wmo_cal = self.wmo_avg + self.calval - rrrval
        self.gre_cal = self.gre_avg + self.calval - rrrval
        print()
        print("+ atl_avg = {}".format(self.atl_avg))
        print("+ calval  = {}".format(self.calval))
        print("- rrrval  = {}".format(-rrrval))
        print("= atl_cal = {}".format(self.atl_avg))
        print()
        print("+ wmo_avg = {}".format(self.wmo_avg))
        print("+ calval  = {}".format(self.calval))
        print("- rrrval  = {}".format(-rrrval))
        print("= wmo_cal = {}".format(self.wmo_avg))
        print()
        print("+ gre_avg = {}".format(self.gre_avg))
        print("+ calval  = {}".format(self.calval))
        print("- rrrval  = {}".format(-rrrval))
        print("= gre_cal = {}".format(self.gre_avg))
        fptr = open("calibration_result.txt", "a")
        time_now = datetime.datetime.now()
        fptr.write("\n")
        fptr.write("NEW CALIBRATION ----------\n")
        fptr.write("column    = {}\n".format(self.column))
        fptr.write("time      = {}\n".format(time_now))
        fptr.write("+ atl_avg = {}\n".format(self.atl_avg))
        fptr.write("+ calval  = {}\n".format(self.calval))
        fptr.write("- rrrval  = {}\n".format(-rrrval))
        fptr.write("= atl_cal = {}\n".format(self.atl_avg))
        fptr.write("\n")
        fptr.write("+ wmo_avg = {}\n".format(self.wmo_avg))
        fptr.write("+ calval  = {}\n".format(self.calval))
        fptr.write("- rrrval  = {}\n".format(-rrrval))
        fptr.write("= wmo_cal = {}\n".format(self.wmo_avg))
        fptr.write("\n")
        fptr.write("+ gre_avg = {}\n".format(self.gre_avg))
        fptr.write("+ calval  = {}\n".format(self.calval))
        fptr.write("- rrrval  = {}\n".format(-rrrval))
        fptr.write("= gre_cal = {}\n".format(self.gre_avg))
        fptr.write("\n")
        fptr.close()
        return True

    def update_databases(self, restrict_to_window):
        if restrict_to_window:
            com0 = "update archive set {}={}+{} {} and dateTime >= {} and dateTime <= {}"
        else:
            com0 = "update archive set {}={}+{} {}"
        col = self.column
        com_atl = com0.format(col, col, self.atl_cal, self.skip_null, self.time1, self.time2)
        com_wmo = com0.format(col, col, self.wmo_cal, self.skip_null, self.time1, self.time2)
        com_gre = com0.format(col, col, self.gre_cal, self.skip_null, self.time1, self.time2)
        cur_atl = self.con_atl.cursor()
        cur_wmo = self.con_wmo.cursor()
        cur_gre = self.con_gre.cursor()
        cur_atl.execute(com_atl)
        cur_wmo.execute(com_wmo)
        cur_gre.execute(com_gre)
        self.con_atl.commit()
        self.con_wmo.commit()
        self.con_gre.commit()
        print("updated databases")
    def plot(self):
        plt.plot(self.time_atl, self.column_atl, color='blue', linestyle='solid', label='ATLAS')
        plt.plot(self.time_wmo, self.column_wmo, color='red',   linestyle='solid', label='WMOD')
        plt.plot(self.time_gre, self.column_gre, color='green',  linestyle='solid', label='GREEN')
        plt.xlabel('epoch time')
        plt.ylabel('{}'.format(self.column))
        plt.title(self.column)
        plt.legend()
        plt.show()

    def update_time(self):
        res = self.init_time()
        if not res:
            print("error:  init_time fail")
            return False
        self.time_window_columns()
        return True
        
    def print_prompt(self):
        prompt = """
options =====================================

COLUMNS = {}

[t1s] time1_str    = {}
[t2s] time2_str    = {}
[t3s] time3_str    = {}
[t1]  time1 epoch  = {}
[t2]  time2_epoch  = {}
[t3]  time3_epoch  = {}
[c]   column       = {}
[v]   calval       = {}
[d]   debug        = {}
[i]   data_in_done = {}
[f]   fit_done     = {}
[p]   plot
[ur]  update databases, restrict to [time1,time2]
[ua]  update databases, all times
[q]   quit
[quit] quit

enter choice> """
        print(prompt.format(
            WEEWX_CALIBRATE_COLUMNS,
            self.time1_str,
            self.time2_str,
            self.time3_str,
            self.time1,
            self.time2,
            self.time3,
            self.column,
            self.calval,
            self.debug,
            self.data_in_done,
            self.fit_done), end='')

    def run(self):
        self.init_args(options)
        self.init_databases()
        self.choice = None
        while True:
            self.print_prompt()
            self.choice = input()
            if self.choice == 't1s':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time1_str> ", end='')
                try:
                    iii = input()
                    ttt = time_epoch_from_iso(iii)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time1 = ttt
                self.time1_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            elif self.choice == 't2s':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time2_str> ", end='')
                try:
                    iii = input()
                    ttt = time_epoch_from_iso(iii)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time2 = ttt
                self.time2_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            elif self.choice == 't3s':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time3_str> ", end='')
                try:
                    iii = input()
                    ttt = time_epoch_from_iso(iii)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time3 = ttt
                self.time3_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            if self.choice == 't1':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time1> ", end='')
                try:
                    ttt = int(input())
                    iii = time_iso_from_epoch(ttt)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time1 = ttt
                self.time1_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            elif self.choice == 't2':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time2> ", end='')
                try:
                    ttt = int(input())
                    iii = time_iso_from_epoch(ttt)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time2 = ttt
                self.time2_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            elif self.choice == 't3':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                print("enter time3> ", end='')
                try:
                    ttt = int(input())
                    iii = time_iso_from_epoch(ttt)
                except Exception as e:
                    print("exception {}".format(str(e)))
                    continue
                self.time3 = ttt
                self.time3_str = iii
                if self.data_in_done:
                    self.update_time()
                self.fit_done = False
                continue
            elif self.choice == 'c':
                print("enter column> ", end='')
                self.column = input()
                self.data_in_done = False
                self.fit_done = False
                res = self.init_column()
                continue
            elif self.choice == 'v':
                print("enter calval> ", end='')
                self.calval = float(input())
                self.fit_done = False
                continue
            elif self.choice == 'd':
                self.debug = not self.debug
            elif self.choice == 'i':
                res = self.get_columns()
                if not res:
                    print("error:  get_columns fail")
                    continue
                self.data_in_done = True
                self.update_time()
            elif self.choice == 'f':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                self.interpolate_columns()
                # fixme option to choose reference other than atlas
                res = self.least_squares(self.time_atl,
                                         self.column_atl,
                                         self.interp_func_atl,
                                         self.interp_func_wmo,
                                         self.interp_func_gre)
                if not res:
                    print("error:  least_squares fail")
                    continue
                self.fit_done = True
                continue
            elif self.choice == 'p':
                if not self.data_in_done:
                    print("error:  require data_in_done")
                    continue
                self.plot()
                continue
            elif self.choice == 'ur':
                if not self.fit_done:
                    print("error must fit first")
                    continue
                restrict_to_window = True
                self.update_databases(restrict_to_window)
                self.data_in_done = False
                self.fit_done = False
                continue
            elif self.choice == 'ua':
                if not self.fit_done:
                    print("error must fit first")
                    continue
                restrict_to_window = False
                self.update_databases(restrict_to_window)
                self.data_in_done = False
                self.fit_done = False
                continue
            elif self.choice == 'q':
                break
            elif self.choice == 'quit':
                break
            
if __name__ == '__main__':
    import optparse
    usage = """%prog [options] [--help]"""
    parser = optparse.OptionParser(usage=usage)
    parser.add_option('--version', dest='version', action='store_true',
                      help='display version')
    parser.add_option('--debug', dest='debug', action='store_true',
                      help='provide additional debug output',
                      default=WEEWX_CALIBRATE_DEBUG)
    (options, args) = parser.parse_args()

    if options.version:
        print("weewx_calibrate version {}".format(WEEWX_CALIBRATE_VERSION))
        exit(0)

    if options.debug:
        WEEWX_CALIBRATE_DEBUG = True

    with weewx_calibrate(options) as wcal:
        wcal.run()

# eee eof
