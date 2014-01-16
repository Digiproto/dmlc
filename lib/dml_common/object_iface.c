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

/**
 * @brief qdev_find_interface : find interface from interface array
 *
 * @param id: the index of array
 * @param iface_name : interface name to be finded
 *
 * @return : interface description
 */
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

/**
 * @brief obj_get_interface_internal : get interface from device
 *
 * @param dev : device object
 * @param iface : interface to be finded
 *
 * @return : interface object
 */
static const void *obj_get_interface_internal(conf_object_t *dev, const char *iface)
{
    const struct InterfaceDescription *id;
    const struct conf_class *dc;
	dc = dev->class_data;

    id = dc->cls_data->resources->ifaces;
    return qdev_find_interface(id, iface);
}

/**
 * @brief obj_get_port_internal : find interface from port
 *
 * @param dev : device object
 * @param iface : interface of port
 * @param port : port to find
 *
 * @return : interface object
 */
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

/**
 * @brief SIM_get_interface : realize the SIM_get_interface to get the interface object
 *
 * @param dev : device object
 * @param iface : the interface name
 *
 * @return : interface object
 */
const void *SIM_get_interface(conf_object_t *dev, const char *iface)
{
    return obj_get_interface_internal(dev, iface);
}

/**
 * @brief SIM_get_port_interface : realize the SIM_get_port_interface method
 *
 * @param dev : the device object
 * @param iface : interface name
 * @param port : port name
 *
 * @return : interface object of port
 */
const void *SIM_get_port_interface(conf_object_t *dev, const char *iface, const char *port)
{
    return obj_get_port_internal(dev, iface, port);
}

/*connect qdev1' connect to dev2 */

/**
 * @brief obj_connect_internal : realize the connect function to connect two object
 *
 * @param dev1 : the first object
 * @param connect : the connect name
 * @param value : the value of another obejet
 * @param index : index of another object
 *
 * @return : return vaule has no any effect
 */
static int obj_connect_internal(conf_object_t *dev1, const char *connect, attr_value_t *value, attr_value_t *index)
{
    int ret = 0;
    const struct conf_class  *dc = dev1->class_data;
    const struct ConnectDescription *con;
    sim_set_attr set = NULL;

    con = dc->cls_data->resources->connects;
    for (; con->name; con++) {
        if (!strcmp(connect, con->name)) {
            set = con->set;
            break;
        }
    }
    if (set) {
        return set(NULL, dev1, value, index);
    }
    return ret;
}

/**
 * @brief SIM_obj_connect : connect device with another object
 *
 * @param dev1 : the object of device
 * @param name : name of connected object name
 * @param value : value of connect object
 * @param index : index of object
 *
 * @return : return value has no any effect
 */
int SIM_obj_connect(conf_object_t *dev1, const char *name, attr_value_t *value, attr_value_t *index)
{
    return obj_connect_internal(dev1, name, value, index);
}

/**
 * @brief SIM_connect_port : realize the method Sim_connect_port to connect device and port
 *
 * @param dev1 : the object of device
 * @param name : name of port
 * @param value : value of port
 * @param index : the index of port array
 *
 * @return : return value has no any effect
 */
int SIM_connect_port(conf_object_t *dev1, const char *name, attr_value_t *value, attr_value_t *index)
{
    return obj_connect_internal(dev1, name, value, index);
}

/**
 * @brief SIM_set_attr : realize the method SIM_set_attr to set the value of attribute
 *
 * @param dev : the object of device
 * @param name : name of attribute
 * @param val : the value of attribute to set
 * @param index : the index of attribute
 *
 * @return : the return value have no effect
 */
int SIM_set_attr(conf_object_t *dev, const char *name, attr_value_t *val, attr_value_t *index)
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
		return set(NULL, dev, val, index);
	}
	return 0;
}
