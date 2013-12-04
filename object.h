/*
 * object.h: 
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
* @file object.h
* @brief object model 
* @author alloc, alloc.young@gmail.com
* @version 1.0
* @date 2013-04-12
*/

#ifndef __OBJECT_H__ 
#define __OBJECT_H__  
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "list.h"
#include "symbol-common.h"
#include "symbol.h"
#include "ast.h"
#include "parameter_type.h"

#define default_attr_type "container"

#define DEV_OBJS_LIST 7
typedef enum object_type {
	Obj_Type_None,
	Obj_Type_Device,
	Obj_Type_Bank,
	Obj_Type_Register,
	Obj_Type_Field,
	Obj_Type_Attribute,
	Obj_Type_Connect,
	Obj_Type_Port,
	Obj_Type_Implement,
	Obj_Type_Interface,
	Obj_Type_Data,
	Obj_Type_Event,
	Obj_Type_Group
} object_type_t;

typedef struct object {
	const char *name;
	const char *qname;
	const char *a_name;
	const char *dotname;
	const char *obj_type;	
	int is_array;
	int array_size;
	int depth;
	tree_t *i_node;
	tree_t *d_node;
	object_type_t encoding;
	tree_t *node;
	int is_abstract;
	struct object *parent;
	const char *desc;
	struct list_head childs;
	struct list_head entry;
	struct list_head parameters;
	struct list_head methods;
	struct list_head templates;	
	struct list_head method_generated;
	struct list_head data;
	struct list_head events;
	symtab_t symtab;
	const char *attr_type;
}object_t;

typedef struct device {
    object_t obj;
    struct list_head constants;
    struct list_head attributes;
    struct list_head connects;
    struct list_head implements;
    struct list_head events;
	struct list_head ports;
	int bank_count;
	object_t **banks;
}device_t;

typedef struct bank_def {
    object_t obj;
    int register_size;
	int size;
	int reg_count;
	object_t **regs;
	struct list_head groups;
	struct list_head implements;
	struct list_head attributes;
}bank_t;

typedef struct register_def {
    object_t obj;
    int size;
    int offset;
	int interval;
	int is_array;
	int array_size;
	int field_count;
	int is_undefined;
	object_t **fields;
} dml_register_t;

typedef struct field {
    object_t obj;
    int low;
    int high;
    int len;
	int is_dummy;
	int is_fixed;
}field_t;

typedef struct template_def {
	const char *name;
	tree_t *node;
    struct list_head entry;
    struct list_head templates;
    struct list_head parameters;
    struct list_head methods;
}template_def_t;

typedef struct template_instance {
	struct template_def *tempalte_def;
	struct list_head entry;
	struct list_head need_instance_methods;
	struct list_head template_instances;
}tempalte_instance_t;

struct generated_template_method_instance {
	struct template_def *template_def;
	struct method *method;
	struct list_head entry;
};

struct template_name {
	const char *name;
	struct template_def *def;
	tree_t *node;
	struct list_head entry;
};

struct template_list {
	const char* src_name;
	const char* name;
	struct template_list* next;
};

struct method_name {
	const char *name;
	tree_t *method;
	struct list_head entry;
};

typedef struct method {
	struct list_head parameters;
	struct list_head returns;
	struct list_head entry;
	struct tree_common *block;
	int in_template;
	int default_def;
	int external; 	 
	symtab_t *symtab;
}method_t;

typedef struct connect {
	object_t obj;
} connect_t;
 
typedef struct iface {
	object_t obj;
} interface_t;

typedef struct attribute {
	object_t obj;
	const char* alloc_type;
	const char* type;
	const char* configuration;
	const char* persistent;
	const char* internal;
	int alloc;
	int ty;
} attribute_t;

typedef struct data {
	object_t obj;
} data_t;

typedef struct implement {
	object_t obj;
} implement_t;


typedef struct event {
	object_t obj;
} event_t;

typedef struct port {
	object_t obj;
	int num;
	object_t **impls;
	struct list_head connects;
	struct list_head implements;
	struct list_head attributes;
} dml_port_t;

typedef struct group {
	object_t obj;
	struct list_head groups;
	struct list_head attributes;
	struct list_head registers;
} group_t;

device_t *create_device_tree(tree_t *root);
void add_object_method(object_t *obj, const char *name);
void add_object_generated_method(object_t *obj);
int object_method_generated(object_t *obj, struct method_name *m);
void print_device_tree(device_t *dev);
void print_object_tree(device_t *dev);
void device_realize(device_t *dev);
const char *get_obj_ref(object_t *obj);
const char *get_obj_qname(object_t *obj);
void process_object_templates(object_t* obj);
object_t *find_index_obj(object_t *obj, int index);
#endif
