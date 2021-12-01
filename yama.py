#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import bisect

print '参数个数为:', len(sys.argv), '个参数。'
print '参数列表:', str(sys.argv)

headers = sys.argv[1]

bad_case = 0
# 1. 查找符号表位置
def find_debug_symbols_location(headers):
    global bad_case
    symbols = []
    for header in open(headers):
        header = header.strip('\n').split(' ')
        count, address, path = header[0], header[1], header[2]
        # FIXME: hard code
        symbol_path = '/Users/xurunkang/Library/Developer/Xcode/iOS DeviceSupport/15.1.1 (19B81) arm64e/Symbols' + path
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
for symbol in symbols:
    print(symbol)

def find_address_in_which_symbol(symbols, address):
    return bisect.bisect(symbols, address)


# 2. 还原符号
print(find_address_in_which_symbol(symbols, 0x00000001f782a000))