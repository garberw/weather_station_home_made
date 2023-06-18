#!/usr/bin/env python
"""
find all combinations;  a combination is ( __FILE__, __func__ ) as reported by g++;
automatically generate FNAME_NEW containing list of all combinations in format of header
for inclusion in 'weather_lib_util.cpp';  for each combination choice true or false is given;
default for all is true;  this turns ser_print() output on for that combination;
find which combinations are new additions; save to FNAME_ADDITIONS;
this is done based on combination independent of value true or false;
if no command line arg is given user can edit FNAME_NEW and mv it to FNAME_ORIG;
if command line arg 'merge' is given run
$ cp FNAME_ORIG FNAME_BACKUP
$ cat FNAME_ADDITIONS >> FNAME_ORIG
"""
import sys
import re
import subprocess
import os
import glob
import datetime

ARDUINO_ROOT     = "/home/garberw/Arduino"
DATE_STR         = datetime.datetime.now().strftime("%Y-%m-%d-%X")
FNAME_ORIG       = "weather_lib_util_ser_print.h"
FNAME_NEW        = "weather_lib_util_ser_print.h.new.eraseme"
FNAME_ADDITIONS  = "weather_lib_util_ser_print.h.additions.eraseme"
FNAME_DUPLICATES = "weather_lib_util_ser_print.h.duplicates.eraseme"
FNAME_BACKUP     = FNAME_ORIG + "." + DATE_STR
ECHO             = False

DEBUG = False

def wg_halt():
    sys.exit(1)


def run_process(cmd, echo = ECHO):
    """
    return tuple (exit_code, capture); capture is list of lines of output from process;
    """
    try:
        exe = cmd.split()
        # subpr = subprocess.Popen(exe, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        # piter = iter(subpr.stdout.readline, b'')
        # capture = []
        # for line in piter:
        #    capture.append(line)
        #    if echo:
        #        print(line)
        capture1 = subprocess.check_output(exe, stderr = subprocess.STDOUT)
        capture2 = capture1.split(b"\n")
        capture3 = [ line.decode() for line in capture2 ]
        capture = [ line for line in capture3 if len(line.strip()) != 0 ]
        if echo:
            for jjj, line in enumerate(capture):
                print(f"{jjj=} {line=}")
        return (0, capture)
    except subprocess.CalledProcessError as exc:
        print(f"returncode={exc.returncode}")
        return (exc.returncode, exc.output)


def display_prototypes():
    """
    run ctags on source code .cpp .h .ino in cwd and get list of lines of output;
    split each line in list and keep fields for filename and function (a combination)
    return list of combinations;
    """
    files = glob.glob("*.cpp")
    files += glob.glob("*.h")
    files += glob.glob("*.ino")
    if DEBUG:
        print(f"{files=}")
    capture = []
    for fff in files:
        cmd = f"ctags -x --c++-kinds=f --language-force=c++ {fff}"
        (exit_code, capture1) = run_process(cmd)
        print(f"ctags exit_code={exit_code} file={fff}")
        capture += capture1
    # fixme exit_code
    temp = []
    for line in capture:
        split_line = line.split()
        if DEBUG:
            print(f"{split_line=}")
        temp.append(( split_line[3] , split_line[0] ))
    temp.sort()
    return temp


def loop_over_dirs():
    """
    run display_prototypes for dir in ../weather_*;
    """
    cwd = os.getcwd()
    os.chdir(ARDUINO_ROOT)
    weather_dir = glob.glob('weather_*')
    result = []
    for wdir in weather_dir:
        os.chdir(ARDUINO_ROOT)
        os.chdir(wdir)
        temp = display_prototypes()
        result += temp
    os.chdir(cwd)
    return result


def find_duplicates(fname, result):
    """
    print (count, line) for each line occuring more than once;
    """
    duplicates = []
    counts = []
    for line in result:
        if line in duplicates:
            continue
        count1 = result.count(line)
        if count1 <= 1:
            continue
        duplicates.append(line)
        counts.append(count1)
    with open(FNAME_DUPLICATES, "w", encoding = "ascii") as fptr:
        for count1, line in zip(counts, duplicates):
            line_out = f"{count1:4d} {line}"
            print(line_out)
            fptr.write(line_out)
            fptr.write("\n")


def generate_new_file(filename, result):
    """
    input is output from loop_over_dirs which is a list of combinations;
    write formatted header simply listing these combinations with default value of true
    setting ser_print() output turned on for that combination;
    """
    with open(filename, "w", encoding = "ascii") as fptr:
        fptr.write("#ifndef weather_lib_util_ser_print_h\n")
        fptr.write("#define weather_lib_util_ser_print_h\n")
        fptr.write("#include <weather_lib_util.h>\n")
        fptr.write("static serial_print_entry display_table[] = {\n")
        for line in result:
            line_out = f'  {{ true, F(\"{line[0]}\"), F(\"{line[1]}\") }},\n'
            fptr.write(line_out)
        fptr.write("};\n")
        fptr.write("#endif\n")
        fptr.write("// eee eof\n")

def parse_existing_file(filename):
    """
    read filename and parse accepting only lines of format
    { true/false, F("file"), F("function") },
    extract combination [ file, function ] from each line;
    return list prev_result of combinations;
    """
    prev_result = []
    pattern = re.compile(r'^\s*\{\s*(true|false)\s*,\s*F\s*\(\s*"[^"]*"\s*\)\s*,\s*F\s*\(\s*"[^"]*"\s*\)\s*,?\s*\}\s*,?\s*$')
    quoted = re.compile(r'"[^"]*"')
    with open(filename, "r", encoding = "ascii") as fptr:
        for line in fptr:
            if not pattern.match(line):
                if DEBUG:
                    print(f"no match {line=}")
                continue
            vals = quoted.findall(line)
            if DEBUG:
                print(f"match {line=}")
                print(f"match {vals=}")
            if len(vals) != 2:
                print("error: not two vals")
            else:
                file = vals[0]
                func = vals[1]
                if (len(file) < 2) or (len(func) < 2):
                    print("error; missing quotes")
                    return []
                fileb = file[0]
                filee = file[-1]
                funcb = func[0]
                funce = func[-1]
                if (fileb != '"') and (filee != '"'):
                    print("error; fileb or filee missing quotes")
                    return []
                if (funcb != '"') and (funce != '"'):
                    print("error; funcb or funce missing quotes")
                    return []
                prev_result.append((file[1:-1], func[1:-1]))
    return prev_result

def write_additions(filename, additions):
    """
    write additions in same format as header FNAME_NEW without header and footer;
    """
    with open(filename, "w", encoding = "ascii") as fptr:
        for line in additions:
            line_out = f'  {{ true, F("{line[0]}"), F("{line[1]}") }},\n'
            fptr.write(line_out)


def copy_orig_to_backup(fname_orig, fname_backup):
    with open(fname_backup, "w", encoding = "ascii") as fptr_backup:
        with open(fname_orig, "r", encoding = "ascii") as fptr_orig:
            for line_orig in fptr_orig:
                fptr_backup.write(line_orig)


def merge_additions(fname_additions, fname_backup, fname_orig):
    # end of array in fname_backup and fname_orig is single line with only "};"
    pattern_end = re.compile(r"^\s*\}\s*;\s*$")
    with open(fname_orig, "w", encoding = "ascii") as fptr_orig:
        with open(fname_additions, "r", encoding = "ascii") as fptr_additions:
            with open(fname_backup, "r", encoding = "ascii") as fptr_backup:
                for line_backup in fptr_backup:
                    if pattern_end.match(line_backup):
                        # read additions and echo to orig
                        # does not contain header or footer
                        for line_additions in fptr_additions:
                            fptr_orig.write(line_additions)
                    fptr_orig.write(line_backup)



def run(merge = None):
    """
    generate FNAME_NEW;  find print and remove duplicates; generate FNAME_ADDITIONS;
    if merge, execute shell commands
    $ cp FNAME_ORIG FNAME_BACKUP
    $ cat FNAME_ADDITIONS >> FNAME_ORIG'
    """
    print("loop_over_dirs; run ctags and collect combinations;")
    result = loop_over_dirs()
    print("=" * 60)
    print()
    print(f"find_duplicates; save result {FNAME_DUPLICATES=}")
    print()
    find_duplicates(FNAME_DUPLICATES, result)
    print()
    # remove duplicates
    print(f"generate_new_file; save result with no duplicates {FNAME_NEW=};")
    if DEBUG:
#    if True:
        print(f"{result=}")
    result = list(set(result))
    generate_new_file(FNAME_NEW, result)
    print()
    print(f"parse_existing_file {FNAME_ORIG=};")
    prev_result = parse_existing_file(FNAME_ORIG)
    if DEBUG:
#    if True:
        print(f"{prev_result=}")
    print()
    print(f"generate additions and write {FNAME_ADDITIONS=}")
    additions = list(set(result) - set(prev_result))
    additions.sort()
    write_additions(FNAME_ADDITIONS, additions)
    print()
    if merge:
        print("merge additions ...")
        print()
        print(f"copy_orig_to_backup {FNAME_ORIG=} {FNAME_BACKUP=}")
        copy_orig_to_backup(FNAME_ORIG, FNAME_BACKUP)
        print()
        print(f"merge {FNAME_ADDITIONS=} at end of {FNAME_ORIG=}")
        merge_additions(FNAME_ADDITIONS, FNAME_BACKUP, FNAME_ORIG)
        print()
    print("done; success;")

if __name__ == '__main__':
    MERGE = False
    if len(sys.argv) == 1:
        pass
    elif len(sys.argv) == 2 and sys.argv[1] == 'merge':
        MERGE = True
    else:
        print("error: unrecognized arg")
    run(MERGE)

# eee eof
