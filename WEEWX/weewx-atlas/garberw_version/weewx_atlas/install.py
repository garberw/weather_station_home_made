# installer for the weewx-sdr driver
# Copyright 2016-2022 William Garber
# Distributed under the terms of the GNU Public License (GPLv3)

from weecfg.extension import ExtensionInstaller

def loader():
    return WeewxAtlasInstaller()

class WeewxAtlasInstaller(ExtensionInstaller):
    def __init__(self):
        super(WeewxAtlasInstaller, self).__init__(
            version="0.88",
            name='weewx_atlas',
            description='Capture data from arduino wpa',
            author="William Garber",
            author_email="william.garber@att.net",
            files=[('bin/user', ['bin/user/weewx_atlas.py'])]
        )
