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

#define default_attr_type "container"

typedef struct object {
	const char *name;
	const char *qname;
	const char *dotname;
	const char *obj_type;
	tree_t *node;
	struct object *parent;
	const char *desc;
	struct list_head childs;
	struct list_head entry;
	struct list_head parameters;
	struct list_head methods;
	struct list_head templates;	
	struct list_head method_generated;
	symtab_t symtab;
	const char *attr_type;
}object_t;

typedef struct device {
    object_t obj;
    struct list_head constants;
    struct list_head attributes;
    struct list_head connects;
    struct list_head banks;
    struct list_head implements;
    struct list_head events;
    struct list_head data;
}device_t;

typedef struct bank_def {
    object_t obj;
    int register_size;
	int size;
}bank_t;

typedef struct register_def {
    object_t obj;
    int size;
    int offset;
	int is_array;
	int array_size;
} dml_register_t;

typedef struct field {
    object_t obj;
    int low;
    int high;
    int len;
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
	struct list_head entry;
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
 
device_t *create_device_tree(tree_t *root);
#endif
