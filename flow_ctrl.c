/*
 * flow_ctrl.c: 
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
#include "flow_ctrl.h"
static int block_has_node_type(tree_t *, type_t);

/**
 * @brief handle_inline_case : charge the inline block has the type node
 *
 * @param t : syntax tree node of inline
 * @param type : the node of type to be finded
 *
 * @return  1 - have the type node
 *			0 - not have the type node
 */
static int handle_inline_case(tree_t *t, type_t type) {

    tree_t *it = t;
    tree_t *tmp;
    symbol_t sym = NULL; 
    int ret = 0; 
   	ref_ret_t ref_ret;

    if(type == CALL_TYPE || type == THROW_TYPE) {
        tmp = it->call_inline.expr;
        sym = get_ref_sym(tmp->expr_brack.expr, &ref_ret, NULL);
        if(!sym) {
            my_DBG("err,no inline function\n");
        }    
        method_attr_t *m = sym->attr;
        tmp = m->common.node;
        tmp = tmp->method.block;
        ret = block_has_node_type(tmp, type);
        if(ret) {
            return ret; 
        }    
    }    
	return ret;
}

/**
 * @brief handle_after_call_case : charge the after_call block have call/throw node
 *
 * @param t : syntax tree node of after_call
 * @param type : type of call/throw
 *
 * @return : 1 - have call/throw node
 *			0 - not have call/throw node
 */
static int handle_after_call_case(tree_t *t, type_t type) {
	if(type == CALL_TYPE || type == THROW_TYPE) {
		return 1;
	}
	return 0;
}

/**
 * @brief handle_try_catch : charge try catch block has one type node
 *
 * @param t : syntax tree node of try catch
 * @param type : the type of node to be finded
 *
 * @return : 1 - have the node of type
 *			0 - not have the node of type
 */
static int handle_try_catch(tree_t *t, type_t type) {
    int ret; 

    if(type == CALL_TYPE || type == THROW_TYPE) {
        ret = block_has_node_type(t->try_catch.catch_block, type);
        if(ret) {
            return ret;
        }
    } else if(type == RETURN_TYPE) {
        ret = block_has_node_type(t->try_catch.try_block, type);
        if(ret) {
			return ret;
        }
        ret = block_has_node_type(t->try_catch.catch_block, type);
        if(ret) {
			return ret;
        }
    }
	return 0;
}

/**
 * @brief block_has_node_type : charge the block has one type node or not
 *
 * @param t : syntax tree node of block
 * @param node_type : type of node
 *
 * @return : 1 - has the node
 *			0 - not have the node
 */
static int block_has_node_type(tree_t *t, type_t node_type) {
    tree_t *node;
    tree_t *it;
    tree_t *tmp;
    int ret = 0;

    node = t;
    if(!node) {
        return 0;
    }
    if(node->common.type == node_type) {
        return 1;
    }
    if(node->common.type == BLOCK_TYPE) {
            it = node->block.statement;
            while(it) {
                type_t type = it->common.type;
                if(type == node_type) {
                    ret = 1;
                    break;
                } else if (type == BLOCK_TYPE) {
                        ret |= block_has_node_type(it, node_type);
                        if(ret) {
                            break;
                        }
                } else if(type == IF_ELSE_TYPE) {
                        ret = block_has_node_type(it->if_else.if_block, node_type);
                        if(ret) {
                            break;
                        }
                        ret = block_has_node_type(it->if_else.else_if, node_type);
                        if(ret) {
                            break;
                        }
                        ret = block_has_node_type(it->if_else.else_block, node_type);
                        if(ret) {
                            break;
                        }
                } else if (type == DO_WHILE_TYPE) {
                    ret = block_has_node_type(it->do_while.block, node_type);
                    if(ret) {
                        break;
                    }
                } else if (type == INLINE_TYPE) {
                    ret = handle_inline_case(it, node_type);
                    if(ret) {
                        break;
                    }
                } else if(type == FOREACH_TYPE) {
                    tmp = it->foreach.block;
                    ret = block_has_node_type(tmp, node_type);
                    if(ret) {
                        break;
                    }
                } else if(type == FOR_TYPE) {
                    ret = block_has_node_type(it->for_tree.block, node_type);
                    if(ret) {
                        break;
                    }
                } else if(type == TRY_CATCH_TYPE) {
                    ret = handle_try_catch(it, node_type);
                    if(ret) {
                        break;
                    }
                } else if(type == AFTER_CALL_TYPE) {
					ret = handle_after_call_case(it, node_type);
					if(ret) {
						break;
					}
				}
                it = it->common.sibling;
            }
    }
    return ret;
}

/**
 * @brief method_has_exec : method have some call/throw expression
 *
 * @param m : syntax tree node of method
 *
 * @return : 1 - have call/throw
 *			0 - node have call/throw expression
 */
int method_has_exec(tree_t *m) {
	tree_t *block = m->method.block;
	return block_has_node_type(block, CALL_TYPE) || block_has_node_type(block, THROW_TYPE);
}

/**
 * @brief method_has_return : charge method has return value
 *
 * @param m : syntax tree node of method
 *
 * @return : 1 - have return value
 *			0 - not have return value
 */
int method_has_return(tree_t *m) {
	tree_t *block = m->method.block;
	return block_has_node_type(block, RETURN_TYPE);
}
