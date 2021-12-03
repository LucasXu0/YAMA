#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import bisect
import subprocess
from colorama import Fore, Style
import math
import ctypes

# input_dir
ARCH = sys.argv[1]
IOS_DEVICESUPPORT_PATH = sys.argv[2]
APP_DSYM_PATH = sys.argv[3]
INTPUT_DIR = sys.argv[4]
MODE = sys.argv[5]

YAMA_FILE_MACH_HEADER       = INTPUT_DIR + '/YAMA_FILE_MACH_HEADER'
YAMA_FILE_RECORDS           = INTPUT_DIR + '/YAMA_FILE_RECORDS'
YAMA_FILE_STACKS            = INTPUT_DIR + '/YAMA_FILE_STACKS'
YAMA_FILE_SERIALIZE_TABLE   = INTPUT_DIR + '/YAMA_FILE_SERIALIZE_TABLE'

def convert_size(size_bytes):
   if size_bytes == 0:
       return "0B"
   size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
   i = int(math.floor(math.log(size_bytes, 1024)))
   p = math.pow(1024, i)
   s = round(size_bytes / p, 2)
   return "%s%s" % (s, size_name[i])

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
            print(Fore.CYAN + 'could not find library(' + path + ') in your local path' + Style.RESET_ALL)
        minimum_slide = min(minimum_slide, slide)
    # print('bad case in [convert_mach_header_to_local_symbols] = ' + str(bad_case))
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
    if not os.path.exists(symbol_path) or symbol_path == '/':
        return address
    command = 'atos' + ' -arch ' + ARCH + ' -o "' + symbol_path + '" -l ' + str_to_hex(slide) + ' ' + str_to_hex(address)
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
        records.append(Record(type_flag=int(raw_record[0]), stack_id=raw_record[1], size=int(raw_record[2]), address=raw_record[3]))
    return records

def get_deserialize():
    yama_deserialize = ctypes.cdll.LoadLibrary("yama_deserialize")
    ret = yama_deserialize.initialize(str.encode(YAMA_FILE_SERIALIZE_TABLE))
    if ret == 0:
        print('get_deserialize fail')
    yama_deserialize.read_stack.restype = ctypes.c_char_p
    return yama_deserialize

def dump_records_total_size(records):
    total_size = 0
    for record in records:
        total_size += record.size
    print(Fore.YELLOW + '\n[YAMA] Total Size = {}'.format(convert_size(total_size)) + Style.RESET_ALL)

def dump_records_with_stacks(mach_headers, stacks, records, maximum_level=512):
    for record in records:
        print("address({}), size = {}".format(record.address, convert_size(record.size)))
        _maximum_level = maximum_level
        if record.stack_id not in stacks:
            continue
        for frame in stacks[record.stack_id]:
            if not _maximum_level:
                break
            ret = resolve_symbol(mach_headers, frame, only_in_main_executable=(MODE == 'lite'))
            if len(ret):
                print('-> {}'.format(ret))
            _maximum_level -= 1

def dump_records_with_deserialize(mach_header, deserialize, records, maximum_level=512):
    for record in records:
        print("address({}), size = {}".format(record.address, convert_size(record.size)))
        _maximum_level = maximum_level
        stacks = deserialize.read_stack(ctypes.c_uint64(0x02db940000001fe1))
        stacks = stacks.decode('utf-8').split(' ')
        for frame in stacks:
            if not _maximum_level:
                break
            ret = resolve_symbol(mach_headers, frame, only_in_main_executable=(MODE == 'lite'))
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
            elif record.type_flag & 2 or record.type_flag & 16 and record.type_flag & 128 == 0:
                filter_records[record.address] = record
    ret = list(filter_records.values())
    if sort:
        ret = sorted(ret, key=lambda x: x.size, reverse=True)
    return ret

mach_headers = get_mach_headers()
# stacks = get_stacks()
records = get_records()
deserialize = get_deserialize()

# dump_headers(mach_headers)
live_records = filter_records(records, mininum_size=0)
dump_records_total_size(live_records)
# dump_records_with_stacks(mach_headers, stacks, live_records, 5)
dump_records_with_deserialize(mach_headers, deserialize, live_records, 5)