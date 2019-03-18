#!/usr/bin/env python3

import subprocess
import os
import re
import sys
import pathlib


def verbose(*args):
    print(*args, file=sys.stderr)


def main(argv):
    libname = argv[1]
    cc = argv[2:]

    verbose('Looking for', libname)
    verbose('cc:', cc)

    o = subprocess.run(cc + ['-print-search-dirs'], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    libdirs = re.search(r'[\^\n]libraries: =(.*)', o.stdout.decode('utf-8')).group(1).strip().split(os.pathsep)

    verbose('Search path:\n\t' + '\n\t'.join(libdirs))

    for libdir in libdirs:
        p = pathlib.Path(libdir) / 'lib{}.so'.format(libname)
        if p.is_file():
            verbose('Found', p)
            p = p.resolve()
            verbose('Real path', p)
            verbose('Name', p.name)
            dlname = re.search(r'(.*?\.so(?:\.[^.]+)?)', p.name).group(0)
            verbose('Reduced name', dlname)
            assert p.with_name(dlname).resolve() == p
            print(dlname)
            quit(0)

    quit(1)


if __name__ == '__main__':
    main(sys.argv)
