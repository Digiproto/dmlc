/*
 * object_resource.h: 
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

#ifndef __OBJECT_RESOURCE_H__ 
#define __OBJECT_RESOURCE_H__  
#include <simics/core/object_iface.h>
#include <simics/bank_access.h>

typedef struct resource_descriptor {
    const struct InterfaceDescription *ifaces;

    /*device connects*/
    const struct ConnectDescription *connects;
    /*device port interfaces*/
    const struct PortDescription *ports;
} class_resource_t;

typedef struct class_data {
        struct conf_object *(*new_instance)(const char *name);
        void (*finalize_instance)(struct conf_object *obj);
        void (*pre_delete_instance)(struct conf_object *obj);
        int (*delete_instance)(struct conf_object *obj);
		const class_resource_t *resources;
		const struct bank_access_description *bank_access;
        const char           *description;
        const char           *class_desc;	
} class_data_t;

#endif /* __OBJECT_RESOURCE_H__ */
