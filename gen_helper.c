/*
 * gen_helper.c: 
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
#include "gen_helper.h"

int is_block(tree_t *node) {
    if(node && node->common.type == BLOCK_TYPE) {
        return 1;
    }
    return 0;
}

static int node_is_expression(tree_t *node) {
    int ret = 0; 

    switch(node->common.type) {
        case EXPR_ASSIGN_TYPE:
        case BINARY_TYPE:
        case CAST_TYPE:
        case SIZEOF_TYPE:
        case UNARY_TYPE:
        case EXPR_BRACK_TYPE:
        case INTEGER_TYPE:
        case FLOAT_TYPE:
        case CONST_STRING_TYPE:
        case UNDEFINED_TYPE:
        case QUOTE_TYPE:
        case COMPONENT_TYPE:
        case SIZEOFTYPE_TYPE:
        case NEW_TYPE:
        case ARRAY_TYPE:
        case BIT_SLIC_EXPR_TYPE:
        case RETURN_TYPE:
            ret = 1; 
            break;
        default:
            break;
    }    
    return ret; 
}

static int is_if_statement(tree_t *node) {
    if(node && node->common.type == IF_ELSE_TYPE) {
        return 1;
    }
    return 0;
}


int need_pos(tree_t *node) {
    return is_if_statement(node) || node_is_expression(node) || is_block(node);
}

int need_semicolon(tree_t *node) {
    return node_is_expression(node);
}

void handle_one_statement(tree_t *node) {
    gen_src_loc(&node->common.location);
    if(need_pos(node)) {
        POS;
	}
    translate(node);
    if(need_semicolon(node)) {
        D(";");
    }
}
