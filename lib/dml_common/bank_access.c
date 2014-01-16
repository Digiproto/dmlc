/*
 * bank_access.c: 
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
#include "simics/bank_access.h"
#include "simics/core/object_class.h"
#include "simics/core/object_model.h"

/**
 * @brief find_bank_access : find the bank access method with bank name
 *
 * @param array : array of banks
 * @param name: name of finding bank
 *
 * @return : access method of bank
 */
static const bank_access_t *find_bank_access(const struct bank_access_description *array, const char *name) {
	const struct bank_access_description *tmp;

	for(tmp = array; tmp->name; tmp++) {
		if(!strcmp(tmp->name, name)) {
			return tmp->access;
		}
	}
	return NULL;
}

/**
 * @brief SIM_find_bank_access : get the bank access method from device with bank name
 *
 * @param dev: the object of device
 * @param name : name of bank
 *
 * @return : bank access method
 */
const bank_access_t *SIM_find_bank_access(conf_object_t *dev, const char *name) {
	const conf_class_t *cls;
	const class_data_t *cls_data;
	const struct bank_access_description *p;

	cls = dev->class_data;
	cls_data = cls->cls_data;
	p = cls_data->bank_access;
	return find_bank_access(p, name);
}

/**
 * @brief get_bank_access : get the index bank access method from bank array in device
 *
 * @param array : array of banks
 * @param index : the index of bank
 *
 * @return : bank access method
 */
static const bank_access_t *get_bank_access(const struct bank_access_description *array, int index) {
	const struct bank_access_description *tmp;
	int i = 0;
	if(index < 0)
		return NULL;
	for(tmp = array; tmp->name; tmp++) {
		if(i == index) {
			return tmp->access;
		}
	}
	return NULL;
}

/**
 * @brief SIM_get_bank_access : get the access method of the index bank of device
 *
 * @param dev : the object of device
 * @param index : index of bank in device
 *
 * @return : bank access method
 */
const bank_access_t *SIM_get_bank_access(conf_object_t *dev, int index) {
	const conf_class_t *cls;
	const class_data_t *cls_data;
	const struct bank_access_description *p;

	cls = dev->class_data;
	cls_data = cls->cls_data;
	p = cls_data->bank_access;
	return get_bank_access(p, index);
}
