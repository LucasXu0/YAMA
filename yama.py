#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import bisect
import subprocess
from colorama import Fore, Back, Style

# input_dir
INTPUT_DIR = sys.argv[1]
IOS_DEVICESUPPORT_PATH = sys.argv[2]
APP_DSYM_PATH = sys.argv[3]

YAMA_FILE_MACH_HEADER   = INTPUT_DIR + '/YAMA_FILE_MACH_HEADER'
YAMA_FILE_RECORDS       = INTPUT_DIR + '/YAMA_FILE_RECORDS'
YAMA_FILE_STACKS        = INTPUT_DIR + '/YAMA_FILE_STACKS'

class Record:
    def __init__(self, type_flag, stack_id, size, address):
        self.type_flag = type_flag
        self.stack_id = stack_id
        self.size = size
        self.address = address

# convert mach_headers path to local path
def get_mach_headers():
    bad_case = 0
    mach_headers = []
    minimum_slide = 'ffffffffffffffff'
    for header in open(YAMA_FILE_MACH_HEADER):
        header = header.strip('\n').split(' ') # ('0000000102644000', /Developer/usr/lib/libBacktraceRecording.dylib)
        slide, path = header[0], header[1]
        symbol_path = IOS_DEVICESUPPORT_PATH + path
        if os.path.exists(symbol_path):
            mach_headers.append((slide, symbol_path))
        else:
            mach_headers.append((slide, '/'))
            bad_case += 1
            print(symbol_path)
            print(Fore.CYAN + 'could not find library(' + path + ') in your local path')
        minimum_slide = min(minimum_slide, slide)
    print(Style.RESET_ALL)
    print('bad case in [convert_mach_header_to_local_symbols] = ' + str(bad_case))
    mach_headers.sort()
    mach_headers.pop(0)
    mach_headers.insert(0, (minimum_slide, APP_DSYM_PATH))
    return mach_headers

def dump_headers(mach_headers):
    for i in range(len(mach_headers)):
        print('[{:3d}] {} {}'.format(i, mach_headers[i][0], mach_headers[i][1]))

def resolve_symbol(mach_headers, address, only_in_main_executable=True):
    def str_to_hex(str):
        return hex(int(str, 16))

    position = bisect.bisect_left(mach_headers, (address, 'AAAAA'))

    if only_in_main_executable and position > 1:
        return ''

    (slide, symbol_path) = mach_headers[position - 1]
    command = 'atos' + ' -arch' + ' arm64' + ' -o "' + symbol_path + '" -l ' + str_to_hex(slide) + ' ' + str_to_hex(address)
    # print(command)
    return subprocess.check_output(command, shell=True).decode("utf-8").strip('\n')

def get_stacks():
    stacks = {}
    for stack in open(YAMA_FILE_STACKS):
        stack = stack.strip('\n').split(' ')
        stacks[stack[0]] = stack[1:]
    return stacks

def get_records():
    records = []
    for raw_record in open(YAMA_FILE_RECORDS):
        raw_record = raw_record.strip('\n').split(' ') # 00000020 07d00400000e7971 0000000000004000 000000010562c000
        records.append(Record(type_flag=raw_record[0], stack_id=raw_record[1], size=int(raw_record[2]), address=raw_record[3]))
    return records

def dump_records_total_size(records):
    total_size = 0
    for record in records:
        total_size += record.size
    print(Fore.BLUE + 'total size = {}MB'.format(total_size / 1024.0 / 1024.0))
    print(Style.RESET_ALL)

def dump_records(mach_headers, stacks, records, maximum_level=512):
    for record in records:
        print("address({}), size = {}".format(record.address, record.size))
        _maximum_level = maximum_level
        for frame in stacks[record.stack_id]:
            if not _maximum_level:
                break
            ret = resolve_symbol(mach_headers, frame)
            if len(ret):
                print('-> {}'.format(ret))
            _maximum_level -= 1

def filter_records(records, only_live=True, mininum_size=1024, sort=True):
    filter_records = {}
    for record in records:
        if record.size < mininum_size:
            continue
        if only_live:
            if record.address in filter_records:
                filter_records.pop(record.address)
            elif record.type_flag == '00000002':
                filter_records[record.address] = record
    ret = list(filter_records.values())
    if sort:
        ret = sorted(ret, key=lambda x: x.size, reverse=True)
    return ret

mach_headers = get_mach_headers()
stacks = get_stacks()
records = get_records()

dump_headers(mach_headers)
live_records = filter_records(records, mininum_size=0)
dump_records_total_size(live_records)
dump_records(mach_headers, stacks, live_records)