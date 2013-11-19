/*
 * gen_object.c: 
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
/**
 * @file gen_object.c
 * @brief object code framework
 * @author alloc, alloc.young@gmail.com
 * @version 1.0
 * @date 2013-05-24
 */

#include "chk_object.h"
extern int method_to_generate;

static void chk_object_method(object_t *obj){
	struct method_name *m;
	struct list_head *p;

	list_for_each(p,&obj->methods){
		m = list_entry(p,struct method_name, entry);
		chk_dml_method(obj, m);
	}
	add_object_generated_method(obj);
} 

static void chk_obj_generic_code(object_t *obj){
	struct list_head *p;
	object_t *t;

	chk_object_method(obj);

	list_for_each(p, &obj->childs){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}
}

static void chk_device_code(device_t *obj){
	struct list_head *p;
	object_t *t;
	device_t *dev = (device_t *)obj;

	chk_obj_generic_code(&dev->obj);

	/*gen connect object */
	list_for_each(p,&dev->connects){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	} 
	/*implement ect*/
	list_for_each(p,&dev->implements){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	} 
	/*connect,port ect*/
	list_for_each(p,&dev->attributes){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	} 

	/* gen port object */
	list_for_each(p,&dev->ports){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	}

	/* gen event object */
	list_for_each(p,&dev->events){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	}
}

void chk_dml_code(device_t *dev) {
	int round = 0;

	while(method_to_generate){
		round++;
		BE_DBG(GENERAL, "code generation round %d, method to generate %d\n",round, method_to_generate);
		chk_device_code(dev);
	}
}


