/*
 * object_model.h: 
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

#ifndef __OBJECT_MODEL_H__ 
#define __OBJECT_MODEL_H__
#include <simics/base_types.h>
struct conf_class;
struct conf_object {
	const struct conf_class *class_data;
	const char *name;
	struct conf_object *clock;
	void *object_data;
	bool configured;
};

conf_object_t *pre_conf_object(const char *obj_name, const char *cls_name);
void conf_object_register(conf_object_t *obj, const char *name);
void SIM_finalize_object(conf_object_t *obj);
int SIM_delete_object(conf_object_t *obj);
#endif /* __OBJECT_MODEL_H__ */
