#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ctypes import *
import ctypes
import os

yama_deserialize = cdll.LoadLibrary(os.getcwd() + "/yama_deserialize")
yama_deserialize.initialize(b"YAMA_FILE_SERIALIZE_TABLE")
yama_deserialize.read_stack.restype = ctypes.c_char_p
ret = yama_deserialize.read_stack(c_uint64(0x02db940000001fe1))
print(ret)
