/*
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    json_device.cpp
 * Email:   lyjforjob@gmail.com
 * Date:    2013-12-10
 */

extern "C" {
#include <stdio.h>
#include <simics/base_types.h>
#include <simics/bank_access.h>
#include <simics/core/object_iface.h>
#include <simics/core/object_model.h>
#include <simics/core/object_class.h>
}

#include "json_device.h"
#include "dml_device.h"
#include "dml_bus.h"
#include "json_loader.h"

interrupt_fn       dml_proxy_interrupt;
interrupt_clear_fn dml_proxy_interrupt_clear;

int device_create(struct devargs *dev, void *extra)
{
	DMLDevice *newdev;
	ExtraSystemCData *data = (ExtraSystemCData*) extra;
	DMLBus *m_bus = data->bus;
	NewDevice create_new_device = data->create_fn;
	dml_proxy_interrupt = data->intr.intr;
	dml_proxy_interrupt_clear = data->intr.intr_clear;
	int idx;

	newdev = create_new_device(dev->name, dev->devclass);
	if(dev->has_addr) {
		if(!dev->has_len) {
			fprintf(stderr, "you have to set %s address map length on systemc.\n", dev->name);
			return -1;
		}
		m_bus->connectDevice(newdev, dev->address, dev->length);
	}

	return 0;
}

int device_con(struct devargs *dev, void *extra)
{
	conf_object_t *obj = (conf_object_t*) SIM_get_conf_object(dev->name);
	int i, ret;

	for(i = 0; i < dev->con_num; i++) {
		conf_object_t *condev = (conf_object_t*) SIM_get_conf_object(dev->conlist[i].dev);
		attr_value_t attr = SIM_make_attr_obj(condev);
		ret = SIM_obj_connect(obj, dev->conlist[i].con, &attr, NULL);
		if(ret) {
			fprintf(stderr, "have something wrong while setting %s con to %s.\n",
					dev->name, dev->conlist[i].dev);
			return -1;
		}
	}
	return 0;
}

int device_irq(struct devargs *dev, void *extra)
{
	int ret;
	conf_object_t *obj = (conf_object_t*) SIM_get_conf_object(dev->name);
	int i;

	for(i = 0; i < dev->irq_num; i++) {
		conf_object_t *irqdev = (conf_object_t*) SIM_get_conf_object(dev->irqlist[i].dev);
		int irqnum = dev->irqlist[i].num;
		attr_value_t attr = SIM_alloc_attr_list(2);
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_integer(irqnum));
		SIM_attr_list_set_item(&attr, 1, SIM_make_attr_obj(irqdev));
		ret = SIM_set_attr(obj, "signal", &attr, NULL);
		if(ret != No_exp) {
			fprintf(stderr, "have something wrong while setting %s irq.\n", dev->name);
			return -1;
		}
#if 0
		/* setting irq number */
		ret = SIM_set_attr(obj, "sig_num", (void*)((size_t)dev->irqlist[i].num));
		if(ret != No_exp) {
			fprintf(stderr, "have something wrong while setting %s irq.\n", dev->name);
			return -1;
		}
		/* setting irq target */
		ret = SIM_obj_connect(obj, "signal", irqdev);
		if(ret != No_exp) {
			fprintf(stderr, "have something wrong while setting %s irq.\n", dev->name);
			return -1;
		}
#endif
	}
	return 0;
}
