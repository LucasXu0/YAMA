#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ctypes import *
import os

yama_deserialize = cdll.LoadLibrary(os.getcwd() + "/yama_deserialize")
yama_deserialize.print_hello_world()