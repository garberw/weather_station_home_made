# installer for the weewx_wmod driver
# Copyright 2022-2028 William Garber
# Distributed under the terms of the GNU Public License (GPLv3)

from weecfg.extension import ExtensionInstaller

def loader():
    return WeewxSchemaGarberwInstaller()

class WeewxSchemaGarberwInstaller(ExtensionInstaller):
    def __init__(self):
        super(WeewxSchemaGarberwInstaller, self).__init__(
            version="0.88",
            name='weewx_schema_garberw',
            description='Add new types to schema',
            author="William Garber",
            author_email="william.garber@att.net",
            # files=[('bin/user', ['bin/user/weewx_schema_garberw.py'])]
            files=[('bin/user', ['bin/user/extensions.py'])]
            )
