# -*- coding: utf-8 -*-

# © Copyright 2013 eJim Lee. All Rights Reserved.
# File:    libtest_framework.py
# Email:   lyjforjob@gmail.com
# Date:    2013-10-22

import os
import ctypes

class common_framework():
	Init_complete = False
	def __init__(self):
		return
	def test_init(self):
		retval = self.libtest.test_init()
		if retval != 0:
			print "init libtest have error"
			self.Init_complete = False
			return False
		print "init libtest have finished"
		self.Init_complete = True
		return True
	def test_mem_write(self, len, addr, data):
		if self.Init_complete == False:
			return False
		retval = self.libtest.test_mem_write(ctypes.c_int16(len), ctypes.c_int32(addr), ctypes.c_int32(data));
		if retval != 0:
			print "write data into 0x%x failed" % addr
			return False
		return True
	def test_mem_read(self, len, addr):
		data = ctypes.c_int32()
		if self.Init_complete == False:
			return False, data.value
		retval = self.libtest.test_mem_read(ctypes.c_int16(len), ctypes.c_int32(addr), ctypes.byref(data));
		if retval != 0:
			print "read data on 0x%x failed" % addr
			return False, data.value
		return True, data.value

# load some module
class skyeye(common_framework):
	def __init__(self, lib_path, libsuf):
		common_framework.__init__(self)
		libcommon_path = os.path.join(lib_path, "libcommon%s" % libsuf)
		libtest_path = os.path.join(lib_path, "libtest%s" % libsuf)
		# libcommon
		self.libcommon = ctypes.CDLL(libcommon_path, ctypes.RTLD_GLOBAL)
		# libtest
		self.libtest = ctypes.CDLL(libtest_path, ctypes.RTLD_GLOBAL)
		self.libtest.test_init.restype = ctypes.c_int
		self.libtest.test_mem_write.restype = ctypes.c_int
		self.libtest.test_mem_read.restype = ctypes.c_int

class systemc(common_framework):
	def __init__(self, lib_path, libsuf):
		common_framework.__init__(self)
		lib_path = os.path.join(lib_path, "lib")
		libcommon_path = os.path.join(lib_path, "libdml_common%s" % libsuf)
		libjsonloader_path = os.path.join(lib_path, "libjson_loader%s" % libsuf)
		libtest_path = os.path.join(lib_path, "libtest%s" % libsuf)
		# libdml_commmon
		self.libtest = ctypes.CDLL(libcommon_path, ctypes.RTLD_GLOBAL)
		# libjsonloader
		self.libtest = ctypes.CDLL(libjsonloader_path, ctypes.RTLD_GLOBAL)
		# libtest
		self.libtest = ctypes.CDLL(libtest_path, ctypes.RTLD_GLOBAL)
		self.libtest.test_init.restype = ctypes.c_int
		self.libtest.test_mem_write.restype = ctypes.c_int
		self.libtest.test_mem_read.restype = ctypes.c_int

class qemu(common_framework):
	def __init__(self, lib_path, libsuf):
		common_framework.__init__(self)
		libtest_path = os.path.join(lib_path, "libqemu-system-arm%s" % libsuf)
		# libqemu-system-arm
		self.libtest = ctypes.CDLL(libtest_path, ctypes.RTLD_GLOBAL)
		self.libtest.test_init.restype = ctypes.c_int
		self.libtest.test_mem_write.restype = ctypes.c_int
		self.libtest.test_mem_read.restype = ctypes.c_int
