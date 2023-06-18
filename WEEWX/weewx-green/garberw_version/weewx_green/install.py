# installer for the weewx_green driver
# Copyright 2022-2028 William Garber
# Distributed under the terms of the GNU Public License (GPLv3)

from weecfg.extension import ExtensionInstaller

def loader():
    return WeewxGreenInstaller()

class WeewxGreenInstaller(ExtensionInstaller):
    def __init__(self):
        super(WeewxGreenInstaller, self).__init__(
            version="0.88",
            name='weewx_green',
            description='Capture data from green',
            author="William Garber",
            author_email="william.garber@att.net",
            files=[('bin/user', ['bin/user/weewx_green.py'])]
            )
