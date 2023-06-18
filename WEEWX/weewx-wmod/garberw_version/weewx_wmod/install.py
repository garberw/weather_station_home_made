# installer for the weewx_wmod driver
# Copyright 2022-2028 William Garber
# Distributed under the terms of the GNU Public License (GPLv3)

from weecfg.extension import ExtensionInstaller

def loader():
    return WeewxWMODInstaller()

class WeewxWMODInstaller(ExtensionInstaller):
    def __init__(self):
        super(WeewxWMODInstaller, self).__init__(
            version="0.88",
            name='weewx_wmod',
            description='Capture data from wmod',
            author="William Garber",
            author_email="william.garber@att.net",
            files=[('bin/user', ['bin/user/weewx_wmod.py'])]
            )
