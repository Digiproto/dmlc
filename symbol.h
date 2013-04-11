/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file symbol.h
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-12-24
*/

#ifndef	SYMBOL_H
#define	SYMBOL_H
#include "types.h"

typedef struct dml_attr{
	float version;
}dml_attr_t;


typedef struct bitorder_attr{
	char* name;
}bitorder_attr_t;

typedef struct import_attr{
	char* filename;
}import_attr_t;
typedef struct template_attr{
	char* name;
	char* desc;
}template_attr_t;
typedef struct loggroup_attr{
}loggroup_attr_t;

typedef struct typedef_attr{
}typedef_attr_t;
typedef struct constant_attr{
	char* name;
	expression_t* value;
}constant_attr_t;
typedef struct struct_attr{
}struct_attr_t;
typedef struct object_stmt_node{
}stmt_attr_t;
typedef struct parameter_attr{
	char* name;
	int is_default;
	int is_auto;
	union{
		char* str;
		expression_t* exp;
	}value;
}parameter_attr_t;

typedef struct method_attr{
	char* name;
	int is_static;
	int is_inline;
	int is_template;
	int argc;
	void* arg_list;
}method_attr_t;

struct bank_attr;

typedef struct register_attr{
	char* name;
	int is_array;
	int size;
	int offset;
	struct bank_attr *bank;
}register_attr_t;

typedef struct register_list_node {
	register_attr_t *reg;
	struct register_list_node *next;
}register_list_node_t;

typedef struct bank_attr{
	char* template_name;
	char* name;
	int register_size;
	int size;
	register_list_node_t *registers;
}bank_attr_t;

typedef struct register_array_attr{
}register_array_attr_t;

typedef struct field_attr{
	char* name;
	int is_range;
}field_attr_t;
typedef struct data_attr{
	char* name;
}data_attr_t;
typedef struct connect_attr{
}connect_attr_t;

typedef struct connect_array_attr{
}connect_array_attr_t;

typedef struct interface_attr{
}interface_attr_t;

typedef struct attribute_attr{
}attribute_attr_t;
typedef struct attribute_array_attr{
}attribute_array_attr_t;

typedef struct event_attr_t{
}event_attr_t;
typedef struct group_attr{
}group_attr_t;
typedef struct group_array_attr{
}group_array_attr_t;

typedef struct port_attr{
}port_attr_t;
typedef struct implement_attr{
}implement_attr_t;

typedef struct bank_list_node {
	bank_attr_t *bank;
	struct bank_list_node *next;
}bank_list_node_t; 

typedef struct device_attr{
	char* name;
	int endian;
	bank_list_node_t *banks;
}device_attr_t;

typedef struct ident_attr {
	char* name;
}ident_attr_t;

typedef struct const_str_attr {
	char* name;
}const_str_attr_t;

typedef struct symbol{
	char *name; 
	int type;  /* READ, WRITE, or NAME */
	union{
		void* default_type;
		device_attr_t* device;
		bitorder_attr_t* bitorder;
	}attr;
	struct symbol* next; /* the other symbol with the same hash value */
} symbol_t;

symbol_t* symbol_find(char* name, int type);
int symbol_insert(char* name, int type, void* attr);
#endif
