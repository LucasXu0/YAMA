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
            bad_case += 1
            print(symbol_path)
            print(Fore.CYAN + 'could not find library(' + path + ') in your local path')
        minimum_slide = min(minimum_slide, slide)
    print(Style.RESET_ALL)
    print('bad case in [convert_mach_header_to_local_symbols] = ' + str(bad_case))
    mach_headers.append((minimum_slide, APP_DSYM_PATH))
    mach_headers.sort()
    return mach_headers

def resolve_symbol(mach_headers, address):
    def str_to_hex(str):
        return hex(int(str, 16))

    position = bisect.bisect_left(mach_headers, (address, 'AAAAA'))
    (slide, symbol_path) = mach_headers[position - 1]
    command = 'atos' + ' -arch' + ' arm64' + ' -o "' + symbol_path + '" -l ' + str_to_hex(slide) + ' ' + str_to_hex(address)
    # print(command)
    subprocess.call(command, shell=True)

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

def filter_live_records(records):
    filter_records = {}
    for record in records:
        if record.address in filter_records:
            filter_records.pop(record.address)
        elif record.type_flag == '00000002':
            filter_records[record.address] = record
    return filter_records.values()

def dump_live_objcect(mach_headers, stacks, records, minimum_size, maximum_level):
    for record in records:
        if record.size < minimum_size:
            continue
        print("address({}), size = {}".format(record.address, record.size))
        stack = stacks[record.stack_id]
        _maximum_level = maximum_level
        for frame in stack:
            if not _maximum_level:
                break
            resolve_symbol(mach_headers, frame)
            _maximum_level -= 1

mach_headers = get_mach_headers()

for header in mach_headers:
    print(header)

stacks = get_stacks()
records = get_records()
# filter_records = filter_live_records(records)
# print(len(records))

# dump_live_objcect(mach_headers, stacks, records, 79 - 1, 5)