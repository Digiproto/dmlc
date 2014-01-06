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
		struct con_intf *con = &dev->conlist[i];
		conf_object_t *condev = (conf_object_t*) SIM_get_conf_object(con->dev);
		if(!condev) {
			fprintf(stderr, "Unable to find %s\n", con->dev);
			return -1;
		}
		attr_value_t index = SIM_make_attr_integer(con->index);
		attr_value_t attr;
		if(con->port) {
			attr = SIM_alloc_attr_list(2);
			SIM_attr_list_set_item(&attr, 0, SIM_make_attr_obj(condev));
			SIM_attr_list_set_item(&attr, 1, SIM_make_attr_string(con->port));
		}else{
			attr = SIM_make_attr_obj(condev);
		}
		ret = SIM_obj_connect(obj, con->con, &attr, &index);
		if(ret) {
			fprintf(stderr, "have something wrong while setting %s con to %s.\n",
					dev->name, con->dev);
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
		if(strcmp(dev->irqlist[i].dev, "cpu") == 0) {
			ret = SIM_set_attr(obj, "cpu_irq", NULL, NULL);
			continue;
		}
		conf_object_t *irqdev = (conf_object_t*) SIM_get_conf_object(dev->irqlist[i].dev);
		if(!irqdev) {
			fprintf(stderr, "can't find irq dev %s\n", dev->irqlist[i].dev);
			return -1;
		}
		int irqnum = dev->irqlist[i].num;
		attr_value_t attr = SIM_alloc_attr_list(2);
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_integer(irqnum));
		SIM_attr_list_set_item(&attr, 1, SIM_make_attr_obj(irqdev));
		ret = SIM_set_attr(obj, "signal", &attr, NULL);
		if(ret != No_exp) {
			fprintf(stderr, "have something wrong while setting %s irq.\n", dev->name);
			return -1;
		}
	}
	return 0;
}

int device_attr(struct devargs *dev, void *extra)
{
	conf_object_t *attrdev;
	int i;

#define CHECK_ATTR_TYPE(str) \
	if(strcmp(attr->type, str) == 0)
#define CHECK_ATTR_TYPE_NOT_IMPLEMENT(str) \
	do{ \
		fprintf(stderr, "attribute not implement type %s\n", str); \
		return -1; \
	}while(0)

	if(dev->attr_num > 0) {
		attrdev = (conf_object_t*) SIM_get_conf_object(dev->name);
		assert(attrdev);
		for(i = 0; i < dev->attr_num; i++) {
			struct attr_intf *attr = &dev->attrlist[i];
			attr_value_t value;
			CHECK_ATTR_TYPE("string") {
				value = SIM_make_attr_string(attr->value);
			}
			else CHECK_ATTR_TYPE("boolean") {
				bool tmp;
				if(strcmp(attr->value, "true") == 0) {
					tmp = true;
				}else if(strcmp(attr->value, "false") == 0) {
					tmp = false;
				}else{
					fprintf(stderr, "%s set a error boolean value (%s)\n",
						attr->attr, attr->value);
					exit(-1);
				}
				value = SIM_make_attr_boolean(tmp);
			}
			else CHECK_ATTR_TYPE("integer") {
				int64_t tmp;
				tmp = strtoll(attr->value, NULL, 0);
				value = SIM_make_attr_integer(tmp);
			}
			else CHECK_ATTR_TYPE("floating") {
				double tmp;
				tmp = strtod(attr->value, NULL);
				value = SIM_make_attr_floating(tmp);
			}
			else CHECK_ATTR_TYPE("object") {
				conf_object_t *tmp;
				tmp = (conf_object_t*) SIM_get_conf_object(attr->value);
				value = SIM_make_attr_obj(tmp);
			}
			else CHECK_ATTR_TYPE("invalid") {
				value = SIM_make_attr_invalid();
			}
			else CHECK_ATTR_TYPE("list") {
				CHECK_ATTR_TYPE_NOT_IMPLEMENT("list");
			}
			else CHECK_ATTR_TYPE("data") {
				CHECK_ATTR_TYPE_NOT_IMPLEMENT("data");
			}
			else CHECK_ATTR_TYPE("dict") {
				CHECK_ATTR_TYPE_NOT_IMPLEMENT("dict");
			}
			else CHECK_ATTR_TYPE("nil") {
				CHECK_ATTR_TYPE_NOT_IMPLEMENT("nil");
			}
			else {
				fprintf(stderr, "%s setting a unknown attribute type %s", dev->name, attr->type);
				return -1;
			}
			attr_value_t idx = SIM_make_attr_integer(attr->index);
			int ret = SIM_set_attr(attrdev, attr->attr, &value, &idx);
			if(ret) {
				fprintf(stderr, "setting %s attribute %s failed\n", dev->name, attr->attr);
				return ret;
			}
		}
	}

	return 0;
#undef CHECK_ATTR_TYPE
#undef CHECK_ATTR_TYPE_NOT_IMPLEMENT
}
