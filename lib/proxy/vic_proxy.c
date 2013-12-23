/*
 * vic_proxy.c:
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

//#include "skyeye_signal.h"

typedef void (*interrupt_fn)(conf_object_t *obj, int irq);
typedef void (*interrupt_clear_fn)(conf_object_t *obj, int irq);

extern interrupt_fn       dml_proxy_interrupt;
extern interrupt_clear_fn dml_proxy_interrupt_clear;

typedef struct vic_proxy {
	conf_object_t obj;
	struct {
		conf_object_t *obj;
		const char *port;
		const simple_interrupt_interface_t *interrupt;
	} vic[2];
	struct {
		conf_object_t *obj;
		const char *port;
		const int_update_interface_t *int_update;
	} int_update[2];
} vic_proxy_t;

/*
static unsigned int count = 0;
static interrupt_signal_t signal;
static void update_int(void) {
	//if((count % 2) == 0) {
		signal.arm_signal.reset = Prev_level;
		send_signal(&signal);
	//}
}
*/

static void vic_proxy_interrupt(conf_object_t *obj, int irq) {
	//vic_proxy_t *proxy = (vic_proxy_t *)obj;

	//count++;
	/*
	if(irq) {
		signal.arm_signal.firq = Low_level;
	} else {
		signal.arm_signal.irq = Low_level;
	}
	update_int();
	*/
	dml_proxy_interrupt(obj, irq);
}

static void vic_proxy_interrupt_clear(conf_object_t *obj, int irq) {
	//vic_proxy_t *proxy = (vic_proxy_t *)obj;

	//count++;
	/*
	if(irq) {
		signal.arm_signal.firq = High_level;
	} else {
		signal.arm_signal.irq = High_level;
	}
	update_int();
	*/
	dml_proxy_interrupt_clear(obj, irq);
}

static int set_intr(conf_object_t *obj,
		conf_object_t *peer, const char *port, int n) {
	vic_proxy_t *proxy = (vic_proxy_t*) obj;
	proxy->vic[n].obj = peer;
	proxy->vic[n].interrupt = SIM_get_interface(peer, "simple_interrupt");
	return 0;
}

static int set_intr0(conf_object_t *obj,
		conf_object_t *peer, const char *port, int idx) {
	return set_intr(obj, peer, port, 0);
}

static int set_intr1(conf_object_t *obj,
		conf_object_t *peer, const char *port, int idx) {
	return set_intr(obj, peer, port, 1);
}

static int get_intr0(conf_object_t *obj,
		conf_object_t **peer, char **port, int idx) {
	return 0;
}

static int get_intr1(conf_object_t *obj,
		conf_object_t **peer, char **port, int idx) {
	return 0;
}

static int set_update(conf_object_t *obj,
		conf_object_t *peer, const char *port, int n) {
	vic_proxy_t *proxy = (vic_proxy_t*) obj;
	proxy->int_update[n].obj = peer;
	proxy->int_update[n].int_update = SIM_get_interface(peer, "int_update");
	return 0;
}

static int set_update0(conf_object_t *obj,
		conf_object_t *peer, const char *port, int n) {
	return set_update(obj, peer, port, 0);
}

static int set_update1(conf_object_t *obj,
		conf_object_t *peer, const char *port, int n) {
	return set_update(obj, peer, port, 1);
}

static int get_update0(conf_object_t *obj,
		conf_object_t **peer, char **port, int idx) {
	return 0;
}

static int get_update1(conf_object_t *obj,
		conf_object_t **peer, char **port, int idx) {
	return 0;
}

const struct ConnectDescription vic_proxy_connects[] = {
	[0] = (struct ConnectDescription) {
		.name = "vic_intr0",
		.set = set_intr0,
		.get = get_intr0,
	},
	[1] = (struct ConnectDescription) {
		.name = "vic_intr1",
		.set = set_intr1,
		.get = get_intr1,
	},
	[2] = (struct ConnectDescription) {
		.name = "vic_update0",
		.set = set_update0,
		.get = get_update0,
	},
	[3] = (struct ConnectDescription) {
		.name = "vic_update1",
		.set = set_update1,
		.get = get_update1,
	},
	[4] = (struct ConnectDescription) {
		.name = NULL,
		.set = NULL,
		.get = NULL,
	},
};

static void proxy_interrupt(conf_object_t *obj, int line) {
	vic_proxy_t *proxy = (vic_proxy_t *)obj;
	if(line  < 32) {
		proxy->vic[0].interrupt->interrupt(proxy->vic[0].obj, line);
	} else {
		proxy->vic[1].interrupt->interrupt(proxy->vic[1].obj, line - 32);
	}
}

static void proxy_interrupt_clear(conf_object_t *obj, int line) {
	vic_proxy_t *proxy = (vic_proxy_t *) obj;
	if(line  < 32) {
		proxy->vic[0].interrupt->interrupt_clear(proxy->vic[0].obj, line);
	} else {
		proxy->vic[1].interrupt->interrupt_clear(proxy->vic[1].obj, line - 32);
	}
}

/*
static void proxy_update_int(conf_object_t *obj) {
	vic_proxy_t* proxy = (vic_proxy_t*) obj;
	proxy->int_update[0].int_update->int_update(proxy->int_update[0].obj);
	proxy->int_update[1].int_update->int_update(proxy->int_update[1].obj);
}
*/

/*
static const int_update_interface_t int_update_iface = {
	.int_update = proxy_update_int,
};
*/

static const simple_interrupt_interface_t vic_proxy_interrupt_iface = {
	.interrupt = vic_proxy_interrupt,
	.interrupt_clear = vic_proxy_interrupt_clear,
};

static const simple_interrupt_interface_t proxy_interrupt_iface = {
	.interrupt = proxy_interrupt,
	.interrupt_clear = proxy_interrupt_clear,
};

static const struct InterfaceDescription vic_proxy_ifaces[] = {
	[0] = (struct InterfaceDescription) {
		.name = "simple_interrupt",
		.iface = &vic_proxy_interrupt_iface,
	},
	[1] = (struct InterfaceDescription) {
		.name = "device_irq",
		.iface = &proxy_interrupt_iface,
	},
	[2] = {}
};

static const struct AttributeDescription vic_proxy_attributes[] = {
	[0] = {}
};

static const class_resource_t vic_proxy_resource = {
	.ifaces = vic_proxy_ifaces,
	.connects = vic_proxy_connects,
	.attributes = vic_proxy_attributes,
};

static conf_object_t* vic_proxy_new_instance(const char *objname) {
	vic_proxy_t *proxy = (vic_proxy_t*) MM_ZALLOC(sizeof(*proxy));
	SIM_object_register(&proxy->obj, objname);
	return &proxy->obj;
}

static const class_data_t vic_proxy_data = {
	.new_instance = vic_proxy_new_instance,
	.resources = &vic_proxy_resource,
};

void init_local(void) {
	static conf_class_t conf_class = {
		.cls_name = "vic_proxy",
		.cls_data = &vic_proxy_data,
	};
	SIM_register_conf_class(conf_class.cls_name, &conf_class);
}
