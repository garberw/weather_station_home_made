#!/usr/bin/env python
"""
gui for displaying weewx data; more flexible about date ranges than weewx web interface;
also allows for calibration; copies database and works only on copy;
"""

import signal
import argparse
import logging
import logging.handlers
import json
import pathlib
import tkinter as tk
import tkinter.ttk as ttk
from tkinter import Menu
from tkinter import messagebox
import sys
import time
import datetime
from datetime import timezone
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.backend_bases import key_press_handler
from matplotlib.figure import Figure
import matplotlib.dates as mdate
import numpy as np
import sqlite3

SERVICE_VERSION = 0.0
DEFAULT_DEBUG = 1
DEFAULT_CONFIG = '~/.config/wg-weather-gui/wg-weather-gui.conf'
DEFAULT_SIZE_WIDTH_MIN = 300
DEFAULT_SIZE_HEIGHT_MIN = 300
AVAILABLE_MAIN_MODES = [
    'mode_graphs',
    'mode_settings',
    'mode_calibration',
    'mode_single_graph',
]
AVAILABLE_WEATHER_STATIONS = {
#   abbrev name      abbrev filename                         line color 
    'AT':  ['Atlas', 'AT',  '/var/lib/weewx/junk.atlas.sdb', (0.0, 0.0, 1.0)],
    'GR':  ['Green', 'GR',  '/var/lib/weewx/junk.green.sdb', (0.0, 1.0, 0.0)],
    'WM':  ['Wmod',  'WM',  '/var/lib/weewx/junk.wmod.sdb' , (1.0, 0.0, 0.0)],
}
# a.k.a. ordinates
AVAILABLE_DATA_TYPES = [
    'windSpeed'      ,
    'windDir'        ,
    'outTemp'        ,
    'outHumidity'    ,
    'radiation'      ,
    'UV'             ,
    'rain_total'     ,
    'inTemp'         ,
    'inHumidity'     ,
    'pressure'       ,
    'gas_resistance' ,
    'iaq'            ,
    'co2'            ,
]


log = logging.getLogger(__name__)

# tkinter objects
root_window = None
root_frame = {}
root_menu = None
root_cvar = {
    'graph_table': None,
    'time_limit1': None,
    'time_limit2': None,
}
resizable_canvas = {}

# the data in memory
database_struct = None

# the settings data structure
fname_config = DEFAULT_CONFIG
settings = {
    'geometry': '300x300+0+0',
    'root_mode': 'mode_settings',
    'redraw_pending_graphs_all': True,
    'redraw_pending_graphs_times': True,
    'redraw_pending_single_graph_all': True,
    'redraw_pending_single_graph_times': True,
    'exit_pending': False,
    'ordinates': None,
    'bin_width': None,
    'graph_table':  None,
    'time_limit1': None,
    'time_limit2': None,
}


def get_timezone():
    return datetime.datetime.now().astimezone().tzinfo


def set_btn_load_database_red(all_not_times):
    global root_window
    btn = root_window.get_ancestor(['frm_graphs', 'frm_graphs_control', 'btn_load_database'])
    if all_not_times:
        settings['redraw_pending_graphs_all'] = True
        settings['redraw_pending_single_graph_all'] = True
    else:
        settings['redraw_pending_graphs_times'] = True
        settings['redraw_pending_single_graph_times'] = True
    if btn:
        btn.configure(bg = '#FF7777')


def set_btn_load_database_green():
    global root_window
    btn = root_window.get_ancestor(['frm_graphs', 'frm_graphs_control', 'btn_load_database'])
    settings['redraw_pending_graphs_all'] = True
    settings['redraw_pending_single_graph_all'] = True
    settings['redraw_pending_graphs_times'] = True
    settings['redraw_pending_single_graph_times'] = True
    if btn:
        btn.configure(bg = '#77FF77')


def color_intensity(color, intensity):
    return (color[0] * intensity, color[1] * intensity, color[2] * intensity)

class ComboboxWithRowCol(ttk.Combobox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    def set_row_col(self, is_row):
        self.is_row = is_row


class ListboxWithRowCol(tk.Listbox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    def set_row_col(self, rrr, ccc):
        self.rrr = rrr
        self.ccc = ccc


WF_PARENT      = 0
WF_NAME        = 1
WF_GRIDROW     = 2
WF_GRIDCOL     = 3
WF_STICKY      = 4
WF_WIDGET      = 5
WF_GRIDROWSPAN = 6
WF_GRIDCOLSPAN = 7

class WeatherGridManager:
    def __init__(self):
        self.my_child = {}
        self.grid_used_row = []
        self.grid_used_col = []
        self.grid_weight_row = []
        self.grid_weight_col = []
    def register_child(self, name, gridrow, gridcol, sticky, widget,
                       gridrowspan = 1, gridcolspan = 1):
        # name is virtual
        parent = self.class_name()
        # print(f'register_child from class parent = {parent} child = {name}')
        self.my_child[name] = (parent, name, gridrow, gridcol, sticky, widget,
                               gridrowspan, gridcolspan)
        if gridcol not in self.grid_used_col:
            self.grid_used_col.append(gridcol)
        if gridrow not in self.grid_used_row:
            self.grid_used_row.append(gridrow)
    def setup_grid(self, weight_row, weight_col):
        # there should be at least one child
        if not self.grid_used_row:
            log.error('no child found')
            return
        if not self.grid_used_col:
            log.error('no child found')
            return
        # check there is a weight for each row and col used
        max_row = max(self.grid_used_row)
        max_col = max(self.grid_used_col)
        if not (max_row <= len(weight_row)):
            log.error('weight_row missing')
        if not (max_col <= len(weight_col)):
            log.error('weight_col missing')
        for rrr, weight in enumerate(weight_row):
            self.rowconfigure(rrr, weight=weight)
        for ccc, weight in enumerate(weight_col):
            self.columnconfigure(ccc, weight=weight)
        for name, child in self.my_child.items():
            (parent, name1, gridrow, gridcol, sticky, widget, gridrowspan, gridcolspan) = child
            print(f'grid {name=}')
            # print(f'{str(child)}')
            widget.grid(row = gridrow,
                        column = gridcol,
                        sticky = sticky,
                        rowspan = gridrowspan,
                        columnspan = gridcolspan)
    def delete_all_children(self, class_name):
        for name, child in self.my_child.items():
            (parent, name1, gridrow, gridcol, sticky, widget, gridrowspan, gridcolspan) = child
            # print(f'delete_all_children class parent = {parent} child = {name}')
            widget.destroy()
        self.my_child.clear()
    def delete_child(self, class_name, name):
        if name not in self.my_child.keys():
            parent = self.class_name()
            log.error(f'delete_child class parent = {parent} child = {name} NOT FOUND')
            return
        child = self.my_child[name]
        (parent, name1, gridrow, gridcol, sticky, widget, gridrowspan, gridcolspan) = child
        # print(f'delete_child class parent = {parent} child = {name}')
        widget.destroy()
        del self.my_child[name]
            

class WeatherFrame(tk.Frame, WeatherGridManager):
    def __init__(self, *args, **kwargs):
        WeatherGridManager.__init__(self)
        tk.Frame.__init__(self, *args, **kwargs)


class WeatherCanvas(tk.Canvas, WeatherGridManager):
    def __init__(self, *args, **kwargs):
        WeatherGridManager.__init__(self)
        tk.Canvas.__init__(self, *args, **kwargs)


class WeatherWindow(tk.Tk, WeatherGridManager):
    def __init__(self, *args, **kwargs):
        WeatherGridManager.__init__(self)
        tk.Tk.__init__(self, *args, **kwargs)


class FrameOrdinatesChoice(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, available_ordinates, indices, rrr, ccc, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # frm_rrr_ccc = tk.Frame(master = master, relief = tk.RAISED, borderwidth = 1)

        self.bin_width_lookup = {
            '0'       : 0,
            '15 min'  : 60 * 15,
            '30 min'  : 60 * 30,
            '45 min'  : 60 * 45,
            '60 min'  : 60 * 60,
            '5/4 hr'  : 3600 * 5 / 4,
            '6/4 hr'  : 3600 * 6 / 4,
            '7/4 hr'  : 3600 * 7 / 4,
            '2   hr'  : 3600 * 2,
            '3   hr'  : 3600 * 3,
            '6   hr'  : 3600 * 6,
            '12  hr'  : 3600 * 12,
            '24  hr'  : 3600 * 24,
            '1 week'  : 3600 * 24 * 7,
            '1 month' : 3600 * 24 * 30,
        }
        
        msg = f'row={rrr}, col={ccc}  bin width (0=none)'
        widget = tk.Label(master = self, text = msg)
        self.register_child(name = 'lbl_available_data_types2',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = widget)

        # the control variable for bin width
        self.cvar_bin = tk.StringVar()

        # bin_width = the previous value or zero if None
        bin_width = settings['bin_width'][rrr][ccc] if settings['bin_width'][rrr][ccc] else 0
        # look up string version
        found = False
        for key, value in self.bin_width_lookup.items():
            if bin_width == value:
                self.cvar_bin.set(key)
                found = True
                break
        if not found:
            try:
                key = int(bin_width)
            except ValueError:
                print(f'error; bad settings["bin_width"][{rrr=}][{ccc=}]={bin_width}')
                key = 0
            self.cvar_bin.set(key)

        # the bin width combobox
        self.row = rrr
        self.col = ccc
        list1 = list(self.bin_width_lookup.keys())
        widget = ComboboxWithRowCol(master = self,
                                    values = list1,
                                    width = 8,
                                    textvariable = self.cvar_bin)
        widget.set_row_col(is_row = True)
        key = self.cvar_bin.get()
        idx = None
        for jjj, key2 in enumerate(self.bin_width_lookup.keys()):
            if key == key2:
                widget.current(jjj)
        widget.bind('<<ComboboxSelected>>', self.handle_bin_width)
        self.register_child(name = 'cmb_bin_width',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = widget)

        # the control variable for ordinates
        cvar = tk.StringVar()
        cvar.set(available_ordinates)
    
        # the listbox
        widget = ListboxWithRowCol(master = self,
                                   listvariable = cvar,
                                   selectmode = tk.MULTIPLE,
                                   exportselection = False) 
        widget.set_row_col(rrr, ccc)
        self.register_child(name = 'lst_selected_ordinates',
                            gridrow = 1,
                            gridcol = 0,
                            gridcolspan = 2,
                            sticky = 'nsew',
                            widget = widget)

        for iii in indices:
            widget.selection_set(iii)
            widget.bind('<<ListboxSelect>>', self.handle_update_ordinates)

        self.setup_grid([0, 1], [1])

    def handle_bin_width(self, event):
        global settings
        global database_struct
        print('handle_bin_width')
        www = event.widget
        rrr = self.row
        ccc = self.col
        print(f'{rrr=} {ccc=}')
        key = self.cvar_bin.get()
        # either the user types in an integer or it is found in bin_width_lookup
        try:
            bin_width = int(key)
        except ValueError:
            try:
                bin_width = self.bin_width_lookup[key]
            except ValueError:
                print(f'error; bad cvar {key=} {rrr=} {ccc=} setting bin_width = 0')
                bin_width = 0
                self.cvar_bin.set(value)
        settings['bin_width'][rrr][ccc] = bin_width
        # redo bin average
        table = database_struct['table']
        matplotlib_graph = table[rrr][ccc]
        curves = matplotlib_graph['curves']
        for curve_name, curve_data in curves.items():
            xxx     = curve_data[3]
            yyy     = curve_data[4]
            xxx_bin, yyy_bin = fill_bins(xxx, yyy, bin_width)
            curve_data[5] = xxx_bin
            curve_data[6] = yyy_bin
        # draw only individual graph
        redraw_graph(matplotlib_graph = matplotlib_graph)
        # do not change color of btn_load_database; already did redraw
        # set_btn_load_database_red(all_not_times = True)

    def handle_update_ordinates(self, event):
        global settings
        print('handle_update_ordinates')
        www = event.widget
        ttt = type(www)
        rrr = www.rrr
        ccc = www.ccc
        print(f'{rrr=} {ccc=}')
        indices = [ int(iii) for iii in www.curselection() ]
        selected_ordinates = [ www.get(int(iii)) for iii in www.curselection() ]
        print(f'{indices=}')
        print(f'{selected_ordinates=}')
        settings['ordinates'][rrr][ccc] = (indices, selected_ordinates)
        set_btn_load_database_red(all_not_times = True)


class FrameOrdinatesTable(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global AVAILABLE_DATA_TYPES
        global AVAILABLE_WEATHER_STATIONS
        global settings
        # frm_ordinates_table = tk.Frame(master = master, relief = tk.RAISED, borderwidth = 1)

        available_ordinates = []
        for abbrev, weather_station in AVAILABLE_WEATHER_STATIONS.items():
            for dt in AVAILABLE_DATA_TYPES:
                selection = weather_station[1] + '_' + dt
                available_ordinates.append(selection)
        graph_rows = settings['graph_table']['rows_val']
        graph_cols = settings['graph_table']['cols_val']

        # expand bin_width
        if 'bin_width' not in settings.keys() or settings['bin_width'] is None:
            settings['bin_width'] = {}
        for rrr in range(graph_rows):
            # expand settings['bin_width']
            if rrr not in settings['bin_width']:
                settings['bin_width'][rrr] = {}
            for ccc in range(graph_cols):
                if ccc not in settings['bin_width'][rrr]:
                    settings['bin_width'][rrr][ccc] = 0

        # expand ordinates
        if 'ordinates' not in settings.keys() or settings['ordinates'] is None:
            settings['ordinates'] = {}
        for rrr in range(graph_rows):
            # expand settings['ordinates']
            if rrr not in settings['ordinates']:
                settings['ordinates'][rrr] = {}
            for ccc in range(graph_cols):
                if ccc not in settings['ordinates'][rrr]:
                    # create new ordinate
                    indices = []
                    selected_ordinates = []
                    settings['ordinates'][rrr][ccc] = (indices, selected_ordinates)
                else:
                    # extract existing ordinate selection listbox
                    indices = settings['ordinates'][rrr][ccc][0]
                    selected_ordinates = settings['ordinates'][rrr][ccc][1]
                widget = FrameOrdinatesChoice(available_ordinates = available_ordinates,
                                              indices = indices,
                                              rrr = rrr,
                                              ccc = ccc,
                                              master = self)
                self.register_child(name = f'frm_{rrr}_{ccc}',
                                    gridrow = rrr,
                                    gridcol = ccc,
                                    sticky = 'nsew',
                                    widget = widget)
        weight_row = [ 1 for rrr in range(graph_rows) ]
        weight_col = [ 1 for ccc in range(graph_cols) ]
        self.setup_grid(weight_row, weight_col)


class FrameOrdinatesTableControl(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global settings
        global root_cvar
        # frm_ordinates_table_control = tk.Frame(master = master)
        cvar_rows = root_cvar['graph_table']['rows']
        cvar_cols = root_cvar['graph_table']['cols']

        msg = 'select dimensions of graph table    '
        self.register_child(name = 'lbl_instructions1',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = msg) )

        # the row label
        self.register_child(name = 'lbl_graph_row',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'graph rows') )
    
        # the row combobox
        list1 = list(range(1, 20))
        print(f'frame_settings_for_graph {cvar_rows.get()=}')
        widget = ComboboxWithRowCol(master = self,
                                    values = list1,
                                    width = 2,
                                    textvariable = cvar_rows)
        widget.set_row_col(is_row = True)
        widget.current(cvar_rows.get() - 1)
        widget.bind('<<ComboboxSelected>>', self.handle_ordinates_table_control)
        self.register_child(name = 'cmb_graph_row',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        # the column label
        self.register_child(name = 'lbl_graph_col',
                            gridrow = 1,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'graph cols') )
    
        # the column combobox
        list1 = list(range(1, 20))
        print(f'frame_settings_for_graph {cvar_cols.get()=}')
        widget = ComboboxWithRowCol(master = self,
                                    values = list1,
                                    width = 2,
                                    textvariable = cvar_cols)
        widget.set_row_col(is_row = False)
        widget.current(cvar_cols.get() - 1)
        widget.bind('<<ComboboxSelected>>', self.handle_ordinates_table_control)
        self.register_child(name = 'cmb_graph_col',
                            gridrow = 1,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([0, 0], [0, 0, 0])

    def handle_ordinates_table_control(self, event):
        global settings
        global root_cvar
        www = event.widget
        if www.is_row:
            settings['graph_table']['rows_val'] = root_cvar['graph_table']['rows'].get()
        else:
            settings['graph_table']['cols_val'] = root_cvar['graph_table']['cols'].get()
        rrr = settings['graph_table']['rows_val']
        ccc = settings['graph_table']['cols_val']
        print(f'handle_ordinates_table_control settings["graph_table"]["rows_val"]={rrr}')
        print(f'handle_ordinates_table_control settings["graph_table"]["cols_val"]={ccc}')
        set_btn_load_database_red(all_not_times = True)


class FrameSingleGraphControl(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        """very similar to frame_ordinates_table_control; different control variables;"""
        super().__init__(*args, **kwargs)
        global settings
        global root_cvar
        graph_rows = settings['graph_table']['rows_val']
        graph_cols = settings['graph_table']['cols_val']

        # frm_single_graph_control = tk.Frame(master = master)

        cvar_single_graph_row = root_cvar['graph_table']['single_graph_row']
        cvar_single_graph_col = root_cvar['graph_table']['single_graph_col']

        # the label
        msg = 'select table coords of single graph to plot '
        self.register_child(name = 'lbl_instructions1',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = msg) )

        # the row label
        self.register_child(name = 'lbl_graph_row',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'single graph row') )
    
        # the row combobox
        list1 = list(range(0, graph_rows))
        print(f'frame_single_graph_control {cvar_single_graph_row.get()=}')
        widget = ComboboxWithRowCol(master = self,
                                    values = list1,
                                    width = 2,
                                    textvariable = cvar_single_graph_row)
        widget.set_row_col(is_row = True)
        widget.current(cvar_single_graph_row.get())
        widget.bind('<<ComboboxSelected>>', self.handle_single_graph_control)
        self.register_child(name = 'cmb_graph_row',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        # the column label
        self.register_child(name = 'lbl_graph_col',
                            gridrow = 1,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'single graph col') )
    
        # the column combobox
        list1 = list(range(0, graph_cols))
        print(f'frame_single_graph_control {cvar_single_graph_col.get()=}')
        widget = ComboboxWithRowCol(master = self,
                                    values = list1,
                                    width = 2,
                                    textvariable = cvar_single_graph_col)
        widget.set_row_col(is_row = False)
        widget.current(cvar_single_graph_col.get())
        widget.bind('<<ComboboxSelected>>', self.handle_single_graph_control)
        self.register_child(name = 'cmb_graph_col',
                            gridrow = 1,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([0, 0], [0, 0, 0])

    def handle_single_graph_control(self, event):
        global settings
        global root_cvar
        www = event.widget
        if www.is_row:
            settings['graph_table']['single_graph_row_val'] = \
                root_cvar['graph_table']['single_graph_row'].get()
        else:
            settings['graph_table']['single_graph_col_val'] = \
                root_cvar['graph_table']['single_graph_col'].get()
        rrr = settings['graph_table']['single_graph_row_val']
        ccc = settings['graph_table']['single_graph_col_val']
        print(f'handle_ordinates_table_control settings["graph_table"]["single_graph_row_val"]={rrr}')
        print(f'handle_ordinates_table_control settings["graph_table"]["single_graph_col_val"]={ccc}')
        settings['redraw_pending_single_graph_all'] = True


class FrameMainOptions(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # frm_main_options = tk.Frame(master = master)

        widget = tk.Button(master = self, text = 'graphs')
        widget.bind('<Button-1>', self.handle_click_graphs)
        self.register_child(name = 'btn_graphs',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = widget)

        widget = tk.Button(master = self, text = 'settings')
        widget.bind('<Button-1>', self.handle_click_settings)
        self.register_child(name = 'btn_settings',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = widget)

        widget = tk.Button(master = self, text = 'calibration')
        widget.bind('<Button-1>', self.handle_click_calibration)
        self.register_child(name = 'btn_calibration',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        widget = tk.Button(master = self, text = 'single_graph')
        widget.bind('<Button-1>', self.handle_click_single_graph)
        self.register_child(name = 'btn_single_graph',
                            gridrow = 0,
                            gridcol = 3,
                            sticky = 'nsew',
                            widget = widget)

        widget = tk.Button(master = self, text = 'exit')
        widget.bind('<Button-1>', self.handle_click_exit)
        self.register_child(name = 'btn_exit',
                            gridrow = 0,
                            gridcol = 4,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([0], [0, 0, 0, 0, 0])

    def handle_click_graphs(self, event):
        global settings
        print("handle_click_graphs")
        settings['root_mode'] = 'mode_graphs'

    def handle_click_settings(self, event):
        global settings
        print("handle_click_settings")
        settings['root_mode'] = 'mode_settings'

    def handle_click_calibration(self, event):
        global settings
        print("handle_click_calibration")
        settings['root_mode'] = 'mode_calibration'

    def handle_click_single_graph(self, event):
        global settings
        print("handle_click_single_graph")
        settings['root_mode'] = 'mode_single_graph'

    def handle_click_exit(self, event):
        global settings
        global root_window
        print("handle_click_exit")
        settings['exit_pending'] = True
        # fixme just kill it
        root_window_close()


class FrameSettings(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global settings
        global root_cvar

        # manage the settings['graph_table'] data structure
        if settings['graph_table'] is None:
            # default
            # fixme
            #        settings['graph_table'] = { 'rows_val': 4,
            #                                   'cols_val': 5,
            #                                   'single_graph_row_val': 1,
            #                                   'single_graph_col_val': 1, }
            settings['graph_table'] = {'rows_val': 1,
                                       'cols_val': 1,
                                       'single_graph_row_val': 0,
                                       'single_graph_col_val': 0, }
        if root_cvar['graph_table'] is None:
            root_cvar['graph_table'] = { 'rows': tk.IntVar(),
                                         'cols': tk.IntVar(),
                                         'single_graph_row': tk.IntVar(),
                                         'single_graph_col': tk.IntVar(), }
        cvar_rows             = root_cvar['graph_table']['rows']
        cvar_cols             = root_cvar['graph_table']['cols']
        cvar_single_graph_row = root_cvar['graph_table']['single_graph_row']
        cvar_single_graph_col = root_cvar['graph_table']['single_graph_col']
        cvar_rows.set            (settings['graph_table']['rows_val'])
        cvar_cols.set            (settings['graph_table']['cols_val'])
        cvar_single_graph_row.set(settings['graph_table']['single_graph_row_val'])
        cvar_single_graph_col.set(settings['graph_table']['single_graph_col_val'])

        # frm_settings = tk.Frame(master = master)

        self.register_child(name = 'frm_main_options',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameMainOptions(master = self) )

        self.register_child(name = 'frm_ordinates_table_control',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget  = FrameOrdinatesTableControl(master = self) )

        self.register_child(name = 'frm_single_graph_control',
                            gridrow = 1,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = FrameSingleGraphControl(master = self) )

        widget = tk.Button(master = self, text = 'save and update graphs')
        widget.bind('<Button-1>', self.handle_save_ordinates)
        self.register_child(name = 'btn_save_ordinates',
                            gridrow = 1,
                            gridcol = 2,
                            sticky = 'ew',
                            widget = widget)
        
        msg = 'select ordinates for graph at'
        self.register_child(name = 'lbl_instructions2',
                            gridrow = 2,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = msg) )

        # the table of ordinate listboxes
        self.register_child(name = 'frm_all_graphs',
                            gridrow = 3,
                            gridcol = 0,
                            gridcolspan = 3,
                            sticky = 'nsew',
                            widget = FrameOrdinatesTable(master = self) )

        # fixme check columns
        self.setup_grid([0, 0, 0, 1], [1, 1, 1])

    def handle_save_ordinates(self):
        pass

class FrameDate(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, time_cvar, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global settings
        global root_cvar
        # frm_date = tk.Frame(master = master)

        cvar_year = time_cvar['year']
        current_year = datetime.date.today().year
        list1 = list(range(2021, current_year+1))
        widget = ttk.Combobox(master = self, values = list1, width = 4, textvariable = cvar_year)
        self.register_child(name = 'cmb_timestamp_year',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = widget)

        list1 = list(range(1, 12+1))
        cvar_month = time_cvar['month']
        widget = ttk.Combobox(master = self, values = list1, width = 2, textvariable = cvar_month)
        self.register_child(name = 'cmb_timestamp_month',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = widget)

        # fixme validate
        list1 = list(range(1, 31+1))
        cvar_day = time_cvar['day']
        widget = ttk.Combobox(master = self, values = list1, width = 2, textvariable = cvar_day)
        self.register_child(name = 'cmb_timestamp_day',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        list1 = list(range(0, 23+1))
        cvar_hour = time_cvar['hour']
        widget = ttk.Combobox(master = self, values = list1, width = 2, textvariable = cvar_hour)
        self.register_child(name = 'cmb_timestamp_hour',
                            gridrow = 0,
                            gridcol = 3,
                            sticky = 'nsew',
                            widget = widget)

        list1 = list(range(0, 59+1))
        cvar_minute = time_cvar['minute']
        widget = ttk.Combobox(master = self, values = list1, width = 2, textvariable = cvar_minute)
        self.register_child(name = 'cmb_timestamp_minute',
                            gridrow = 0,
                            gridcol = 4,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([1], [1, 1, 1, 1, 1])


class FrameGraphChooseDate(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, begin_end, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # (master, begin_end):
        global settings
        global root_cvar
        if begin_end == 'begin':
            if settings['time_limit1'] is None:
                # initialize the structure and default unix_epoch_val
                unix_epoch = time.time() - 3600 * 24
                settings['time_limit1'] = { 'unix_epoch_val': unix_epoch, }
            if root_cvar['time_limit1'] is None:
                # initialize the control variables
                root_cvar['time_limit1'] = { 'unix_epoch': tk.IntVar(),
                                             'year': tk.IntVar(),
                                             'month': tk.IntVar(),
                                             'day': tk.IntVar(),
                                             'hour': tk.IntVar(),
                                             'minute': tk.IntVar(), }
                # copy over unix_epoch_val and use it to fill in control variables
                root_cvar['time_limit1']['unix_epoch'].set(settings['time_limit1']['unix_epoch_val'])
                self.handle_epoch_update0('time_limit1')
            time_cvar = root_cvar['time_limit1']
            time_limit = settings['time_limit1']

        if begin_end == 'end':
            if settings['time_limit2'] is None:
                unix_epoch = time.time()
                settings['time_limit2'] = { 'unix_epoch_val': unix_epoch, }
            if root_cvar['time_limit2'] is None:
                root_cvar['time_limit2'] = { 'unix_epoch': tk.IntVar(),
                                             'year': tk.IntVar(),
                                             'month': tk.IntVar(),
                                             'day': tk.IntVar(),
                                             'hour': tk.IntVar(),
                                             'minute': tk.IntVar(), }
                root_cvar['time_limit2']['unix_epoch'].set(settings['time_limit2']['unix_epoch_val'])
                self.handle_epoch_update0('time_limit2')
            time_cvar = root_cvar['time_limit2']
            time_limit = settings['time_limit2']

        # frm_graph_choose_date = tk.Frame(master = master)

        self.register_child(name = 'lbl_timestamp',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'dateTime') )

        self.register_child(name = 'frame_date',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = FrameDate(master = self, time_cvar = time_cvar) )
        
        widget = tk.Button(master = self, text = 'Update')
        if begin_end == 'begin':
            widget.bind('<Button-1>', self.handle_timestamp_update1)
        if begin_end == 'end':
            widget.bind('<Button-1>', self.handle_timestamp_update2)
        self.register_child(name = 'btn_timestamp',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        self.register_child(name = 'lbl_unix_epoch',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = 'unix epoch') )

        cvar_epoch = time_cvar['unix_epoch']
        widget = tk.Entry(master = self,
                          width = 10,
                          exportselection = False,
                          textvariable = cvar_epoch)
        self.register_child(name = 'ent_unix_epoch',
                            gridrow = 1,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = widget)

        widget = tk.Button(master = self, text = 'Update')
        if begin_end == 'begin':
            widget.bind('<Button-1>', self.handle_epoch_update1)
        if begin_end == 'end':
            widget.bind('<Button-1>', self.handle_epoch_update2)
        self.register_child(name = 'btn_unix_epoch',
                            gridrow = 1,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([1, 1], [1, 1, 1])

    def handle_timestamp_update0(self, time_limit):
        global root_cvar
        global settings
        cv = root_cvar[time_limit]
        cv_year   = cv['year'  ].get()
        cv_month  = cv['month' ].get()
        cv_day    = cv['day'   ].get()
        cv_hour   = cv['hour'  ].get()
        cv_minute = cv['minute'].get()
        timezone = get_timezone()
        local_time = datetime.datetime(cv_year, cv_month, cv_day, cv_hour, cv_minute,
                                       tzinfo=timezone)
        unix_epoch = local_time.timestamp()
        settings[time_limit]['unix_epoch_val'] = unix_epoch
        cv['unix_epoch'].set(unix_epoch)
        # time_limit['local_time'] = local_time
        set_btn_load_database_red(all_not_times = False)

    def handle_timestamp_update1(self, event):
        global settings
        self.handle_timestamp_update0('time_limit1')

    def handle_timestamp_update2(self, event):
        global settings
        self.handle_timestamp_update0('time_limit2')

    def handle_epoch_update0(self, time_limit):
        global root_cvar
        global settings
        cv = root_cvar[time_limit]
        unix_epoch = cv['unix_epoch'].get()
        unix_epoch = unix_epoch - unix_epoch % 60
        timezone = get_timezone()
        local_time = datetime.datetime.fromtimestamp(unix_epoch, tz=timezone)
        cv['year'  ].set(local_time.year)
        cv['month' ].set(local_time.month)
        cv['day'   ].set(local_time.day)
        cv['hour'  ].set(local_time.hour)
        cv['minute'].set(local_time.minute)
        settings[time_limit]['unix_epoch_val'] = unix_epoch
        cv['unix_epoch'].set(unix_epoch)
    #    time_limit['local_time'] = local_time
        set_btn_load_database_red(all_not_times = False)

    def handle_epoch_update1(self, event):
        global settings
        self.handle_epoch_update0('time_limit1')


    def handle_epoch_update2(self, event):
        global settings
        self.handle_epoch_update0('time_limit2')

# order is;
# (A) redraw_pending_graphs_all or redraw_pending_single_graph_all;
# clear_database_struct;
# load_database;
# redraw_all;

# (B) redraw_pending_graphs_times or redraw_pending_single_graph_times;
# refresh_database_struct;
# load_database;
# redraw_times;

# keep existing row; col array including lines and data;
# delete ordinates that have been removed;
# keep rows and columns outside current span;
def refresh_database_struct():
    global database_struct
    global AVAILABLE_WEATHER_STATIONS
    global settings
    graph_rows = settings['graph_table']['rows_val']
    graph_cols = settings['graph_table']['cols_val']
    if database_struct is None:
        database_struct = {}
        databases = {}
        database_struct['databases'] = databases
        for abbrev, weather_station in AVAILABLE_WEATHER_STATIONS.items():
            name   = weather_station[0]
            abbrev = weather_station[1]
            fname  = weather_station[2]
            db = {}
            databases[abbrev] = db
            db['fname'] = fname
            db['con'] = sqlite3.connect(db['fname'])
            db['cur'] = db['con'].cursor()
        database_struct['table'] = {}
    table = database_struct['table']
    # fixme do not del existing rows and columns
    for rrr in range(graph_rows):
        if not (rrr in table.keys()):
            table[rrr] = {}
        for ccc in range(graph_cols):
            if not (ccc in table[rrr].keys()):
                table[rrr][ccc] = {}
            _, selected_ordinates = settings['ordinates'][rrr][ccc]
            matplotlib_graph = table[rrr][ccc]
            if not ('curves' in matplotlib_graph.keys()):
                matplotlib_graph['curves'] = {}
            curves = matplotlib_graph['curves']
            curves_keys = list(curves.keys())
            for curve_name in curves_keys:
                if not curve_name in selected_ordinates:
                    curve_data = curves[curve_name]
                    line = curve_data[8]
                    line.destroy()
                    del curves[curve_name]


# keep only rows_val x cols_val table of empty matplotlib_graph dictionaries;
def clear_database_struct():
    global database_struct
    global settings
    graph_rows = settings['graph_table']['rows_val']
    graph_cols = settings['graph_table']['cols_val']

    refresh_database_struct()

    table = database_struct['table']
    for rrr in range(graph_rows):
        for ccc in range(graph_cols):
            _, selected_ordinates = settings['ordinates'][rrr][ccc]
            matplotlib_graph = { 'curves': {} }
            table[rrr][ccc] = matplotlib_graph


def get_car(vvv):
    return list(map(lambda x: x[0], vvv))


def get_cdr(vvv):
    return list(map(lambda x: x[1], vvv))


def load_database():
    global database_struct
    global settings
    global AVAILABLE_WEATHER_STATIONS
    unix_epoch1 = settings['time_limit1']['unix_epoch_val']
    unix_epoch2 = settings['time_limit2']['unix_epoch_val']
    graph_rows = settings['graph_table']['rows_val']
    graph_cols = settings['graph_table']['cols_val']

    databases = database_struct['databases']
    table = database_struct['table']
    log.info('load database')
    for rrr in range(graph_rows):
        for ccc in range(graph_cols):
            _, selected_ordinates = settings['ordinates'][rrr][ccc]
            bin_width = settings['bin_width'][rrr][ccc]
            matplotlib_graph = table[rrr][ccc]
            curves = matplotlib_graph['curves']
            curves_keys = list(curves.keys())
            for curve_name1 in selected_ordinates:
                # abbreviation of which weather station
                abbrev = curve_name1[0:2]
                color = AVAILABLE_WEATHER_STATIONS[abbrev][3]
                # data type == selected ordinate without weather station prefix
                curve_name0 = curve_name1[3:]
                log.info(f'{rrr=} {ccc=} {curve_name1=} {abbrev=} {curve_name0=}')
                # extract the sqlite3 cursor for this weather station
                db = databases[abbrev]
                cur = db['cur']
                cmd = f'select dateTime, {curve_name0} from archive where {curve_name0} is not null and dateTime >= {unix_epoch1} and dateTime <= {unix_epoch2}'
                # fixme may want this
                # log.info(f'{cmd=}')
                res = list(cur.execute(cmd))
                xxx = get_car(res)
                yyy = get_cdr(res)
                if curve_name1 in curves_keys:
                    # this is a reload redraw_pending_..._times; keep lines;
                    curve_data = curves[curve_name1]
                    line = curve_data[8]
                else:
                    # this is a new load redraw_pending_..._all; redraw_all will generate lines;
                    line = None
                # fixme may want this
                # log.info(f'{len(res)=} {len(xxx)=} {len(yyy)=}')
#                for jjj, val in enumerate(res):
#                    log.info(f'{jjj=} {val=} {xxx[jjj]=} {yyy[jjj]=}')
                xxx_bin, yyy_bin = fill_bins(xxx, yyy, bin_width)
                curve_data = [rrr, ccc, curve_name1, xxx, yyy, xxx_bin, yyy_bin, color, line]
                curves[curve_name1] = curve_data
    set_btn_load_database_green()


def bin_index(ttt, xmin, xmax, bin_width):
    # e.g. 6 // 3 - 4 // 3 != (6 - 4) // 3;
    imin = int(xmin // bin_width)
    ittt = int(ttt  // bin_width)
    return ittt - imin


def fill_bins(xxx, yyy, bin_width):
    print('fill_bins input', '*' * 50)
    for jjj, (ttt, sss) in enumerate(zip(xxx, yyy)):
        print(f'{jjj=}, {ttt=}, {sss=}')
    if bin_width == 0:
        return xxx, yyy
    xmin = min(xxx)
    xmax = max(xxx)
    imin = bin_index(xmin, xmin, xmax, bin_width)
    imax = bin_index(xmax, xmin, xmax, bin_width)
    ilen = imax - imin + 1
    print(f'{xmin=}')
    print(f'{xmax=}')
    print(f'{imin=}')
    print(f'{imax=}')
    print(f'{ilen=}')
    print(f'{len(xxx)=}')
    print(f'{len(yyy)=}')
    xxx_bin1 = [ 0.0 ] * ilen
    yyy_bin1 = [ 0.0 ] * ilen
    nnn_bin1 = [  0  ] * ilen
    # iii[jjj] == bin_index of xxx[jjj]
    iii = list(map(lambda ttt: bin_index(ttt, xmin, xmax, bin_width), xxx))
    # xxx_bin[iii] and yyy_bin[iii] are sum of elements in bin iii
    # nnn_bin[iii] is count of elements in bin iii
    # accumulate bins
    for jjj, ttt in enumerate(xxx):
        xxx_bin1[iii[jjj]] += ttt
    for jjj, ttt in enumerate(yyy):
        yyy_bin1[iii[jjj]] += ttt
    for jjj, ttt in enumerate(iii):
        nnn_bin1[iii[jjj]] += 1
    # remove empty bins nnn_bin == 0
    nnn_bin = []
    xxx_bin = []
    yyy_bin = []
    for jjj, ttt in enumerate(nnn_bin1):
        if nnn_bin1[jjj]:
            N = float(nnn_bin1[jjj])
            nnn_bin.append(nnn_bin1[jjj])
            xxx_bin.append(xxx_bin1[jjj] / N)
            yyy_bin.append(yyy_bin1[jjj] / N)
    print('fill_bins output', '*' * 50)
    for jjj, (nnn, ttt, sss) in enumerate(zip(nnn_bin, xxx_bin, yyy_bin)):
        print(f'{jjj=}, {nnn=}, {ttt=}, {sss=}')
    return xxx_bin, yyy_bin

# update matplotlib_graph with matplotlib line for each ordinate (curve_data)
def draw_graph(master, matplotlib_graph):
    global resizable_canvas
    # master == frm_scollable
    frm_scrollable = master
    if not matplotlib_graph:
        lbl_Empty = tk.Label(master = frm_scrollable, text = 'Empty', anchor = tk.CENTER)
        return lbl_Empty

    # frm_draw_graph = tk.Frame(master = master, relief = tk.RAISED, borderwidth = 1)

    # units are pixels    fixme
    winfo_dx = resizable_canvas['winfo_dx']
    winfo_dy = resizable_canvas['winfo_dy']
    # fixme size minimum; good default is figsize = (3.5, 2.95)
    # fig = Figure(figsize = (3.5, 2.95), dpi = 100)
    # units are inches; dpi = 100 (dots per inch); assume "dot" == "pixel";
    sizex = max(4.0, winfo_dx / 100.0)
    sizey = max(3.0, winfo_dy / 100.0)
    fig = Figure(figsize = (sizex, sizey), dpi = 100)
    ax = fig.add_subplot()
    matplotlib_graph['fig'] = fig
    matplotlib_graph['ax'] = ax
    
    title = ''
    curves = matplotlib_graph['curves']
    for jjj, (curve_name, curve_data) in enumerate(curves.items()):
        curve_name1  = curve_data[2]
        color = curve_data[7]
        title += curve_name
        if jjj < len(curves) - 1:
            title += ','
    fig.suptitle(title, color = 'black')

    rrr = None
    ccc = None
    updated_lines = {}
    for curve_name, curve_data in curves.items():
        rrr     = curve_data[0]
        ccc     = curve_data[1]
        curve_name1 = curve_data[2]
        xxx     = curve_data[3]
        yyy     = curve_data[4]
        xxx_bin = curve_data[5]
        yyy_bin = curve_data[6]
        color   = curve_data[7]
        # curve_data[8] is line
        # secs    = mdate.epoch2num(xxx_bin)
        # tz = timezone.utc
        timezone = get_timezone()
        # for some reason this does not work
        # secs = list(map(lambda x: datetime.datetime.fromtimestamp(x, tz=timezone), xxx_bin))
        secs = list(map(lambda x: datetime.datetime.fromtimestamp(x), xxx_bin))
        # may want to check curve_name1 == curve_name
        # line, = ax.plot(xxx, yyy, color = color, linestyle = 'solid', linewidth = 0.5,
        # label = curve_name1)
        # marker == ',' for pixel
        line, = ax.plot(secs, yyy_bin,
                        color = color, linestyle = 'solid', marker = ',',
                        linewidth = 0.5, label = curve_name1)
        updated_lines[curve_name] = line
    for curve_name, line in updated_lines.items():
        curve_data = curves[curve_name]
        curve_data[8] = line
    # fixme this could easily be extended to display year when needed automatically
    # date_fmt = '%y/%m/%d %H:%M'
    date_fmt = '%m/%d %H:%M'
    date_formatter = mdate.DateFormatter(date_fmt)
    ax.xaxis.set_major_formatter(date_formatter)
    fig.autofmt_xdate()
    ax.grid(True)
    # ax.set_xlabel('epoch time [s]')
    ax.set(xlabel = None)
    ax.set_ylabel(f'row = {rrr} col={ccc}')
    # fixme canvas or frm_scollable
    canvas1 = FigureCanvasTkAgg(fig, master = frm_scrollable)
    canvas1.draw()

    # canvas1.get_tk_widget().grid(row = RRR, column = CCC, sticky = 'nsew')
    return canvas1.get_tk_widget() 


def redraw_graph(matplotlib_graph):
    curves = matplotlib_graph['curves']
    fig = matplotlib_graph['fig']
    ax = matplotlib_graph['ax']
    for curve_name, curve_data in curves.items():
        xxx_bin = curve_data[5]
        yyy_bin = curve_data[6]
        line = curve_data[8]
        # secs = mdate.epoch2num(xxx_bin)
        # tz = timezone.utc
        timezone = get_timezone()
        # for some reason this does not work
        # secs = list(map(lambda x: datetime.datetime.fromtimestamp(x, tz=timezone), xxx_bin))
        secs = list(map(lambda x: datetime.datetime.fromtimestamp(x), xxx_bin))
        line.set_data(secs, yyy_bin)
    fig.autofmt_xdate()


class FrameScrollable(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self,
                 master,
                 selected_rows,
                 selected_cols,
                 *args,
                 **kwargs):
        super().__init__(master = master, *args, **kwargs)
        global resizable_canvas
        # ttk.Frame not tk.Frame
        # frm_scrollable = tk.Frame(canvas) == self
        frm_scrollable = self
        self.canvas = master

        nrows = len(selected_rows)
        ncols = len(selected_cols)
        frows   = float(nrows)
        fcols   = float(ncols)
        winfo_y = float(self.canvas.winfo_reqheight())
        winfo_x = float(self.canvas.winfo_reqwidth())
        winfo_dy = winfo_y / frows
        winfo_dx = winfo_x / fcols
        resizable_canvas['canvas'    ] = self.canvas
        resizable_canvas['frows'     ] = frows
        resizable_canvas['fcols'     ] = fcols
        resizable_canvas['winfo_y'   ] = winfo_y
        resizable_canvas['winfo_x'   ] = winfo_x
        resizable_canvas['winfo_dy'  ] = winfo_dy
        resizable_canvas['winfo_dx'  ] = winfo_dx
        print(f'{frows=:7.3f} {winfo_y=:7.3f} {winfo_dy=:7.3f} ', end='')
        print(f'{fcols=:7.3f} {winfo_x=:7.3f} {winfo_dx=:7.3f}')

        frm_scrollable.bind('<Configure>', self.handle_frame_scrollable_configure)
    
        # embed an ancestor of canvas (frm_scroll) as the widget to be managed by canvas;
        # (0, 0) is coordinate point;
        # anchor = 'nw' means coordinate point is position of top left of window;
        self.canvas.create_window((0, 0), window = frm_scrollable, anchor = 'nw')

        # self.redraw(selected_rows, selected_cols)
        # def redraw(self, selected_rows, selected_cols):
        # frm_scrollable = self
        # nrows = len(selected_rows)
        # ncols = len(selected_cols)
        # self.delete_all_children('FrameScrollable')

        refresh_database_struct()

        table = database_struct['table']

        # should have (nrows=1; ncols=1;) or (nrows=rows_val; ncols=cols_val;);
        RRR = 0
        for rrr in selected_rows:
            CCC = 0
            for ccc in selected_cols:
                matplotlib_graph = table[rrr][ccc]
                widget = draw_graph(master = frm_scrollable, matplotlib_graph = matplotlib_graph)
                # fixme !!!!!!!!!!!!!!!!!!!!!
                # widget = draw_graph(master = self.canvas, matplotlib_graph = matplotlib_graph)
                name = f'draw_graph_{RRR}_{CCC}'
                self.register_child(name = name,
                                    gridrow = RRR,
                                    gridcol = CCC,
                                    sticky = 'nsew',
                                    widget = widget)
                # gridding done by widget
                # widget.grid(row = RRR, column = CCC, sticky = 'nsew')
                CCC += 1
            RRR += 1

        weight_row = [ 1 for rrr in range(nrows) ]
        weight_col = [ 1 for ccc in range(ncols) ]
        self.setup_grid(weight_row, weight_col)

    def redraw_times(self, selected_rows, selected_cols):
        table = database_struct['table']
        for rrr in selected_rows:
            for ccc in selected_cols:
                matplotlib_graph = table[rrr][ccc]
                redraw_graph(matplotlib_graph = matplotlib_graph)

    def handle_frame_scrollable_configure(self, event):
        """units of winfo_x, y, dx, dy are pixels"""
        global resizable_canvas
        canvas     = resizable_canvas['canvas']
        frows      = resizable_canvas['frows']
        fcols      = resizable_canvas['fcols']
        winfo_y    = float(event.height)
        winfo_x    = float(event.width)
        winfo_dy   = winfo_y / frows
        winfo_dx   = winfo_x / fcols 
        print(f'{frows=:7.3f} {winfo_y=:7.3f} {winfo_dy=:7.3f} ', end='')
        print(f'{fcols=:7.3f} {winfo_x=:7.3f} {winfo_dx=:7.3f}')
        # fixme !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        # fixme do not need this
        canvas.configure(scrollregion = canvas.bbox('all'))
        resizable_canvas['winfo_y' ] = winfo_y
        resizable_canvas['winfo_x' ] = winfo_x
        resizable_canvas['winfo_dy'] = winfo_dy
        resizable_canvas['winfo_dx'] = winfo_dx


class CanvasGraphTable(WeatherCanvas):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self,
                 master,
                 selected_rows,
                 selected_cols,
                 *args,
                 **kwargs):
        super().__init__(master = master, *args, **kwargs)
        self.container = master
        self.canvas = self
        self.frm_scrollable = None

        # we need the canvas to create the scrollbars
        self.scrollbar_x = tk.Scrollbar(self.container,
                                        orient = 'horizontal',
                                        command = self.canvas.xview)

        self.scrollbar_y = tk.Scrollbar(self.container,
                                        orient = 'vertical',
                                        command = self.canvas.yview) 

        self.canvas.configure(xscrollcommand = self.scrollbar_x.set)
        self.canvas.configure(yscrollcommand = self.scrollbar_y.set)

        self.redraw_all(selected_rows, selected_cols)
        
    def get_scrollbars(self):
        return self.scrollbar_x, self.scrollbar_y

    def redraw_all(self, selected_rows, selected_cols):
        """completely redo frame and all graphs in it;"""
        self.delete_child('CanvasGraphTable', 'frm_scrollable')

        # we need the scrollbars to bind to fscroll <Configure>
        self.frm_scrollable = FrameScrollable(master = self.canvas,
                                              selected_rows = selected_rows,
                                              selected_cols = selected_cols)

        # frm_scrollable.tkraise()

        self.register_child(name = 'frm_scrollable',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = self.frm_scrollable)

        self.setup_grid([1], [1])

    def redraw_times(self, selected_rows, selected_cols):
        self.frm_scrollable.redraw_times(selected_rows, selected_cols)


class FrameGraphTable(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    """container for scroll bars and FrameScroll"""
    def __init__(self, selected_rows, selected_cols, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # ttk.Frame not tk.Frame
        # frm_graph_table = tk.Frame(master = master) == container == self
        container = self

        nrows = len(selected_rows)
        ncols = len(selected_cols)

        # declare canvas before scrollbar_x; scrollbar_y;
        canvas = CanvasGraphTable(master = container,
                                  selected_rows = selected_rows,
                                  selected_cols = selected_cols)
        scrollbar_x, scrollbar_y = canvas.get_scrollbars()
        
        self.register_child(name = 'canvas_graph_table',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = canvas)

        self.register_child(name = 'scrollbar_x',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'ew',
                            widget = scrollbar_x)

        self.register_child(name = 'scrollbar_y',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'ns',
                            widget = scrollbar_y)

        # recommended order
        # actual_graph.grid()
        # no mention of frm_scrollable.grid()
        # frm_graph_table.grid(row = 0, column = 0, sticky = 'nsew')
        # canvas.grid     (row = 0, column = 0, sticky = 'nsew')
        # scrollbar_y.grid(row = 0, column = 1, sticky = 'ns')
        # scrollbar_x.grid(row = 1, column = 0, sticky = 'ew')
        self.setup_grid([1, 0], [1, 0])

class FrameGraphsControl(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # frm_graphs_control = tk.Frame(master = master)

        self.register_child(name = 'frm_main_options',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameMainOptions(master = self) )

        # fixme depressed when database loaded already
        widget = tk.Button(master = self, text = 'load database')
        widget.bind('<Button-1>', self.handle_button_load_database)
        self.register_child(name = 'btn_load_database',
                            gridrow = 0,
                            gridcol = 1,
                            sticky = 'nsew',
                            widget = widget)

        widget = FrameGraphChooseDate(master = self, begin_end = 'begin')
        self.register_child(name = 'frm_graph_choose_date_beg',
                            gridrow = 0,
                            gridcol = 2,
                            sticky = 'nsew',
                            widget = widget)

        widget = FrameGraphChooseDate(master = self, begin_end = 'end')
        self.register_child(name = 'frm_graph_choose_date_end',
                            gridrow = 0,
                            gridcol = 3,
                            sticky = 'nsew',
                            widget = widget)

        self.setup_grid([0], [0, 0, 0, 0])

    def handle_button_load_database(self, event):
        if settings['redraw_pending_graphs_all'] or \
           settings['redraw_pending_single_graph_all']:
            clear_database_struct()
        elif settings['redraw_pending_graphs_times'] or \
             settings['redraw_pending_single_graph_times']:
            refresh_database_struct()
        load_database()


class FrameGraphs(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global settings
        # frm_graphs = tk.Frame(master = master)
        graph_rows = settings['graph_table']['rows_val']
        graph_cols = settings['graph_table']['cols_val']

        self.register_child(name = 'frm_graphs_control',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameGraphsControl(master = self) )

        selected_rows = list(range(graph_rows))
        selected_cols = list(range(graph_cols))
        self.register_child(name = 'frm_graph_table',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameGraphTable(master = self,
                                                     selected_rows = selected_rows,
                                                     selected_cols = selected_cols) )
        self.setup_grid([0, 1], [1])


class FrameSingleGraph(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        global settings
        # frm_single_graph = tk.Frame(master = master)

        self.register_child(name = 'frm_main_options',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameMainOptions(master = self) )

        rrr = settings['graph_table']['single_graph_row_val']
        ccc = settings['graph_table']['single_graph_col_val']
        selected_rows = [ rrr ]
        selected_cols = [ ccc ]
        self.register_child(name = 'frm_graph_table',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameGraphTable(master = self,
                                                       selected_rows = selected_rows,
                                                       selected_cols = selected_cols) )
        self.setup_grid([0, 1], [1])


class FrameCalibration1(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        
        self.register_child(name = 'frm_calibration1',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = tk.Label(master = self, text = "Under Construction") )
        self.setup_grid([1], [1])


def handle_root_window_close(event):
    root_window_close()


def handle_interrupt(self, signum, stack):
    """catch keyboard interrupts; clean up; close resources;"""
    log.error('Handling interrupt.  Closing main window.')
    print(f'{signum=}')
    print(f'{stack=}')
    root_window_close()


def root_window_close():
    global settings
    global root_window
    settings['exit_pending'] = False
    if messagebox.askokcancel('Quit', 'Do you want to quit?'):
        root_window.save_configuration()
        root_window.destroy()
        log.info('exiting.')
        sys.exit(0)


class FrameCalibration(WeatherFrame):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # frm_calibration = tk.Frame(master = master)

        self.register_child(name = 'frm_main_options',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameMainOptions(master = self) )

        self.register_child(name = 'frm_calibration1',
                            gridrow = 1,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameCalibration1(master = self) )

        self.setup_grid([0, 1], [1])


class WindowRoot(WeatherWindow):
    def class_name(self):
        return self.__class__.__name__
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def begin(self):
        global settings
        global root_menu
        root_window = self
        signal.signal(signal.SIGTERM, handle_interrupt)
        signal.signal(signal.SIGINT , handle_interrupt)
        root_window.resizable(width = True, height = True)
        root_window.title('wg-weather-gui')
        root_window.bind('<Control-KeyPress-w>', handle_root_window_close)
        root_window.bind('<Control-KeyPress-q>', handle_root_window_close)
        root_window.protocol('WM_DELETE_WINDOW', root_window_close)
        self.load_configuration()

        # main draw(); only call once;

        # self.root_menu = self.create_root_menu()
        self.root_menu = None

        # settings first to initialize settings when configuration blank
        self.register_child(name = 'frm_settings',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameSettings(master = root_window) )
        self.register_child(name = 'frm_graphs',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameGraphs(master = root_window) )
        self.register_child(name = 'frm_calibration',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameCalibration(master = root_window) )
        self.register_child(name = 'frm_single_graph',
                            gridrow = 0,
                            gridcol = 0,
                            sticky = 'nsew',
                            widget = FrameSingleGraph(master = root_window) )
        self.setup_grid([1], [1])
        settings['redraw_pending_graphs_all'] = False
        settings['redraw_pending_single_graph_all'] = False
        settings['redraw_pending_graphs_times'] = False
        settings['redraw_pending_single_graph_times'] = False

        set_btn_load_database_red(all_not_times = True)
        # infinite loop;
        self.redraw_graphs()
        root_window.mainloop()


    def delete_some_data(self):
        global root_cvar
        """not used"""
        log.info('frame_root wipe it all out')
        # settings['ordinates'] = None
        for jjj, ppp in root_frame.items():
            frm = ppp[0]
            print(f'deleting {jjj=} {ppp=} {frm=}')
            frm.destroy()
        if root_cvar['time_limit1']:
            del root_cvar['time_limit1']
            root_cvar['time_limit1'] = None
        if root_cvar['time_limit2']:
            del root_cvar['time_limit2']
            root_cvar['time_limit2'] = None
        if root_cvar['graph_table']:
            del root_cvar['graph_table']
            root_cvar['graph_table'] = None
        root_frame = {}

    def redraw_graphs(self):
        global settings
        #frm_scrollable_graphs = self.get_ancestor(['frm_graphs',
        #                                           'frm_graph_table',
        #                                           'canvas_graph_table',
        #                                           'frm_scrollable'])
        #frm_scrollable_single_graph = self.get_ancestor(['frm_single_graph',
        #                                                 'frm_graph_table',
        #                                                 'canvas_graph_table',
        #                                                 'frm_scrollable'])
        frm_canvas_graph_table_graphs = self.get_ancestor(['frm_graphs',
                                                           'frm_graph_table',
                                                           'canvas_graph_table'])
        frm_canvas_graph_table_single_graph = self.get_ancestor(['frm_single_graph',
                                                                 'frm_graph_table',
                                                                 'canvas_graph_table'])
        if settings['exit_pending']:
            print('exit_pending')
            self.root_window_close()
        # call all before times in case not initialized
        if settings['redraw_pending_graphs_all']:
            print('redraw graphs (multiple)')
            graph_rows = settings['graph_table']['rows_val']
            graph_cols = settings['graph_table']['cols_val']
            selected_rows = list(range(graph_rows))
            selected_cols = list(range(graph_cols))
            frm_canvas_graph_table_graphs.redraw_all(selected_rows, selected_cols)
            settings['redraw_pending_graphs_all'] = False
        if settings['redraw_pending_single_graph_all']:
            print('redraw single_graph_all')
            rrr = settings['graph_table']['single_graph_row_val']
            ccc = settings['graph_table']['single_graph_col_val']
            selected_rows = [ rrr ]
            selected_cols = [ ccc ]
            frm_canvas_graph_table_single_graph.redraw_all(selected_rows, selected_cols)
            settings['redraw_pending_single_graph_all'] = False
        if settings['redraw_pending_graphs_times']:
            print('redraw graphs (multiple) redo times only')
            graph_rows = settings['graph_table']['rows_val']
            graph_cols = settings['graph_table']['cols_val']
            selected_rows = list(range(graph_rows))
            selected_cols = list(range(graph_cols))
            frm_canvas_graph_table_graphs.redraw_times(selected_rows, selected_cols)
            settings['redraw_pending_graphs_times'] = False
        if settings['redraw_pending_single_graph_times']:
            print('redraw single_graph')
            rrr = settings['graph_table']['single_graph_row_val']
            ccc = settings['graph_table']['single_graph_col_val']
            selected_rows = [ rrr ]
            selected_cols = [ ccc ]
            frm_canvas_graph_table_single_graph.redraw_times(selected_rows, selected_cols)
            settings['redraw_pending_single_graph_times'] = False
        # fixme save configuration every redraw
        self.save_configuration()
        self.current_mode_window_in_front()
        root_window.after(3000, self.redraw_graphs)

    def get_ancestor(self, ancestors):
        root_window = self
        widget = root_window
        for name in ancestors:
            if not (name in widget.my_child.keys()):
                return None
            widget = widget.my_child[name][WF_WIDGET]
            # (name1, gridrow, gridcol, sticky, widget, gridrowspan, gridcolspan) = child
        return widget

    def current_mode_window_in_front(self):
        global settings
        frame = None
        if settings['root_mode'] == 'mode_graphs':
            frame = self.my_child['frm_graphs'][WF_WIDGET]
        elif settings['root_mode'] == 'mode_settings':
            frame = self.my_child['frm_settings'][WF_WIDGET]
        elif settings['root_mode'] == 'mode_calibration':
            frame = self.my_child['frm_calibration'][WF_WIDGET]
        elif settings['root_mode'] == 'mode_single_graph':
            frame = self.my_child['frm_single_graph'][WF_WIDGET]
        else:
            log.error(f'unknown {settings["root_mode"]=}')
        frame.tkraise()

    def save_calibration(self):
        root_window = self
        log.debug('save_calibration')

    def export_configuration(self):
        root_window = self
        log.debug('export_configuration')

    def load_configuration(self):
        global fname_config
        global settings
        root_window = self
        try:
            with open(fname_config, 'r') as fp:
                json_settings = fp.read()
        except FileNotFoundError as exc:
            log.info('configuration file not found')
            return
        settings = json.loads(json_settings)
        # fixme reinit data structures
        ORD = settings['ordinates']   if 'ordinates'   in settings.keys() else None
        BIN = settings['bin_width']   if 'bin_width'   in settings.keys() else None
        tbl = settings['graph_table'] if 'graph_table' in settings.keys() else None
        graph_rows = tbl['rows_val']  if tbl and 'rows_val' in tbl.keys() else None
        graph_cols = tbl['cols_val']  if tbl and 'cols_val' in tbl.keys() else None
        # convert indices from strings to numbers
        if BIN and graph_rows and graph_cols:
            for rrr in range(graph_rows):
                RRR = str(rrr)
                bin_row = BIN[RRR]
                for ccc in range(graph_cols):
                    CCC = str(ccc)
                    bin_col = bin_row[CCC]
                    del bin_row[CCC]
                    bin_row[ccc] = bin_col
                del BIN[RRR]
                BIN[rrr] = bin_row
        # convert indices from strings to numbers
        if ORD and graph_rows and graph_cols:
            for rrr in range(graph_rows):
                RRR = str(rrr)
                ord_row = ORD[RRR]
                for ccc in range(graph_cols):
                    CCC = str(ccc)
                    ord_col = ord_row[CCC]
                    del ord_row[CCC]
                    ord_row[ccc] = ord_col
                del ORD[RRR]
                ORD[rrr] = ord_row
        if graph_rows and graph_cols:
            clear_database_struct()
        if 'geometry' in settings.keys():
            geometry = settings['geometry']
            root_window.geometry(geometry)
            root_window.minsize(DEFAULT_SIZE_WIDTH_MIN, DEFAULT_SIZE_HEIGHT_MIN)
        else:
            root_window.geometry('200x200+0+0')
        settings['redraw_pending_graphs_all'] = True
        settings['redraw_pending_single_graph_all'] = True

    def save_configuration(self):
        global fname_config
        global settings
        root_window = self
        settings['geometry'] = root_window.geometry()
        json_settings = json.dumps(settings)
        path_fname_config = pathlib.Path(fname_config)
        parent = path_fname_config.parent
        if not parent.exists():
            # parents = True; resembles mkdir -p
            parent.mkdir(parents = True)
        with open(fname_config, 'w') as fp:
            fp.write(json_settings)

    def create_root_menu(self):
        root_window = self
        root_menu = Menu(root_window)
        root_window.config(menu = root_menu)
        item_file = Menu(root_menu, tearoff = False)
        item_file.add_command(label = 'Save Calibration', command = self.save_calibration)
        item_file.add_command(label = 'Export Configuration', command = self.export_configuration)
        root_menu.add_cascade(label = 'File', menu = item_file)
        item_actions = Menu(root_menu, tearoff = False)
        # item_actions.add_command(label = 'Action1', command = action1)
        root_menu.add_cascade(label = 'Actions', menu = item_actions)
        return root_menu


def process_options():
    """
    parse command args and
    (1) handle simplest options then exit or
    (2) return config_dict and continue
    """

    usage = """wg-weather-gui.py [--debug] [--help] [--version]"""
    print('argparse.ArgumentParser')
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
                        dest='config',
                        default=DEFAULT_CONFIG,
                        action='store',
                        help='configuration file with sensor map')

    print('parser.parse_args')
    options = parser.parse_args()

    if options.version:
        print(f"wg-weather-gui.py {SERVICE_VERSION=}")
        sys.exit(1)

    if options.debug:
        print('log.setLevel(logging.DEBUG)')
        log.setLevel(logging.DEBUG)

    global fname_config
    if options.config:
        # get rid of tilde
        fname_config = options.config
        fname_config = str(pathlib.Path(fname_config).expanduser())
        print(f'using {fname_config=}')

    """
    print(f'{options.config=}')
    log.debug('options.config = %s', str(options.config))
    log.debug('reading config_dict')
    _, config_dict = weecfg.read_config(options.config)
    """
    return options


def setup_matplotlib():
    # fixme matplotlib.rc moved from frame_draw_graph
    font = {'family' : 'monospace',
            'weight' : 'medium',
            'size'   : 6 }
    lines = {'linewidth' : 0.5,
             'linestyle' : 'solid' }
    matplotlib.rc('font', **font)
    matplotlib.rc('lines', **lines)


def main():
    global root_window
    log.addHandler(logging.StreamHandler(sys.stdout))
    log.addHandler(logging.handlers.SysLogHandler(address="/dev/log"))
    print('log.setLevel INFO')
    log.setLevel(logging.INFO)

    setup_matplotlib()
    
    options = process_options()
  
    root_window = WindowRoot()
    root_window.begin()


if __name__ == '__main__':
    main()

# eee eof
