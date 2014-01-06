/*
 * object_model.c: 
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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "simics/util/list.h"
#include "simics/core/object_model.h"
#include "simics/core/object_class.h"

static conf_object_t *clock;
#define CLOCK_NAME "event_proxy"
static LIST_HEAD(obj_list);
typedef struct object_list {
	struct list_head entry;
	conf_object_t *obj; 
} obj_list_t;

static conf_object_t *SIM_new_object(const conf_class_t *cls, const char *name) {
	conf_object_t *tmp = NULL;

	if(cls->cls_data->new_instance) {
		tmp = cls->cls_data->new_instance(name);	
		tmp->class_data = cls;
		tmp->clock = clock;
		tmp->name = (const char *)strdup(name); 
	}
	return tmp;
}

void VT_object_constructor(conf_object_t *obj, const char *name) {
}

void SIM_object_register(conf_object_t *obj, const char *name) {
	obj_list_t *tmp;
	tmp = (obj_list_t*) MM_ZALLOC(sizeof(*tmp));
	assert(tmp);

	INIT_LIST_HEAD(&tmp->entry);
	obj->name = strdup(name);
	tmp->obj = obj;
	list_add_tail(&tmp->entry, &obj_list);
}

void event_proxy_init_local(void);
conf_object_t* SIM_pre_conf_object(const char *obj_name, const char *cls_name) {
	const conf_class_t *cls = NULL;

	/* create a clock */
	if(!clock) {
		event_proxy_init_local();
		cls = SIM_class_find(CLOCK_NAME);
		if(cls) {
			clock = SIM_new_object(cls, CLOCK_NAME);
		}else{
			printf("cannot find class: %s\n", CLOCK_NAME);
		}
	}

	cls = SIM_class_find(cls_name);
	if(cls) {
		return SIM_new_object(cls, obj_name);
	} else {
		printf("cannot find class: %s\n", cls_name);
	}
	return NULL;
}

conf_object_t* SIM_get_conf_object(const char *obj_name) {
	struct list_head *p;
	obj_list_t *tmp;

	list_for_each(p, &obj_list) {
		tmp = list_entry(p, obj_list_t, entry);
		if(!strcmp(tmp->obj->name, obj_name)) {
			return tmp->obj;
		}
	}
	return NULL;
}

int SIM_delete_object(conf_object_t *obj) {
	const class_data_t *cls_data;

	cls_data = obj->class_data->cls_data;
	if(cls_data->delete_instance) {
		return cls_data->delete_instance(obj);
	}
	return 0;
}

void SIM_finalize_object(conf_object_t *obj) {
	const class_data_t *cls_data;

	cls_data = obj->class_data->cls_data;
	if(cls_data->finalize_instance) {
		cls_data->finalize_instance(obj);
	}
}
