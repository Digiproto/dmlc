/*
 * decl.h:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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

#ifndef __DECL_H__
#define __DECL_H__
#include "types.h"

enum { CONST_QUAL = 0x1, VECT_QUAL = 0x2 };
enum { POINTER_TO = 1, ARRAY_OF, FUNCTION_RETURN };

struct cdecl;
struct type_deriv_list;
#define TYPE_COMMON \
	int categ : 16;	\
	int qual : 16;	\
	int is_extern : 4; \
	int is_typedef : 4;	\
	int is_static : 2; \
	int is_local : 2; \
	int is_auto : 4; \
	int no_decalare : 16; \
	int size;		\
	struct cdecl* bty; \
	struct type_deriv_list* drv;

struct type_common {
	TYPE_COMMON;
};

typedef struct element
{
    struct cdecl* decl;
    struct element *next;
} element_t;

struct type_struct {
	TYPE_COMMON;
	int is_defined;
	const char* id;
	element_t* elem;
	symtab_t table;
};

struct type_layout {
	TYPE_COMMON;
	int is_defined;
	const char* id;
	const char* bitorder;
	element_t* elem;
	symtab_t table;
};

typedef struct bit_elem {
    struct cdecl* decl;
    struct bit_elem* next;
	int start;
	int end;
	int size;
} bit_element_t;;

struct type_bitfields {
	TYPE_COMMON;
	int is_defined;
	int bit_size;
	const char* id;
	bit_element_t* elem;
	symtab_t table;
};

typedef struct vector
{
    void **data;
    int len;
} vector_t;

typedef struct signature
{
	int has_no_decalare : 16;
    int has_ellipse : 16;
    vector_t* params;
} signature_t;

typedef struct function_type
{
	TYPE_COMMON;
    signature_t* sig;
} function_type_t;

typedef struct cdecl {
	union {
		struct type_common common;
		struct type_struct struc;
		struct type_layout layout;
		struct type_bitfields bitfields;
		struct function_type function;
	};
	const char* typedef_name;
	const char* var_name;
} cdecl_t;

typedef struct type_deriv_list {
    int ctor;
    union {
        void* len;
        int qual;
        signature_t* sig;
    };
    struct type_deriv_list *next;
} type_deriv_list_t;

typedef struct param
{
	int is_ellipsis;
    const char *id;
    cdecl_t* ty;
} param_t;

typedef struct array_decl array_decl_t;

typedef struct {
	unsigned char_type : 1;
	unsigned int_type  : 1;
	unsigned double_type : 1;
	unsigned float_type : 1;

	unsigned short_type : 1;
	/* long,  long int, long doulbe, long long */
	unsigned long_type : 2;
	unsigned void_type : 1;


	unsigned unsigned_type : 1;
	unsigned signed_type : 1;
	unsigned bool_type : 1;
	unsigned string_type : 1;

	unsigned no_type : 1;
	unsigned vect_type : 1;
	unsigned data_type : 1;

	unsigned type_const : 3;
	unsigned addr_const : 3;
	unsigned array_type : 2;
	unsigned point_type : 4;

	unsigned struct_type : 1;
	unsigned layout_type : 1;
	unsigned bitfields_type : 1;
	unsigned typeof_type : 1;

	unsigned enum_type : 1;
	unsigned union_type : 1;
	unsigned typedef_type : 1;

	unsigned is_constant: 1;

	unsigned is_extern : 1;
	unsigned is_local : 1;
	unsigned is_auto : 1;
	unsigned is_static : 1;

	unsigned is_func : 2;
	unsigned is_func_pointer : 1;
	/* aggregate type defined: struct, layout, etc. */
	unsigned aggregate_defined : 1;

	char* struct_name;
	char* layout_name;
	char* bitfields_name;
	char* typeof_name;
	char* enum_name;
	char* union_name;
	char* typedef_name;
	char* pre_dml_name;
	char* func_point_name;
	int pre_dml_type;
	array_decl_t* array_decl;
	void* func_decl;
}decl_type_t;

struct decl_common {
	unsigned value_const : 1;
	unsigned is_unsigned : 1;
	unsigned is_signed : 1;
	unsigned addr_const : 1;

	unsigned is_data : 1;
	unsigned is_constant: 1;
	unsigned is_extern : 1;
	unsigned is_local : 1;

	unsigned is_auto : 1;
	unsigned is_static : 1;
	unsigned point_num: 22;

	char* decl_str;
	type_t type;
};

struct decl_int {
	struct decl_common common;
	/* int, int1..64, uint1..64
	 * short (int), long (int), long long (int)
	 * unsigned, signed
	 * */
	char* int_name;
};

struct decl_char {
	struct decl_common common;
};

struct decl_float {
	struct decl_common common;
	/* float, double, long double */
	char* float_name;
};

struct decl_enum {
	struct decl_common common;
};

typedef struct variable_name {
	const char* var_name;
	int var_num;
	struct variable_name* next;
}var_name_t;

typedef struct decl {
	char* decl_str;
	int is_defined;
	char* defined_name;
	decl_type_t* type;
	//expression_t* value;
	void* value;
	var_name_t* var;
}decl_t;

struct array_decl {
	decl_t* decl;
	int is_fix;
	char* ident;
	void* expr;
	void* expr_end;
	struct array_decl* next;
};

void parse_extern_cdecl_or_ident(tree_t* node, symtab_t table);
void check_undef_template(symtab_t table);
void add_template_to_table(symtab_t table, const char* template_name, int second_check);
void print_templates(symtab_t table);
void parse_local_decl(tree_t* node, symtab_t table);
void parse_data_cdecl(tree_t* node, symtab_t table);
void set_decl_type(decl_type_t* decl_type, type_t type);
type_t get_decl_type(decl_t* decl);
decl_t* parse_ctypedecl(tree_t* node, symtab_t table);
decl_t* parse_ctypedecl(tree_t* node, symtab_t table);
void parse_typedef_cdecl(tree_t* node, symtab_t table);
void parse_top_struct_cdecl(tree_t* node, symtab_t table);

int record_type(cdecl_t* type);
cdecl_t* pointer_to(cdecl_t* type);
cdecl_t* array_of(cdecl_t* type, int len);
cdecl_t* parse_ctype_decl(tree_t* node, symtab_t table);
cdecl_t* parse_typeoparg(tree_t* node, symtab_t table);

//#define DEBUG_DECLARE

#ifdef DEBUG_DECLARE
#define DEBUG_DECL printf
#else
#define DEBUG_DECL
#endif

#endif /* __DECL_H__ */
