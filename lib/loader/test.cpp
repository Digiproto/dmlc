/*
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    test.cpp
 * Email:   lyjforjob@gmail.com
 * Date:    2013-12-30
 */

extern "C" {
#include <stdio.h>
#include <simics/base_types.h>
int test_init(void);
int test_mem_write(short len, uint32_t addr, uint32_t data);
int test_mem_read(short len, uint32_t addr, void* data);
}
#include "dml_device.h"
#include "dml_bus.h"
#include "json_loader.h"
#include "json_parser.h"

static void tmp_proxy_interrupt(conf_object_t *obj, int irq) {
}

static void tmp_proxy_interrupt_clear(conf_object_t *obj, int irq) {
}

class tmpDev : public DMLDevice {
public:
	tmpDev(const char *objname, const char *clsname)
		: DMLDevice(objname, clsname) {}
	void addBank(void) { addBankAccess(); }
};

class tmpBus : public DMLBus {
	void connectDevice(DMLDevice *dev, uint64_t startAddr, uint32_t len) {
		addPort(startAddr, len, dev);
		((tmpDev*) dev)->addBank();
	}
};

static DMLDevice* create_new_device(const char *objname, const char *clsname) {
	return new tmpDev(objname, clsname);
}

static tmpBus tmpbus;
int test_init(void)
{
	ExtraSystemCData data;
	data.bus = &tmpbus;
	data.create_fn = create_new_device;
	data.intr.intr = tmp_proxy_interrupt;
	data.intr.intr_clear = tmp_proxy_interrupt_clear;
	int ret = qdev_dynamic_conf("device.conf", (void*) &data);
	if(ret) {
		fprintf(stderr, "dynamic config device failed.\n");
		return ret;
	}
	return 0;
}

int test_mem_write(short len, uint32_t addr, uint32_t data)
{
	return tmpbus.writeBus(len, addr, data);
}

int test_mem_read(short len, uint32_t addr, void *data)
{
	return tmpbus.readBus(len, addr, data);
}

#ifdef TEST_DEBUG
int main(int argc, const char *argv[])
{
	int buf;
	test_init();
	test_mem_write(32, 0x1000, 0x0fedcba0);
	test_mem_read(32, 0x1000, &buf);
	return 0;
}
#endif
