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

typedef struct irq_proxy {
	conf_object_t             obj;
	conf_object_t            *sig_obj;
	const simple_interrupt_interface_t *signal;
	int                       sig_no;
}irq_proxy_t;

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

static conf_object_t* irq_proxy_new_instance(const char *objname) {
	irq_proxy_t *proxy = (irq_proxy_t*) MM_ZALLOC(sizeof(*proxy));
	SIM_object_register(&proxy->obj, objname);
	return &proxy->obj;
}

static int sig_num_set(conf_object_t *obj, attr_value_t val) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	proxy->sig_no = (int) ((long) val);
	return 0;
}

static attr_value_t sig_num_get(conf_object_t *obj) {
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	return (attr_value_t*) ((long) proxy->sig_no);
}

static int signal_set(conf_object_t *obj, conf_object_t *peer,
		const char *port, int idx)
{
	irq_proxy_t *proxy = (irq_proxy_t *)obj;
	proxy->sig_obj = peer;
	if(port) {
		proxy->signal = SIM_get_port_interface(peer, "device_irq", port);
	}else{
		proxy->signal = SIM_get_interface(peer, "device_irq");
	}
	return 0;
}

static int signal_get(conf_object_t *obj, conf_object_t **peer,
		char **port, int idx)
{
	return 0;
}

static const struct ConnectDescription irq_proxy_connects[] = {
	[0] = {
		.name = "signal",
		.set = signal_set,
		.get = signal_get,
	},
	[1] = {}
};

static const struct InterfaceDescription irq_proxy_ifaces[] =  {
	[0] = (struct InterfaceDescription) {
		.name = "signal",
		.iface = &irq_proxy_iface,
	},
	[1] = {}
};

static const struct AttributeDescription irq_proxy_attributes[] = {
	[0] = (struct AttributeDescription) {
		.name = "sig_num",
		.set = sig_num_set,
		.get = sig_num_get,
	},
	[1] = {}
};

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
