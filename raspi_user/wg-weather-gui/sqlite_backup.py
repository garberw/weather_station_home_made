#!/usr/bin/env python
"""
copy one database to another using sqlite3.backup()
works even if the database is being accessed by other clients
or concurrently by the same connection;
"""

import sys
import argparse
import sqlite3

def progress(status, remaining, total):
    print(f'Copied {total-remaining} of {total} pages...')


def sqlite3_backup(fname_src, fname_dst, quiet):
    """
    fname_src e.g. 'example.db'
    fname_dst e.g. 'example.db'
    """
    src = sqlite3.connect(fname_src)
    dst = sqlite3.connect(fname_dst)
    with dst:
        if quiet:
            src.backup(dst, pages=1)
        else:
            src.backup(dst, pages=1, progress=progress)
    dst.close()
    src.close()

def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('src',
                        help = 'the input database source to be backed up')
    parser.add_argument('dst',
                        help = 'the output backup destination result')
    parser.add_argument('-q', '--quiet',
                        action = 'store_true',
                        help = 'only output on error')
    options = parser.parse_args()
    return options

def main():
    options = parse_args(argv)
    sqlite3_backup(options.src, options.dest, options.quiet)

if __name__ == '__main__':
    main(sys.argv)
