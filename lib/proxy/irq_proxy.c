/*
 * irq_proxy.c: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <simics/dev_iface.h>
#include <simics/base_types.h>
#include <simics/core/object_model.h>
#include <simics/core/object_class.h>
#include <simics/core/object_iface.h>
#include <simics/core/object_resource.h>
#include <stdio.h>

typedef struct irq_proxy {
	conf_object_t             obj;
	conf_object_t            *sig_obj;
	const simple_interrupt_interface_t *signal;
	int                       sig_no;
}irq_proxy_t;

// ************** cpu_irq *****************
typedef void (*interrupt_fn)(conf_object_t *obj, int irq);
typedef void (*interrupt_clear_fn)(conf_object_t *obj, int irq);

extern interrupt_fn       dml_proxy_interrupt;
extern interrupt_clear_fn dml_proxy_interrupt_clear;

static void irq_proxy_cpu_irq(conf_object_t *obj, int irq) {
	dml_proxy_interrupt(obj, irq);
}

static void irq_proxy_cpu_irq_clear(conf_object_t *obj, int irq) {
	dml_proxy_interrupt_clear(obj, irq);
}

static const simple_interrupt_interface_t irq_proxy_cpu_irq_iface = {
	.interrupt = irq_proxy_cpu_irq,
	.interrupt_clear = irq_proxy_cpu_irq_clear,
};

static int cpu_irq_set(void *_, conf_object_t *obj, attr_value_t *val,  attr_value_t *_index) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	proxy->signal = (void*) &irq_proxy_cpu_irq_iface;
	return No_exp;
}

static attr_value_t cpu_irq_get(void *_, conf_object_t *obj, attr_value_t *_index) {
#if 0
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	attr_value_t attr;
	if(proxy->sig_obj) {
		attr = SIM_alloc_attr_list(2);
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_integer(proxy->sig_no));
		SIM_attr_list_set_item(&attr, 1, SIM_make_attr_obj(proxy->sig_obj));
	}
	return attr;
#endif
	attr_value_t attr;
	return attr;
}
// ************** cpu_irq end *****************

// ************** signal *****************
static void irq_proxy_signal_raise(conf_object_t *obj) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	proxy->signal->interrupt(proxy->sig_obj, proxy->sig_no);
}

static void irq_proxy_signal_lower(conf_object_t *obj) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	proxy->signal->interrupt_clear(proxy->sig_obj, proxy->sig_no);
}

static const signal_interface_t irq_proxy_iface =  {
	.signal_raise = irq_proxy_signal_raise,
	.signal_lower = irq_proxy_signal_lower,
};

static int signal_set(void *_, conf_object_t *obj, attr_value_t *val,  attr_value_t *_index) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	conf_object_t *peer = NULL;
	int irqnum = 0;
	
	if(SIM_attr_is_list(*val)) {
		irqnum = SIM_attr_integer(SIM_attr_list_item(*val, 0));
		peer = SIM_attr_object(SIM_attr_list_item(*val, 1));
	}else{
		return -1;
	}
	proxy->sig_no = irqnum;
	proxy->sig_obj = peer;
	proxy->signal = SIM_get_interface(peer, "device_irq");
	return No_exp;
}

static attr_value_t signal_get(void *_, conf_object_t *obj, attr_value_t *_index) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	attr_value_t attr;
	if(proxy->sig_obj) {
		attr = SIM_alloc_attr_list(2);
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_integer(proxy->sig_no));
		SIM_attr_list_set_item(&attr, 1, SIM_make_attr_obj(proxy->sig_obj));
	}
	return attr;
}
// ************** signal end *****************

// ************** interrupt *****************
static void interrupt(conf_object_t *obj, int irq) {
	//printf("%s: irq %d\n", __func__, irq);
	dml_proxy_interrupt(obj, irq);
}

static void interrupt_clear(conf_object_t *obj, int irq) {
	//printf("%s: irq %d\n", __func__, irq);
	dml_proxy_interrupt_clear(obj, irq);
}

static const simple_interrupt_interface_t irq_proxy_simple_interrupt_iface = {
	.interrupt = interrupt,
	.interrupt_clear = interrupt_clear,
};
// ************** interrupt end *****************

static const struct ConnectDescription irq_proxy_connects[] = {
	[0] = {}
};

static const struct InterfaceDescription irq_proxy_ifaces[] =  {
	[0] = (struct InterfaceDescription) {
		.name = "signal",
		.iface = &irq_proxy_iface,
	},
	[1] = (struct InterfaceDescription) {
		.name = "simple_interrupt",
		.iface = &irq_proxy_simple_interrupt_iface,
	},
	[2] = {}
};

static const struct AttributeDescription irq_proxy_attributes[] = {
	[0] = (struct AttributeDescription) {
		.name = "signal",
		.set = signal_set,
		.get = signal_get,
	},
	[1] = (struct AttributeDescription) {
		.name = "cpu_irq",
		.set = cpu_irq_set,
		.get = cpu_irq_get,
	},
	[2] = {}
};

static conf_object_t* irq_proxy_new_instance(const char *objname) {
	irq_proxy_t *proxy = (irq_proxy_t*) MM_ZALLOC(sizeof(*proxy));
	SIM_object_register(&proxy->obj, objname);
	return &proxy->obj;
}

static const class_resource_t irq_proxy_resource = {
	.ifaces = irq_proxy_ifaces,
	.connects = irq_proxy_connects,
	.attributes = irq_proxy_attributes,
};

static const class_data_t irq_proxy_data = {
	.new_instance = irq_proxy_new_instance,
	.resources = &irq_proxy_resource,
};

void init_local(void) {
	static conf_class_t conf_class = {
		.cls_name = "irq_proxy",
		.cls_data = &irq_proxy_data,
	};
	SIM_register_conf_class(conf_class.cls_name, &conf_class);
}
