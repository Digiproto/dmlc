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
#include "symbol.h"
#include "symbol-common.h"
#include "tree.h"

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

/**
 * @brief : define struct to store record element
 */
typedef struct element
{
    struct cdecl* decl;  // declaration about element
    struct element *next; // the next element
} element_t;

struct type_struct {
	TYPE_COMMON;	// type for struct
	int is_defined; // sign of defined or not for second check
	const char* id; // name of struct
	element_t* elem; // elements of struct
	symtab_t table; // table of struct
};

struct type_layout {
	TYPE_COMMON;
	int is_defined; // sign of defined or not  for second check
	const char* id; // name of layout
	const char* bitorder; // the bit order layout
	element_t* elem; // the elements of layout
	symtab_t table; // table of layout
};

/**
 * @brief : information about bit element in bitfields struct
 */
typedef struct bit_elem {
	struct cdecl* decl; // declaration about bit element
	struct bit_elem* next; // the next element
	tree_t *start; // the element start space
	tree_t *end; // element end space
	const char *name;
	int size; // size of element
} bit_element_t;;

struct type_bitfields {
	TYPE_COMMON;
	int is_defined; // sign of defined or not for second chek
	int bit_size; // size of bitfields
	const char* id; // name of bitfields
	bit_element_t* elem; // elements of bitfields
	symtab_t table; // table of bitfields
};

/**
 * @brief : defined for storing fucntion paramater
 */
typedef struct vector
{
    void **data;
    int len;
} vector_t;

/**
 * @brief : defined for storing function information
 */
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

typedef struct type_object {
	TYPE_COMMON;
	void *obj;
} type_object_t;

/**
 * @brief : definition for variable declaration
 */
typedef struct cdecl {
	/* type of variable */
	union {
		struct type_common common;
		struct type_struct struc;
		struct type_layout layout;
		struct type_bitfields bitfields;
		struct function_type function;
		struct type_object object;
	};
	/* the original type name of typedef */
	const char* typedef_name;
	tree_t *node;
	/* variable identifier */
	const char* var_name;
} cdecl_t;

typedef struct type_deriv_list {
    int ctor;
    union {
        int len;
        int qual;
        signature_t* sig;
    };
    struct type_deriv_list *next;
} type_deriv_list_t;

/**
 * @brief : the definition of parameter about function
 */
typedef struct param
{
	int is_ellipsis; // function parameter is ellipsis or not
    const char *id; // name of parameter
    cdecl_t* ty; // type of parameter
} param_t;

void parse_extern_cdecl_or_ident(tree_t* node, symtab_t table);
void check_undef_template(symtab_t table);
void add_template_to_table(symtab_t table, const char* template_name, int second_check);
void print_templates(symtab_t table);
void parse_local_decl(tree_t* node, symtab_t table);
void parse_data_cdecl(tree_t* node, symtab_t table);
void parse_typedef_cdecl(tree_t* node, symtab_t table);
void parse_top_struct_cdecl(tree_t* node, symtab_t table, void* attr);
cdecl_t *parse_cdecl(tree_t *node, symtab_t table);

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
