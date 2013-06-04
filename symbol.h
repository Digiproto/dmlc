
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
#include "tree.h"
#include "expression.h"
#include "decl.h"

struct symbol_common {
   tree_t* node;
   int table_num;
};

typedef struct variable {
    int type;
    char *name;
} variable_t;

typedef struct dml_attr
{
	const char* version;
	struct symbol_common common;
} dml_attr_t;


typedef struct bitorder_attr
{
	const char* endian;
	struct symbol_common common;
} bitorder_attr_t;

typedef struct import_attr
{
	const char *filename;
} import_attr_t;

typedef struct template_attr
{
	struct symbol_common common;
	const char *name;
	const char *desc;
	symtab_t table;
} template_attr_t;
typedef struct loggroup_attr
{
	struct symbol_common common;
} loggroup_attr_t;

typedef struct typedef_attr
{
} typedef_attr_t;
typedef struct constant_attr
{
	const char *name;
	struct symbol_common common;
	expression_t *value;
} constant_attr_t;
typedef struct struct_attr
{
    struct symbol_common common;
    const char* name;
    symtab_t table;
} struct_attr_t;
typedef struct object_stmt_node
{
} stmt_attr_t;

typedef struct paramspec {
	int is_default;
	int is_auto;
	int type;
	const char* str;
	expression_t *expr;
}paramspec_t;

typedef struct parameter_attr
{
	const char *name;
	struct paramspec_t* spec;
} parameter_attr_t;

typedef struct value {
}value_t;

typedef struct params {
	int is_notype;
	decl_t* decl;
	char* var_name;
#if 0
	decl_type_t* decl_type;
	char* decl_str;
	expression_t *value;
#endif
}params_t;

typedef struct method_params {
	int in_argc;
	params_t** in_list;
	int ret_argc;
	params_t** ret_list;
}method_params_t;

typedef struct method_attr
{
	struct symbol_common common;
	const char *name;
	int is_extern;
	int is_default;
	int is_static;
	int is_inline;
	int is_template;
	method_params_t* method_params;
	symtab_t table;
} method_attr_t;

struct bank_attr;

typedef struct arraydef_attr {
	int fix_array;
	const char* ident;
	int low;
	int high;
}arraydef_attr_t;

typedef struct foreach_attr {
	struct symbol_common common;
	char* ident;
	expression_t* expression;
	symtab_t table;
}foreach_attr_t;

typedef struct register_attr
{
	struct symbol_common common;
	const char *name;
	int is_array;
	int size;
	int offset;
	char** templates;
	int templates_num;
	char* desc;
	arraydef_attr_t* arraydef;
	struct bank_attr *bank;
	symtab_t table;
} register_attr_t;

typedef struct register_list_node
{
	register_attr_t *reg;
	struct register_list_node *next;
} register_list_node_t;

typedef struct bank_attr
{
	struct symbol_common common;
	const char** templates;
	const char *name;
	const char* desc;
	int template_num;
	int register_size;
	int size;
	symtab_t table;
	register_list_node_t *registers;
} bank_attr_t;

typedef struct try_catch_attr {
	struct symbol_common common;
	symtab_t try_table;
	symtab_t catch_table;
}try_catch_attr_t;

typedef struct bitfield_attr {
    struct symbol_common common;
    const char* name;
    symtab_t table;
}bitfield_attr_t;

typedef struct register_array_attr
{
} register_array_attr_t;

typedef struct bitrange_attr{
	struct symbol_common common;
	int is_fix;
	expression_t expr;
	expression_t expr_end;
}bitrange_attr_t;

typedef struct field_attr
{
	struct symbol_common common;
	const char *name;
	const char** templates;
	const char* desc;
	int template_num;
	int is_range;
	bitrange_attr_t* bitrange;
	symtab_t table;
} field_attr_t;

typedef struct data_attr
{
	const char *name;
} data_attr_t;
typedef struct connect_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	int is_array;
	arraydef_attr_t* arraydef;
	symtab_t table;
} connect_attr_t;

typedef struct connect_array_attr
{
} connect_array_attr_t;

typedef struct interface_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	symtab_t table;
} interface_attr_t;

typedef struct attribute_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int is_array;
	int template_num;
	arraydef_attr_t* arraydef;
	symtab_t table;
} attribute_attr_t;
typedef struct attribute_array_attr
{
} attribute_array_attr_t;

typedef struct event_attr_t
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	symtab_t table;
} event_attr_t;

typedef struct group_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	int is_array;
	arraydef_attr_t* arraydef;
	symtab_t table;
} group_attr_t;

typedef struct group_array_attr
{
} group_array_attr_t;

typedef struct port_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	symtab_t table;
} port_attr_t;
typedef struct implement_attr
{
	struct symbol_common common;
	const char* name;
	const char** templates;
	const char* desc;
	int template_num;
	symtab_t table;
} implement_attr_t;

typedef struct bank_list_node
{
	bank_attr_t *bank;
	struct bank_list_node *next;
} bank_list_node_t;

typedef struct device_attr
{
	struct symbol_common common;
	const char *name;
	int endian;
	bank_list_node_t *banks;
	tree_t* node;
} device_attr_t;

typedef struct layout_attr {
    struct symbol_common common;
    char* str;
    symtab_t table;
} layout_attr_t;

typedef struct ident_attr
{
	const char *name;
} ident_attr_t;

typedef struct const_str_attr
{
	const char *name;
} const_str_attr_t;

#endif
