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

#include "gen_object.h"
#include "gen_debug.h"
extern int method_to_generate;
static void gen_obj_generic_code(object_t *obj);


/**
 * @brief gen_object_method : entry to generate method code of object
 *
 * @param obj : the object to be generated
 */
static void gen_object_method(object_t *obj){
	struct method_name *m;
	struct list_head *p;

	list_for_each(p,&obj->methods){
		m = list_entry(p,struct method_name, entry);
		gen_dml_method(obj, m);
	}	
	add_object_generated_method(obj);	
} 

static void gen_none_objs(object_t *obj) {
}

static void gen_event_objs(object_t *obj) {
	struct list_head *p;
	object_t *t;

	list_for_each(p, &obj->events) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
}

static void gen_bank_objs(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
	struct list_head *p;
	object_t *t;

	gen_event_objs(obj);
	list_for_each(p, &bank->attributes) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
	list_for_each(p, &bank->implements) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
	list_for_each(p, &bank->groups) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
}

static void gen_register_objs(object_t *obj) {
	gen_event_objs(obj);
}

static void gen_attribute_objs(object_t *obj) {
	gen_event_objs(obj);
}

static void gen_connect_objs(object_t *obj) {
	gen_event_objs(obj);
}

static void gen_port_objs(object_t *obj) {
	struct list_head *p;
	object_t *t;
	dml_port_t *port = (dml_port_t *)obj;

	gen_event_objs(obj);
	list_for_each(p, &port->connects) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
	list_for_each(p, &port->attributes) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
	list_for_each(p, &port->implements) {
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
}

static void gen_group_objs(object_t *obj) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &obj->events) {
   		tmp = list_entry(p, object_t, entry);
    	gen_obj_generic_code(tmp);
	}
}

static void (*gen_obj_others[])(object_t *obj) = {
	[Obj_Type_None] = gen_none_objs,
	[Obj_Type_Device] = gen_none_objs,
	[Obj_Type_Bank] = gen_bank_objs,
	[Obj_Type_Register] = gen_register_objs,
	[Obj_Type_Field]   = gen_none_objs,
	[Obj_Type_Attribute] = gen_attribute_objs,
	[Obj_Type_Connect] = gen_connect_objs,
	[Obj_Type_Port]    = gen_port_objs,
	[Obj_Type_Implement ... Obj_Type_Event] = gen_none_objs,
	[Obj_Type_Group] = gen_group_objs
};
 

/**
 * @brief gen_obj_generic_code : entry to generate object code
 *
 * @param obj : the object to be generated
 */
static void gen_obj_generic_code(object_t *obj){
	struct list_head *p;
	object_t *t;

	gen_object_method(obj);

	list_for_each(p, &obj->childs){
		t = list_entry(p, object_t, entry);
		gen_obj_generic_code(t);
	}
	gen_obj_others[obj->encoding](obj);

}

/**
 * @brief gen_device_code : generate the code of objects in device
 *
 * @param obj : the object of device
 */
static void gen_device_code(device_t *obj){
	struct list_head *p;
	object_t *t;
	device_t *dev = (device_t *)obj;
	int i;

	gen_obj_generic_code(&dev->obj);
	/*gen connect object */
	list_for_each(p,&dev->connects){
		t = list_entry(p,object_t,entry);
		gen_obj_generic_code(t);
	} 
	/*implement ect*/
	list_for_each(p,&dev->implements){
		t = list_entry(p,object_t,entry);
		gen_obj_generic_code(t);
	} 
	/*event ect*/
	list_for_each(p,&dev->obj.events){
		t = list_entry(p,object_t,entry);
		gen_obj_generic_code(t);
	}
	/*connect,port ect*/
	list_for_each(p,&dev->ports){
		t = list_entry(p,object_t,entry);
		gen_obj_generic_code(t);
		dml_port_t *port = (dml_port_t *)t;
		for(i = 0; i < port->num; i++) {
			gen_obj_generic_code(port->impls[i]);
		}
	}
	/* generate attribute code */
	list_for_each(p, &dev->attributes) {
		t = list_entry(p, object_t , entry);
		gen_obj_generic_code(t);
	}

}

extern FILE *out;

/**
 * @brief gen_dml_code : generate other code of dml
 *
 * @param dev : the object of device
 * @param f : c file to be generated
 */
void gen_dml_code(device_t *dev, FILE* f) {
	int round = 0;

	out = f;
	gen_code_once_noplatform(dev, f); 

	
	while(method_to_generate){
		round++;
		BE_DBG(GENERAL, "code generation round %d, method to generate %d\n",round, method_to_generate);
		gen_device_code(dev);
	}
}


