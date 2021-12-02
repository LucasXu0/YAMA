#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import bisect
import subprocess
from threading import Thread
from time import sleep

print('argument count = ' + str(len(sys.argv)))

headers = sys.argv[1]
stack_id_maps_path = sys.argv[2]

bad_case = 0
# 1. 查找符号表位置
def find_debug_symbols_location(headers):
    global bad_case
    symbols = []
    for header in open(headers):
        header = header.strip('\n').split(' ')
        count, address, path = header[0], header[1], header[2]
        # FIXME: hard code
        symbol_path = '/Users/runkang.xu/Library/Developer/Xcode/iOS DeviceSupport/14.3 (18C66)/Symbols' + path
        # print(count, address, symbol_path)
        ret = os.path.exists(symbol_path)
        # print(ret)
        if not ret:
            bad_case += 1
        else:
            symbols.append((address, symbol_path))
    symbols.sort()
    return symbols


symbols = find_debug_symbols_location(headers)
print('无法查找文件个数 = ' + str(bad_case))
for i in range(len(symbols)):
    print(i, symbols[i])

def find_address_in_which_symbol(symbols, address):
    return bisect.bisect_left(symbols, (address, 'A'))

def aasync(f):
    def wrapper(*args, **kwargs):
        thr = Thread(target=f, args=args, kwargs=kwargs)
        thr.start()
    return wrapper

@aasync
def convert_address_to_symbol(symbols, address):
    symbol_index = find_address_in_which_symbol(symbols, address)
    symbol = symbols[symbol_index - 1]
    command = 'atos -arch arm64 -o "' + symbol[1] + '" -l ' + symbol[0] + ' ' + address
    # ret = os.system(command)
    subprocess.call(command, shell=True)
    # print(command + ' && ')
    # return ret

def convert_stack_id_map(stack_id_maps_path):
    stack_id_map = {}
    for line in open(stack_id_maps_path):
        line = line.strip('\n').split(' ')
        stack_id_map[line[0]] = line[1:]
    return stack_id_map

def convert_stack_id_map_to_symbol(stack_id_map, symbols):
    for key in stack_id_map:
        print(key)
        for line in stack_id_map[key]:
            # print(line)
            if len(line) and line != '0x0000000000000000':
                convert_address_to_symbol(symbols, line)
            else:
                print(line)
        print('==============================')


# 2. 还原符号
# print(find_address_in_which_symbol(symbols, '0x00000001abb7c350'))

# convert_address_to_symbol(symbols, '0x00000001abb7c350')
stack_id_map = convert_stack_id_map(stack_id_maps_path)
# print(stack_id_map)
convert_stack_id_map_to_symbol(stack_id_map, symbols)

exit()