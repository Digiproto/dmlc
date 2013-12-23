/*
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    dml_device.cpp
 * Email:   lyjforjob@gmail.com
 * Date:    2013-12-22
 */

extern "C" {
#include <stdio.h>
#include <simics/base_types.h>
#include <simics/bank_access.h>
#include <simics/core/object_model.h>
}
#include "dml_device.h"

DMLDevice::DMLDevice(const char *objname, const char *classname) {
	obj = SIM_pre_conf_object(objname, classname);
	iface = NULL;
}

conf_object* DMLDevice::getObj(void) {
	return obj;
}

void DMLDevice::addBankAccess(void) {
	if(!iface) {
		iface = SIM_get_bank_access(obj, 0);
		if(!iface)
			fprintf(stderr, "%s don't have bank access\n", obj->name);
		/* not implement for more */
	}
}

int DMLDevice::readBankAccess(uint32_t addr, void *buf, size_t size) {
	return iface->read(obj, addr, buf, size);
}

int DMLDevice::writeBankAccess(uint32_t addr, const void *buf, size_t size) {
	return iface->write(obj, addr, buf, size);
}
