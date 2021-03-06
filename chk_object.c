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
#include <assert.h>
#include "chk_object.h"
extern int method_to_generate;

/**
 * @brief chk_object_method : check the method of one object
 *
 * @param obj : the object of dml
 */
static void chk_object_method(object_t *obj){
	struct method_name *m;
	struct list_head *p;

	list_for_each(p, &obj->methods){
		m = list_entry(p,struct method_name, entry);
		chk_dml_method(obj, m);
	}
	add_object_generated_method(obj);
} 

static void check_none_object(object_t* obj) {
	assert(obj != NULL);
	/* we do not need to do anything */
	return;
}

static void chk_obj_generic_code(object_t *obj);

/**
 * @brief check_common_event : check the method of event object
 *
 * @param obj : the object of event
 */
static void check_common_event(object_t* obj) {
	assert(obj != NULL);
	struct list_head *p;
	object_t *tmp;
	list_for_each(p, &obj->events){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	return;
}

/**
 * @brief check_bank_sepcial_obj : check the method of special objects of bank
 *
 * @param obj : the object of bank
 */
static void check_bank_sepcial_obj(object_t* obj) {
	assert(obj != NULL);
	bank_t*  bank = (bank_t*)obj;
	struct list_head *p;
	object_t *t;

	list_for_each(p, &obj->events){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}
	list_for_each(p, &bank->attributes){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}
	list_for_each(p, &bank->implements){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}
	list_for_each(p, &bank->groups){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}

	return;
}

/**
 * @brief check_register_special_object : check the special object of register
 *
 * @param obj : the object of register
 */
static void check_register_special_object(object_t* obj) {
	assert(obj != NULL);
	check_common_event(obj);
	return;
}

/**
 * @brief check_attribute_special_object : check the special object of attribute
 *
 * @param obj : the object of attribute
 */
static void check_attribute_special_object(object_t* obj) {
	assert(obj != NULL);
	check_common_event(obj);
	return;
}

/**
 * @brief check_connect_special_object : check the special object of connect
 *
 * @param obj : the object of connect
 */
static void check_connect_special_object(object_t* obj) {
	assert(obj != NULL);
	check_common_event(obj);
	return;
}

/**
 * @brief check_port_special_object : check the special object of port
 *
 * @param obj : the object of port
 */
static void check_port_special_object(object_t* obj) {
	assert(obj != NULL);
	struct list_head *p;
	object_t *tmp;
	dml_port_t* port = (dml_port_t*)obj;
	list_for_each(p, &obj->events){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	list_for_each(p, &port->connects){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	list_for_each(p, &port->attributes){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	list_for_each(p, &port->implements){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	return;
}

/**
 * @brief check_group_special_obj : check the special object of group
 *
 * @param obj : the object of group
 */
static void check_group_special_obj(object_t* obj) {
	assert(obj != NULL);
	struct list_head* p;
	object_t* tmp;
	group_t* gp = (group_t*)obj;
	list_for_each(p, &obj->events){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	list_for_each(p, &gp->registers){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
	list_for_each(p, &gp->groups){
		tmp = list_entry(p, object_t, entry);
		chk_obj_generic_code(tmp);
	}
}

static void (*check_obj_special[])(object_t* obj) = {
	[Obj_Type_None] = check_none_object,
	[Obj_Type_Device] = check_none_object,
	[Obj_Type_Bank] = check_bank_sepcial_obj,
	[Obj_Type_Register] = check_register_special_object,
	[Obj_Type_Field]  = check_none_object,
	[Obj_Type_Attribute] = check_attribute_special_object,
	[Obj_Type_Connect]  = check_connect_special_object,
	[Obj_Type_Port]    = check_port_special_object,
	[Obj_Type_Implement] = check_none_object,
	[Obj_Type_Interface] = check_none_object,
	[Obj_Type_Data]     = check_none_object,
	[Obj_Type_Event]    = check_none_object,
	[Obj_Type_Group]    = check_group_special_obj
};

/**
 * @brief check_obj_special_code : check some special object mehtod about one object
 *
 * @param obj : object of dml
 */
static void check_obj_special_code(object_t* obj) {
		assert(obj != NULL);
		check_obj_special[obj->encoding](obj);
		return;
}

/**
 * @brief chk_obj_generic_code : check the method of object and
 * start to check the methods of its child
 *
 * @param obj : the object of dml
 */
static void chk_obj_generic_code(object_t *obj){
	struct list_head *p;
	object_t *t;

	chk_object_method(obj);

	list_for_each(p, &obj->childs){
		t = list_entry(p, object_t, entry);
		chk_obj_generic_code(t);
	}
	check_obj_special_code(obj);
}

/**
 * @brief chk_device_code : check the method of object based on the method list of object
 *
 * @param obj : the object of device
 */
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
	list_for_each(p,&dev->obj.events){
		t = list_entry(p,object_t,entry);
		chk_obj_generic_code(t);
	}
}

/**
 * @brief chk_dml_code : entry to check the method of device and other objects
 *
 * @param dev : the object of device
 */
void chk_dml_code(device_t *dev) {
	int round = 0;

	while(method_to_generate){
		round++;
		BE_DBG(GENERAL, "code generation round %d, method to generate %d\n",round, method_to_generate);
		chk_device_code(dev);
	}
}


