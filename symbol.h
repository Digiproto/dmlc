
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
#include "parameter_type.h"

/**
 * @brief : common part of symbol
 */
struct symbol_common {
   tree_t* node;
   int table_num;
};

typedef struct dml_attr
{
	struct symbol_common common;
	const char* version;
} dml_attr_t;

typedef struct bitorder_attr
{
	struct symbol_common common;
	const char* name;
	const char* endian;
} bitorder_attr_t;

typedef struct import_attr
{
	struct symbol_common common;
	symtab_t table;
	const char *file;
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

typedef struct constant_attr
{
	const char *name;
	struct symbol_common common;
	expr_t *value;
} constant_attr_t;

typedef struct struct_attr
{
    struct symbol_common common;
    const char* name;
	cdecl_t* decl;
    symtab_t table;
} struct_attr_t;

typedef struct paramspec {
	tree_t* expr_node;
	param_value_t* value;
}paramspec_t;

typedef struct parameter_attr
{
	int is_original;
	struct symbol_common common;
	const char *name;
	paramspec_t* param_spec;
} parameter_attr_t;

typedef struct params {
	int is_notype;
	cdecl_t* decl;
	const char* var_name;
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
	int is_parsed;
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

typedef struct select_attr {
	struct symbol_common common;
	const char* ident;
	int type;
	expr_t* in_expr;
}select_attr_t;

typedef struct foreach_attr {
	struct symbol_common common;
	const char* ident;
	expr_t* expr;
	symtab_t table;
}foreach_attr_t;

typedef struct label_attr {
	struct symbol_common common;
	const char* name;
	symtab_t table;
}label_attr_t;

/**
 * @brief : object common part
 */
struct object_common {
	tree_t* node;
	symtab_t table;
	int table_num;
	const char *name;
	const char* desc;
	char** templates;
	int templates_num;
	int obj_type;
	int is_defined;
};

struct bank_attr;

typedef struct register_attr
{
	struct object_common common;
	int is_array;
	int size;
	int offset;
	arraydef_attr_t* arraydef;
	struct bank_attr *bank;
} register_attr_t;

typedef struct register_list_node
{
	register_attr_t *reg;
	struct register_list_node *next;
} register_list_node_t;

typedef struct bank_attr
{
	struct object_common common;
	register_list_node_t *registers;
	int register_size;
	int size;
} bank_attr_t;

typedef struct try_catch_attr {
	struct symbol_common common;
	symtab_t try_table;
	symtab_t catch_table;
}try_catch_attr_t;

typedef struct bitfield_attr {
    struct symbol_common common;
    const char* name;
	const char* size_str;
	int size;
    symtab_t table;
}bitfield_attr_t;

typedef struct bitrange_attr{
	struct symbol_common common;
	int is_fix;
	expr_t* expr;
	expr_t* expr_end;
}bitrange_attr_t;

typedef struct field_attr
{
	struct object_common common;
	bitrange_attr_t* bitrange;
	int is_array;
} field_attr_t;

typedef struct connect_attr
{
	struct object_common common;
	int is_array;
	arraydef_attr_t* arraydef;
} connect_attr_t;

typedef struct interface_attr
{
	struct object_common common;
} interface_attr_t;

typedef struct attribute_attr
{
	struct object_common common;
	int is_array;
	arraydef_attr_t* arraydef;
	void* attr_obj;
} attribute_attr_t;

typedef struct event_attr
{
	struct object_common common;
} event_attr_t;

typedef struct group_attr
{
	struct object_common common;
	int is_array;
	arraydef_attr_t* arraydef;
} group_attr_t;

typedef struct port_attr
{
	struct object_common common;
	int is_array;
	arraydef_attr_t* arraydef;
} port_attr_t;

typedef struct implement_attr
{
	struct object_common common;
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
	symtab_t root_table;
} device_attr_t;

typedef union object_attr {
	struct object_common common;
	struct bank_attr bank;
	struct register_attr reg;
	struct field_attr field;
	struct connect_attr connect;
	struct interface_attr interface;
	struct attribute_attr attribute;
	struct event_attr event;
	struct group_attr group;
	struct port_attr port;
	struct implement_attr implement;
}object_attr_t;

void params_insert_table(symtab_t table, method_params_t* method_params);
paramspec_t* get_paramspec(tree_t* node, symtab_t table);
void undef_var_insert(symtab_t table, tree_t* name);
void print_all_symbol(symtab_t symtab);

//#define DEBUG_SYMBOLS

#ifdef DEBUG_SYMBOLS
#define DEBUG_TOP_LEVEL debug_proc
#define DEBUG_OTHER_LEVEL debug_blue
#define DEBUG_SYMBOL printf
#else
#define DEBUG_TOP_LEVEL
#define DEBUG_OTHER_LEVEL
#define DEBUG_SYMBOL
#endif

#endif
