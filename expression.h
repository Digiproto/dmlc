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
    int is_const;
	int final_type;
    int is_undefined;
    const_expr_t* const_expr;
    tree_t* node;
	struct expression_t* next;
} expression_t;

expression_t* parse_expression(tree_t* node, symtab_t table);
void parse_log_args(tree_t* node, symtab_t table);
expression_t* get_ident_value(tree_t* node, symtab_t table,  expression_t* expr);

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
