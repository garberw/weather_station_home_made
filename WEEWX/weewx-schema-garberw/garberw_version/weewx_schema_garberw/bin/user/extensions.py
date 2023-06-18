#
#    Copyright (c) 2009-2015 Tom Keffer <tkeffer@gmail.com>
#
#    See the file LICENSE.txt for your full rights.
#

"""User extensions module

This module is imported from the main executable, so anything put here will be
executed before anything else happens. This makes it a good place to put user
extensions.
"""

import locale
# This will use the locale specified by the environment variable 'LANG'
# Other options are possible. See:
# http://docs.python.org/2/library/locale.html#locale.setlocale
locale.setlocale(locale.LC_ALL, '')

# garberw added =====================================================
import weewx
import weewx.units

weewx.units.obs_group_dict['caseTemp1'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp2'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp3'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp4'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp5'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp6'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp7'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseTemp8'     ] = 'group_temperature'
weewx.units.obs_group_dict['caseHumid1'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid2'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid3'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid4'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid5'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid6'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid7'    ] = 'group_percent'
weewx.units.obs_group_dict['caseHumid8'    ] = 'group_percent'
weewx.units.obs_group_dict['cpuTemp1'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp2'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp3'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp4'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp5'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp6'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp7'      ] = 'group_temperature'
weewx.units.obs_group_dict['cpuTemp8'      ] = 'group_temperature'
weewx.units.obs_group_dict['gas_resistance'] = 'group_resistance'
weewx.units.obs_group_dict['iaq'           ] = 'group_iaq'

weewx.units.USUnits['group_resistance'] = 'kohm'
weewx.units.USUnits['group_iaq'       ] = 'iaq'

weewx.units.MetricUnits['group_resistance'] = 'kohm'
weewx.units.MetricUnits['group_iaq'       ] = 'iaq'

weewx.units.MetricWXUnits['group_resistance'] = 'kohm'
weewx.units.MetricWXUnits['group_iaq'       ] = 'iaq'

weewx.units.default_unit_format_dict['kohm'] = '%.1f'
weewx.units.default_unit_format_dict['iaq'] = '%.1f'

weewx.units.default_unit_label_dict['kohm'] = ' kOhm'
weewx.units.default_unit_label_dict['iaq'] = ' IAQ'

KOHM_PER_OHM = 0.001
OHM_PER_KOHM = 1000.0
# weewx.units.conversion_dict['ohm']  = { 'kohm': lambda x : x * KOHM_PER_OHM }                     
# weewx.units.conversion_dict['kohm'] = { 'ohm' : lambda x : x * OHM_PER_KOHM }                     

