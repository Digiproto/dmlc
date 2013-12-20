/*
 * object_iface.c: 
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
#include "simics/core/object_class.h"
#include "simics/core/object_model.h"
#include "simics/core/object_iface.h"

static const void *qdev_find_interface(const struct InterfaceDescription *id, const char *iface_name){
    const void *iface = NULL;

    for ( ; id->name; id++) {
        if (!strcmp(id->name, iface_name)) {
            iface = id->iface;
            return iface;
        }
    }
    return NULL;
}

static const void *obj_get_interface_internal(conf_object_t *dev, const char *iface)
{
    const struct InterfaceDescription *id;
    const struct conf_class *dc;
	dc = dev->class_data;

    id = dc->cls_data->resources->ifaces;
    return qdev_find_interface(id, iface);
}

static const void *obj_get_port_internal(conf_object_t *dev, const char *iface, const char *port)
{
    const struct PortDescription *pd;
    const struct InterfaceDescription *id;
    const struct conf_class *dc = dev->class_data;

    pd = dc->cls_data->resources->ports;
    for ( ; pd->name; pd++) {
            if (!strcmp(pd->name, port)) {
                id = pd->id;
                return qdev_find_interface(id, iface);
            }
    }
    return NULL;
}

const void *SIM_get_interface(conf_object_t *dev, const char *iface)
{
    return obj_get_interface_internal(dev, iface);
}

const void *SIM_get_port_interface(conf_object_t *dev, const char *iface, const char *port)
{
    return obj_get_port_internal(dev, iface, port);
}

/*connect qdev1' connect to dev2 */
static int obj_connect_internal(conf_object_t *dev1, const char *connect, conf_object_t *dev2, const char *port, int idx)
{
    int ret = 0;
    const struct conf_class  *dc = dev1->class_data;
    const struct ConnectDescription *con;
    connect_set_t set = NULL;

    con = dc->cls_data->resources->connects;
    for (; con->name; con++) {
        if (!strcmp(connect, con->name)) {
            set = con->set;
            break;
        }
    }
    if (set) {
        return set(dev1, dev2, port, idx);
    }
    return ret;
}

int SIM_obj_connect(conf_object_t *dev1, const char *name, conf_object_t *dev2)
{
    return obj_connect_internal(dev1, name, dev2, NULL, 0);
}

int SIM_connect_port(conf_object_t *dev1, const char *name, conf_object_t *dev2, const char *port)
{
    return obj_connect_internal(dev1, name, dev2, port, 0);
}

int SIM_set_attr(conf_object_t *dev, const char *name, attr_value_t val)
{
	const struct conf_class *cd = dev->class_data;
	const struct AttributeDescription *attr;
	sim_set_attr set = NULL;

	attr = cd->cls_data->resources->attributes;
	for (; attr->name; attr++) {
		if(!strcmp(attr->name, name)) {
			set = attr->set;
			break;
		}
	}
	if(set) {
		return set(dev, val);
	}
	return 0;
}
