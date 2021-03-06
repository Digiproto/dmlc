/*
 * gen_expression.h: 
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

#ifndef __GEN_EXPRESSION_H__ 
#define __GEN_EXPRESSION_H__  
#include "gen_common.h"

void translate_binop_expr(tree_t *t);
void translate_expr_brack_direct(tree_t *t);
void translate_assign(tree_t *t);
void translate_bit_slic(tree_t *t);
void translate_bit_slice2(tree_t *t);
//void translate_bit_slic_assign(tree_t *t);
void translate_brack_expr(tree_t *t);
void translate_unary_expr(tree_t *t);
void translate_pre_expr(tree_t *t);
void translate_post_expr(tree_t *t);
void translate_cast_expr(tree_t *t);
void translate_layout(tree_t *t);
void translate_bitfields(tree_t *t);
void translate_cond_expr(tree_t *);
#endif /* __GEN_EXPRESSION_H__ */
