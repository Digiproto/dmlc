/*
 * object_class.h: 
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

#ifndef __OBJECT_CLASS_H__ 
#define __OBJECT_CLASS_H__  
#include <stdlib.h>
#include <simics/core/object_resource.h>
typedef struct conf_class {
	const char *cls_name;
	const class_data_t *cls_data;
} conf_class_t;

const conf_class_t *SIM_class_find(const char *cls_name);
void SIM_register_conf_class(const char *name, const conf_class_t *cls);
#endif /* __OBJECT_CLASS_H__ */
