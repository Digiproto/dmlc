/*
 * expression.h:
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

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "tree.h"
#include "debug_color.h"
#include "decl.h"

typedef struct const_expr {
    int is_undefined;
    int is_interger;
	int out_64bit;
    char* int_str;
    long long int_value;
    int is_float;
    float float_value;
    int is_hex;
    char* hex_str;
    int is_binary;
    char* bianry_str;
    int is_string;
    char* string;
    int is_bool;
    int bool_value;
}const_expr_t;

typedef struct expression {
    int is_const; // the expression result value is constant
	int is_constant_op; // the calculate expression is for constant
						// the grammar: constant ident = expression
	int final_type;
	/* undefined type */
    int is_undefined;
	/* user undeclared variable */
	int is_undeclare;
	const char* undecl_name;
    const_expr_t* const_expr;
    tree_t* node;
	struct expression* next;
	void* func;
} expression_t;

typedef struct value {
	int i[2];
	struct int_value {
		int out_64bit;
		union {
			long long value;
			char* int_str;
		};
	}int_v;
	double d;
	void* p;
}value_t;

typedef struct expr {
	cdecl_t* type;
	int isarray : 1;
	int isfunc : 1;
	int lvalue : 1;
	int is_const : 1;
	int is_undefined : 1;
	int is_null : 1;
	int is_obj : 1;
	int no_defined : 1;
	int op : 24;
	struct expr* kids[2];
	value_t* val;
	tree_t* node;
}expr_t;

#define is_scalar_type(type) ((type->common.categ >= BOOL_T) && (type->common.categ <= POINTER_T))
#define both_scalar_type(type1, type2) (is_scalar_type(type1) && is_scalar_type(type2))

#define is_int_type(type) ((type->common.categ >= BOOL_T) && (type->common.categ <= LONG_T))
#define both_int_type(type1, type2) (is_int_type(type1) && is_int_type(type2))

#define is_double_type(type) ((type->common.categ >= FLOAT_T) && (type->common.categ <= DOUBLE_T))
#define both_double_type(type1, type2) (is_double_type(type1) && is_double_type(type2))

#define is_arith_type(type) ((type->common.categ >= BOOL_T) && (type->common.categ <= DOUBLE_T))
#define both_arith_type(type1, type2) (is_arith_type(type1) && is_arith_type(type2))

#define is_function_prt(type) ((type->common.categ == POINTER_T) && (type->common.bty->common.categ == FUNCTION_T))

#define is_ptr_type(type) (type->common.categ == POINTER_T || type->common.categ == STRING_T)
#define is_null_ptr(expr) (expr->is_null)

#define is_common_type(categ) (categ >= BOOL_T && (categ <= TYPEDEF_T))

#define no_common_type(type) (type->common.categ >= STRUCT_T && type->common.categ <= FUNCTION_T)
#define both_no_common_type(type1, type2) (no_common_type(type1) && no_common_type(type2))

#define is_array_type(type) (type->common.categ == ARRAY_T || type->common.categ == POINTER_T)
#define both_array_type(type1, type2) (is_array_type(type1) && is_array_type(type2))

#define is_same_type(type1, type2) (type1->common.categ == type2->common.categ)
#define is_no_type(type1) (type1->common.categ == NO_TYPE)

#define is_parameter_type(type) (type->common.categ == PARAMETER_TYPE)

#define new_int_type(expr) \
	do { \
		expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
		expr->type->common.categ = INT_T;	\
		expr->type->common.size = sizeof(int) * 8; \
	} while(0)

#define new_long_type(expr) \
	do { \
		expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
		expr->type->common.categ = LONG_T;	\
		expr->type->common.size = sizeof(long) * 8; \
	} while(0)

#define new_char_type(expr) \
	do { \
		expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
		expr->type->common.categ = CHAR_T;	\
		expr->type->common.size = sizeof(char) * 8; \
	} while(0)

#define new_doule_type(expr) \
	do { \
		expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
		expr->type->common.categ = DOUBLE_T;	\
		expr->type->common.size = sizeof(double) * 8; \
	} while(0)

typedef struct reference {
	const char* name;
	int is_array;
	unsigned is_pointer;
	struct reference* next;
} reference_t;

expr_t* check_expr(tree_t* node, symtab_t table);
expr_t* check_comma_expr(tree_t* node, symtab_t table);
cdecl_t* get_typeof_type(tree_t* node, symtab_t table);

expr_t* parse_log_args(tree_t* node, symtab_t table);
int get_typedef_type(symtab_t table, char* name);
int charge_type(int type1, int type2);

//#define DEBUG_EXPRESSION

#ifdef DEBUG_EXPRESSION
#define DEBUG_TEMPLATE_SYMBOL debug_green
#define DEBUG_IDENT_VALUE debug_purple
#define DEBUG_EXPR printf
#else
#define DEBUG_TEMPLATE_SYMBOL
#define DEBUG_IDENT_VALUE
#define DEBUG_EXPR
#endif

#endif /* __EXPRESSION_H__ */
