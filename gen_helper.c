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

/**
 * @brief is_block : check the syntax tree node is block or not
 *
 * @param node : syntax tree node
 *
 * @return : 1 - tree node is block
 *			0 - not block tree node
 */
int is_block(tree_t *node) {
    if(node && node->common.type == BLOCK_TYPE) {
        return 1;
    }
    return 0;
}

symtab_t get_block_table(tree_t *node) {
	return	node->block.table;	
}

/**
 * @brief node_is_expression : check the syntax tree node is expression or not
 *
 * @param node : syntax tree node to check
 *
 * @return : 1 - syntax tree node is expression
 *			0 - syntax tree node is not expression
 */
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

/**
 * @brief is_if_statement : check tree node is if statement or not
 *
 * @param node : syntax tree node of checking
 *
 * @return : 1 - if else tree node
 *			0 - not if else tree node
 */
static int is_if_statement(tree_t *node) {
    if(node && node->common.type == IF_ELSE_TYPE) {
        return 1;
    }
    return 0;
}


int need_pos(tree_t *node) {
    return is_if_statement(node) || node_is_expression(node) || is_block(node);
}

/**
 * @brief need_semicolon : check the node need semicolon after generated
 *
 * @param node : syntax tree node
 *
 * @return : 1 - need semicolon
 *			0 - not need semicolon
 */
int need_semicolon(tree_t *node) {
    return node_is_expression(node);
}

/**
 * @brief handle_one_statement : generate the block with only one statement
 *
 * @param node : syntax tree node of statement
 */
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
