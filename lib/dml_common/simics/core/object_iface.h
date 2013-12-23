/*
 * object_iface.h: 
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

#ifndef __OBJECT_IFACE_H__ 
#define __OBJECT_IFACE_H__  
#include <stdlib.h>
#include "../base_types.h"
struct InterfaceDescription {
    const char *name;
    const void *iface;
};

struct PortDescription {
    const char *name;
    const struct InterfaceDescription *id;
};

typedef int (*connect_set_t)(struct conf_object *dev1, struct conf_object *dev2, const char *port, int idx);
typedef int (*connect_get_t)(struct conf_object *dev1, struct conf_object **dev2, char **port,int idx);

struct ConnectDescription {
    const char *name;
    connect_set_t set;
    connect_get_t get;
};

typedef void* attr_value_t;
typedef int          (*sim_set_attr)(struct conf_object *obj, attr_value_t attr);
typedef attr_value_t (*sim_get_attr)(struct conf_object *obj);

struct AttributeDescription {
	const char *name;
	sim_set_attr set;
	sim_get_attr get;
};

const void *SIM_get_interface(conf_object_t *dev, const char *iface);
const void *SIM_get_port_interface(conf_object_t *dev, const char *iface, const char *port);
int SIM_obj_connect(conf_object_t *dev1, const char *name, conf_object_t *dev2);
int SIM_connect_port(conf_object_t *dev1, const char *name, conf_object_t *dev2, const char *port);
int SIM_set_attr(conf_object_t *dev, const char *name, attr_value_t attr);
#endif /* __OBJECT_IFACE_H__ */
