/*
 * gen_common.h: 
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

#ifndef __GEN_COMMON_H__ 
#define __GEN_COMMON_H__  

#include <string.h>
#include "object.h"
#include "ref.h"
#include "gen_utility.h"

typedef struct flow_control {
        symbol_t exec;
} flow_ctrl_t;

typedef struct context_table {
        symtab_t current;
        symtab_t saved;
        symtab_t method_parent;
        object_t *obj;
} context_t;

void gen_dml_method(object_t *obj, struct method_name *m);
const char *get_cdecl_name(tree_t *node);
const char *get_type_info(tree_t *node);
void gen_dml_method_header(object_t *obj, tree_t *m);
void cdecl_or_ident_list_params_alias(tree_t *params, int ret);
void do_block_logic(tree_t *block);
void pre_gen_method(object_t *obj, tree_t *method, context_t *context);
void post_gen_method(object_t *obj, tree_t *method, context_t *context);
void do_block_logic(tree_t *block);
void translate_block(tree_t *node);
void translate_foreach(tree_t *node);
void translate_call(tree_t *node);
void translate_cdecl(tree_t *node);
void translate_ctypedecl(tree_t *node);
void translate_ctypedecl_ptr(tree_t *node);
void translate_stars(tree_t *node);
void translate_cdecl2(tree_t *node);
void translate_ident(tree_t *node);
void translate_if_else(tree_t *node);
void translate_dml_keyword(tree_t *node);
void translate_integer_literal(tree_t *node);
void translate_log(tree_t *node);
void translate_expr_list(tree_t *expr,const char *prefix);
void translate_inline(tree_t *t);
void translate_c_keyword(tree_t *t);
void translate_local(tree_t *t);
void translate_typeof(tree_t *t);
void translate_quote(tree_t *t);
void translate_ref_expr(tree_t *t);
void translate_after_call(tree_t *t);
void translate_cdecl3_array(tree_t *t);
void translate_while(tree_t *t);
void translate_float(tree_t *t);
void translate_for(tree_t *t);
void translate_break(tree_t *t);
void translate_continue(tree_t *t);
void translate_switch(tree_t *t);
void translate_case(tree_t *t);
void translate_default(tree_t *t);
int block_empty(tree_t *t);
symbol_t  get_call_expr_info(tree_t *node, symtab_t table);
symbol_t  get_expression_sym(tree_t *node);
int check_method_param(symbol_t sym, tree_t* call_expr, tree_t* ret_expr, int in_line);
#endif /* __GEN_COMMON_H__ */
