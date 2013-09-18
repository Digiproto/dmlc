/*
 * expression.c:
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "symbol.h"
#include "tree.h"
#include "types.h"
#include "ast.h"
#include "symbol-common.h"
#include "debug_color.h"
#include "gen_common.h"
#include "info_output.h"

void free_sibling(tree_t** node) {
	assert(*node != NULL);
	if ((*node)->common.sibling) {
		free((*node)->common.sibling);
	}

	return;
}

void free_child(tree_t** node) {
	assert(*node != NULL);
	if ((*node)->common.child) {
		free((*node)->common.child);
	}

	return;
}

void assgin_sibling_child_null(tree_t** node) {
	assert(*node != NULL);
	(*node)->common.sibling = NULL;
	(*node)->common.child = NULL;

	return ;
}

static int dml_struct_type(symbol_t symbol) {
    assert(symbol != NULL);

    int type = symbol->type;
    int refer_type = 0;

    switch (type) {
        case DEVICE_TYPE:
        case BANK_TYPE:
        case REGISTER_TYPE:
        case FIELD_TYPE:
        case CONNECT_TYPE:
        case INTERFACE_TYPE:
        case ATTRIBUTE_TYPE:
        case EVENT_TYPE:
        case GROUP_TYPE:
        case PORT_TYPE:
        case IMPLEMENT_TYPE:
        case DATA_TYPE:
        case METHOD_TYPE:
        case PARAMETER_TYPE:
            refer_type = type;
            break;
        default:
            refer_type = 0;
            break;
    }
    return refer_type;
}

static int find_dml_struct(symtab_t table, const char* name) {
    assert(name != NULL);
    assert(table != NULL);

    symbol_t symbol =  symbol_find_curr_notype(table, name);
    int dml_type = 0;
    if (symbol) {
        dml_type = dml_struct_type(symbol);
        if (dml_type) {
            return dml_type;
        }
    }

    if (table->parent) {
        dml_type = find_dml_struct(table->parent, name);
    }
    else {
        symtab_t root_table = get_root_table();
        if (table != root_table) {
            dml_type = find_dml_struct(root_table, name);
        }
    }

    return dml_type;
}

static int check_refer_dml_struct(tree_t* node, symtab_t table) {
    assert(node != NULL);
    assert(table != NULL);

    char* refer_name = (char*)(node->ident.str);

    return find_dml_struct(table, refer_name);
}

int charge_type_int(int type) {
	if (type == CHAR_TYPE || (type == INTEGER_TYPE)
			|| (type == INT_TYPE) || (type == SHORT_TYPE)
			|| (type == UNSIGNED_TYPE) || (type == SIGNED_TYPE)
			|| (type == LONG_TYPE) || (type == NO_TYPE)) {
		return 1;
	}
	else {
		return 0;
	}
}

int charge_node_is_const(tree_t* node) {
	assert(node != NULL);
	int type = node->common.type;
	if ((type == INTEGER_TYPE) || (type == FLOAT_TYPE)
			|| (type == CONST_STRING_TYPE) || (type == UNDEFINED_TYPE)) {
		return 1;
	}
	else {
		return 0;
	}
}

symbol_t get_symbol_from_root_table(const char* name, type_t type) {
	assert(name != NULL);
	symtab_t root_table = get_root_table();
	symbol_t symbol = NULL;
	if (type == 0) {
		symbol = symbol_find_notype(root_table, name);
	}
	else {
		symbol = symbol_find_curr(root_table, name, type);
	}

	return symbol;
}

tree_t* copy_data_from_constant(tree_t* node) {
	assert(node != NULL);
	tree_t* new_node = NULL;
	tree_t* tmp = NULL;
	switch (node->common.type) {
		case INTEGER_TYPE:
			new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &node->common.location);
			new_node->int_cst.int_str = node->int_cst.int_str;
			new_node->int_cst.value = node->int_cst.value;
			new_node->int_cst.out_64bit = node->int_cst.out_64bit;
			new_node->common.print_node = print_interger;
			new_node->common.translate = translate_integer_literal;
			break;
		case FLOAT_TYPE:
			new_node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst), &node->common.location);
			new_node->float_cst.float_str = node->float_cst.float_str;
			new_node->float_cst.value = node->float_cst.value;
			new_node->common.print_node = print_float_literal;
			new_node->common.translate = translate_float;
			break;
		case CONST_STRING_TYPE:
			new_node = (tree_t*)create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string), &node->common.location);
			new_node->string.length = node->string.length;
			new_node->string.pointer = node->string.pointer;
			new_node->common.print_node = print_string;
			break;
		case EXPR_BRACK_TYPE:
			tmp = node->expr_brack.expr_in_brack;
			if (charge_node_is_const(tmp)) {
				new_node = copy_data_from_constant(tmp);
			}
			else {
				fprintf(stderr, "The value is no constant : %s(%d)\n",
						tmp->common.name, tmp->common.type);
				exit(-1);
			}
			break;
		default:
//			fprintf(stderr, "other constant value type: %s(%d)\n",
//					node->common.name, node->common.type);
//				/* TODO: handle the error */
//				exit(-1);
			PERROR("other constant value type type: %s(type: %s)\n", node->common.location,
					node->common.name, TYPENAME(node->common.type));
			break;
	}
	return new_node;
}

constant_attr_t* get_constant_attr(tree_t** node, symtab_t table) {
	assert(*node != NULL);
	if (((*node)->common.type != IDENT_TYPE) && ((*node)->common.type != DML_KEYWORD_TYPE)) {
//		fprintf(stderr, "not constant node %s: %d\n", (*node)->common.name, (*node)->common.type);
//		/* FIXME: handle  the error */
//		exit(-1);
		PERROR("not constant node: %s(type: %s)\n", (*node)->common.location,
				(*node)->common.name, TYPENAME((*node)->common.type));
	}
	symbol_t symbol = symbol_find(table, (*node)->ident.str, CONSTANT_TYPE);
	if ((symbol == NULL) && (table->no_check == 1)) {
		symbol = get_symbol_from_root_table((*node)->ident.str, CONSTANT_TYPE);
	}
	return (constant_attr_t*)(symbol->attr);
}

type_t change_node_to_const(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	constant_attr_t* attr = get_constant_attr(node, table);
	tree_t* new_node = copy_data_from_constant(attr->value->node);
	new_node->common.sibling = (*node)->common.sibling;
	new_node->common.child = (*node)->common.child;
	assgin_sibling_child_null(node);
	free(*node);
	*node = new_node;
	cal_expression(node, table, expr);
	DEBUG_EXPR("constant node type: %s, value type: %d, expr->type : %d\n",
			new_node->common.name, attr->value->final_type, expr->final_type);
	return attr->value->final_type;
}

expression_t* get_const_expr_value(tree_t* node, expression_t* expr) {
	assert(node != NULL);
	/* FIXME: assert is noly for debugging */
	DEBUG_EXPR("In %s, line = %d, node type: %s : %d\n",
			__func__, __LINE__, node->common.name, node->common.type);
	const_expr_t* const_expr = (const_expr_t*)gdml_zmalloc(sizeof(const_expr_t));
	expr->is_const = 1;
	expr->const_expr = const_expr;
	switch (node->common.type) {
		case INTEGER_TYPE:
			expr->final_type = INTEGER_TYPE;
			const_expr->is_interger = 1;
			const_expr->int_str = (char*)(node->int_cst.int_str);
			const_expr->out_64bit = node->int_cst.out_64bit;
			const_expr->int_value = node->int_cst.value;
			break;
		case FLOAT_TYPE:
			expr->final_type = FLOAT_TYPE;
			const_expr->is_float = 1;
			const_expr->float_value = node->float_cst.value;
			break;
		case CONST_STRING_TYPE:
			expr->final_type = CONST_STRING_TYPE;
			const_expr->is_string = 1;
			const_expr->string =(char*)(node->string.pointer);
			break;
		case UNDEFINED_TYPE:
			expr->final_type = UNDEFINED_TYPE;
			const_expr->is_undefined = 1;
			break;
	}

	expr->node = node;
	return expr;
}

int charge_type(int type1, int type2) {
	switch(type1) {
		case NO_TYPE:
		case CHAR_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == DOUBLE_TYPE) || (type2 == INT_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == FLOAT_TYPE)
					|| (type2 == NO_TYPE)) {
				return type2;
			}
			else if ((type1 == NO_TYPE) && (type2 == BOOL_TYPE)) {
				return type2;
			}
			else if ((type2 == BOOL_TYPE) || (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				/* FIXME: handle the warning */
				return type2;
			}
			else if ((type1 == NO_TYPE) && (type2 == CONST_STRING_TYPE)) {
				return type2;
			}
			else {
				error("(line: %d)incompatible types when assigning to type: %s", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case INT_TYPE:
		case INTEGER_TYPE:
			if ((type2 == DOUBLE_TYPE) || (type2 == INT_TYPE) || (type2 == FLOAT_TYPE)
					|| (type2 == LONG_TYPE) || (type2 == SHORT_TYPE)
					|| (type2 == SIGNED_TYPE) || (type2 == UNSIGNED_TYPE)) {
				return type2;
			}
			else if ((type2 == CHAR_TYPE) || (type2 == NO_TYPE) || (type2 == UNDEFINED_TYPE)
					|| (type2 == BOOL_TYPE) || type2 == INTEGER_TYPE) {
				return type1;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				/* FIXME: handle the warning */
				return type2;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case DOUBLE_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == DOUBLE_TYPE) || (type2 == INT_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == FLOAT_TYPE)
					|| (type2 == BOOL_TYPE) || (type2 == NO_TYPE)
					|| (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case FLOAT_TYPE:
			if (type2 == DOUBLE_TYPE) {
				return type2;
			}
			else if ((type2 == CHAR_TYPE) || (type2 == INT_TYPE) || (type2 == UNDEFINED_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == FLOAT_TYPE)
					|| (type2 == BOOL_TYPE) || (type2 == NO_TYPE)) {
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case LONG_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == INT_TYPE) || (type2 == NO_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == BOOL_TYPE)
					|| (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else if ((type2 == DOUBLE_TYPE) || (type2 == FLOAT_TYPE)) {
				return type2;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				return type2;
			}
			break;
		case SHORT_TYPE:
			if ((type2 == DOUBLE_TYPE) || (type2 == INT_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == FLOAT_TYPE)) {
				return type2;
			}
			else if ((type2 == CHAR_TYPE) || (type2 == NO_TYPE)
					|| (type2 == BOOL_TYPE) || (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				return type2;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
			}
			break;
		case SIGNED_TYPE:
		case UNSIGNED_TYPE:
			if ((type2 == DOUBLE_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SIGNED_TYPE) || (type2 == UNSIGNED_TYPE)
					|| (type2 == FLOAT_TYPE)) {
				return type2;
			}
			else if ((type2 == INT_TYPE) || (type2 == INTEGER_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == CHAR_TYPE)
					|| (type2 == NO_TYPE) || (type2 == BOOL_TYPE)
					|| (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				return type2;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case BOOL_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == DOUBLE_TYPE) || (type2 == INT_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == FLOAT_TYPE)
					|| (type2 == BOOL_TYPE)) {
				return type2;
			}
			else if ((type2 == NO_TYPE) || (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case POINTER_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == INT_TYPE) || (type2 == UNDEFINED_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == BOOL_TYPE)
					|| (type2 == POINTER_TYPE) || (type2 == NO_TYPE)) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case CONST_STRING_TYPE:
			if (type2 == CONST_STRING_TYPE) {
				return type2;
			}
			else if (type2 == POINTER_TYPE) {
				warning("assignment makes %s from %s without a cast", TYPENAME(type1), TYPENAME(type2));
				return type2;
			}
			else if ((type2 == NO_TYPE) || (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		case UNDEFINED_TYPE:
			return type2;
			break;
		case STRUCT_TYPE:
			if (type2 == STRUCT_TYPE) {
				/* FIXME: should charge the struct name */
				return type1;
			}
			else if (type2 == NO_TYPE) {
				return type1;
			}
			else {
				error("(line: %d)incompatible types when assigning to type(%s)", __LINE__, TYPENAME(type2));
				/* FIXME: handle the error */
//				exit(-1);
				return -1;
			}
			break;
		default:
			/* TODO: the UNDEFINED_TYPE */
			error("(line: %d)undefined type(%s)", __LINE__, TYPENAME(type1));
			/*FIXME: handle the error */
//			exit(-1);
			break;
	}

	return 0;
}

int charge_left_right_expr_type(expression_t* left_expr, expression_t* right_expr) {
	assert(left_expr != NULL);
	assert(right_expr != NULL);

	if ((left_expr->is_undeclare) || (right_expr->is_undeclare)) {
		return 0;
	}

	return charge_type(left_expr->final_type, right_expr->final_type);
}

void cal_assign_left(tree_t** node, symtab_t table, expression_t* left_expr, expression_t* right_expr) {
	assert(*node != NULL);
	assert(left_expr != NULL);
	assert(right_expr != NULL);

    if (table->no_check) {
        right_expr->node = *node;
        right_expr->final_type = NO_TYPE;
        return;
    }

	tree_t* left_node = (*node)->expr_assign.left;
    if (left_node->common.type == QUOTE_TYPE) {
        tree_t* ident = left_node->quote.ident;
        int quote_type = 0;
        if (strcmp(ident->ident.str, "this") == 0) {
            left_expr->final_type = INT_TYPE;
        }
        else {
            quote_type = check_refer_dml_struct(ident, table);
            if (quote_type == REGISTER_TYPE || quote_type == FIELD_TYPE
                    || quote_type == DATA_TYPE || quote_type == ATTRIBUTE_TYPE) {
                left_expr = cal_expression(&((*node)->expr_assign.left), table, left_expr);
            }
            else if (quote_type > 0 ){
                fprintf(stderr, "error: cannot assign to this expression, type: %d\n", quote_type);
                /* TODO: handle the error */
                exit(-1);
            }
            else {
                left_expr->is_undeclare = 1;
                left_expr->undecl_name = ident->ident.str;
            }
        }
    }
	else {
		left_expr = cal_expression(&((*node)->expr_assign.left), table, left_expr);
		if (left_expr->is_const) {
			PERROR("value required as left operand of assignment", left_expr->node->common.location);
			/* FIXME: should handle the error */
	//		exit(-1);
		}
	}

	if ((left_expr->is_undeclare) || (right_expr->is_undeclare)) {
		return ;
	}

	int type = 0;
	type = charge_left_right_expr_type(left_expr, right_expr);
	right_expr->final_type = type;

	right_expr->node = *node;

	return;
}

expression_t* cal_common_assign(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d\n", __FUNCTION__, __LINE__);

	expr = cal_expression(&((*node)->expr_assign.right), table, expr);
	if (expr->final_type == CONSTANT_TYPE) {
		tree_t* tmp = (*node)->expr_assign.right;
		expr->final_type = change_node_to_const(&((*node)->expr_assign.right), table, expr);
	}
	expression_t* left_expr = (expression_t*)gdml_zmalloc(sizeof(expression_t));
	cal_assign_left(node, table, left_expr, expr);
    if (left_expr->is_undeclare) {
        expr->is_undeclare = 1;
        expr->undecl_name = left_expr->undecl_name;
    }
	expr->is_const = 0;
	expr->node = *node;

	return expr;
}

expression_t* cal_assign_expr(tree_t** node, symtab_t table, expression_t* expr) {
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	assert(*node != NULL);
	assert(expr != NULL);
	assert(table != NULL);
	switch((*node)->expr_assign.type) {
			/* = */
		case EXPR_ASSIGN_TYPE:
			cal_common_assign(node, table, expr);
			break;
			/* += */
		case ADD_ASSIGN_TYPE:
			/* -= */
		case SUB_ASSIGN_TYPE:
			/* *= */
		case MUL_ASSIGN_TYPE:
			/* /= */
		case DIV_ASSIGN_TYPE:
			/* %= */
		case MOD_ASSIGN_TYPE:
			/* |= */
		case OR_ASSIGN_TYPE:
			/* &= */
		case AND_ASSIGN_TYPE:
			/* ^= */
		case XOR_ASSIGN_TYPE:
			/* <<= */
		case LEFT_ASSIGN_TYPE:
			/* >>== */
		case RIGHT_ASSIGN_TYPE:
			cal_binary_expr(node, table, expr);
			cal_common_assign(node, table, expr);
			break;
	}
	expr->node = *node;
	return expr;
}

expression_t* cal_ternary_expr(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	if ((((*node)->ternary.cond) == NULL)
			|| (((*node)->ternary.expr_true) == NULL)
			|| (((*node)->ternary.expr_false) == NULL)) {
//		fprintf(stderr, " The ternary need expression\n");
		PERROR("not expression in ternary operation", expr->node->common.location);
		/* FIXME: handle the error */
//		exit(-1);
	}

	int true_type, false_type;
	cal_expression(&((*node)->ternary.cond), table, expr);
	cal_expression(&((*node)->ternary.expr_true), table,  expr);
	true_type = expr->final_type;
	cal_expression(&((*node)->ternary.expr_false), table, expr);
	false_type = expr->final_type;
	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}
	/* TODO: the expression type node defined */
	if (true_type == false_type) {
		expr->final_type = true_type;
	}
	else {
		/* TODO: should determine the final type */
		expr->final_type = NO_TYPE;
	}
	expr->node = *node;

	DEBUG_EXPR("In %s, line = %d, node type: %s, type: %d, true type: %d, false_type: %d\n",
			__FUNCTION__, __LINE__, (*node)->common.name, expr->final_type, true_type, false_type);
	return expr;
}

int get_left_right_value(tree_t* left, tree_t* right, tree_t** new_node, expression_t* expr, operator_type_t op_type) {
	assert(left != NULL);
	assert(right != NULL);

	float left_value, right_value, final_value;
	int left_type, right_type, final_type;

	left_value = right_value = final_value = 0;
	left_type = right_type = final_type = 0;

	if (left->common.type == INTEGER_TYPE) {
		if (left->int_cst.out_64bit) {
//			fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", left->int_cst.int_str);
			PWARN("the binary operator have problem.(left operand: %s)",
					left->common.location, left->int_cst.int_str);
			return -1;
		}
		else {
			left_value = left->int_cst.value;
			left_type = INTEGER_TYPE;
		}
	}
	else if (left->common.type == FLOAT_TYPE) {
		left_value = left->float_cst.value;
		left_type = FLOAT_TYPE;
	}
	else {
//		fprintf(stderr, "line: %d, error: invalid operands to binary\n", __LINE__);
		PERROR("(line: %d)invalid operands(%s and %s) to binary",
				left->common.location, __LINE__, TYPENAME(left->common.type), TYPENAME(right->common.type));
		/* FXIME: handle the error */
//		exit(-1);
	}

	if (right->common.type == INTEGER_TYPE) {
		if (right->int_cst.out_64bit) {
//			fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", left->int_cst.int_str);
			PWARN("the binary operator have problem.(left operand: %s)",
					right->common.location, right->int_cst.int_str);
			return -1;
		}
		else {
			right_value = right->int_cst.value;
			right_type = INTEGER_TYPE;
		}
	}
	else if (right->common.type == FLOAT_TYPE) {
		right_value = right->float_cst.value;
		right_type = FLOAT_TYPE;
	}
	else {
//		fprintf(stderr, "The left(%s) or right(%s) node type is invalid operands\n",
//				right->common.name, right->common.name);
		PWARN("the left(%s) or right(%s) value is invalid oprand",
				left->common.location, left->common.name, right->common.name);
		return -1;
	}

	final_type = charge_type(left_type, right_type);
	switch(op_type) {
		case ADD_TYPE:
		case ADD_ASSIGN_TYPE:
			final_value = (left_value + right_value);
			break;
		case SUB_TYPE:
		case SUB_ASSIGN_TYPE:
			final_value = (left_value - right_value);
			break;
		case MUL_TYPE:
		case MUL_ASSIGN_TYPE:
			final_value = (left_value * right_value);
			break;
		case DIV_TYPE:
		case DIV_ASSIGN_TYPE:
			if ((right_type != INTEGER_TYPE) && (right_type != INT_TYPE) &&
				(right_type != CHAR_TYPE)    && (right_type != SHORT_TYPE) && (right_type != LONG_TYPE) &&
				(right_type != FLOAT_TYPE)   && (right_type != DOUBLE_TYPE) &&
				(right_type != SIGNED_TYPE)  && (right_type != UNSIGNED_TYPE)) {
//				fprintf(stderr, "dividor not int\n");
				PERROR("divisor is invalid type(%s)", right->common.location, TYPENAME(right->common.type));
				/* TODO: handle the error */
//				exit(-1);
				break;
			}
			final_value = (left_value / right_value);
			break;
		case EQ_OP_TYPE:
			expr->final_type = BOOL_TYPE;
			final_value = (left_value == right_value);
			break;
		case NE_OP_TYPE:
			expr->final_type = BOOL_TYPE;
			final_value = (left_value != right_value);
			break;
		case LESS_TYPE:
			expr->final_type = BOOL_TYPE;
			final_value = (left_value < right_value);
			break;
		case GREAT_TYPE:
			expr->final_type = BOOL_TYPE;
			final_value = (left_value > right_value);
			break;

	}

	if (final_type == FLOAT_TYPE) {
		char* float_str = (char*)gdml_zmalloc(sizeof(float));
		sprintf(float_str, "%f", final_value);
		*new_node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst), &expr->node->common.location);
		(*new_node)->float_cst.float_str = float_str;
		(*new_node)->float_cst.value = final_value;
		(*new_node)->common.print_node = print_float_literal;
		expr->final_type = final_type;
	}
	if ((final_type == INTEGER_TYPE) || (final_type == BOOL_TYPE)) {
		char* int_str = (char*)gdml_zmalloc(sizeof(int));
		int value = (int)final_value;
		sprintf(int_str, "%d", value);
		*new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &expr->node->common.location);
		(*new_node)->int_cst.value = (int)final_value;
		(*new_node)->int_cst.int_str = int_str;
		(*new_node)->common.print_node = print_interger;
		expr->final_type = final_type;
	}

	if ((left->common.sibling) || (left->common.child)
			|| (right->common.sibling) || (right->common.child)) {
//		fprintf(stderr, "In %s, the left or right node have sibling or child\n", __FUNCTION__);
		PERROR("(FUNC: %s, Line: %d) the left(%s) or right(%s) node have sibling or child",
				expr->node->common.location,
				__FUNCTION__, __LINE__, left->common.name, right->common.name);
		/* FIXME: only for debugging */
//		exit(-1);

		return -1;
	}

	return 0;
}

expression_t* cal_add_expr(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	tree_t* left = NULL;
	tree_t* right = NULL;
	operator_type_t op_type = 0;
	int left_type, right_type;

	if ((*node)->binary.type) {
		cal_expression(&((*node)->binary.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->binary.right), table, expr);
		right_type = expr->final_type;
		op_type = (*node)->binary.type;
		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->binary.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->binary.right), table, expr);
		}
		left = (*node)->binary.left;
		right = (*node)->binary.right;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;
		op_type = (*node)->expr_assign.type;
		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->expr_assign.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->expr_assign.right), table, expr);
		}
		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
	}

	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	if (charge_node_is_const(left) && charge_node_is_const(right)) {
		tree_t* new_node = NULL;
		if ((left->common.type == CONST_STRING_TYPE) &&
				(right->common.type == CONST_STRING_TYPE)) {
			int len = (left->string.length) + (right->string.length);
			char* str = (char*)gdml_zmalloc(sizeof(char) * len);
			sprintf(str, "%s", left->string.pointer);
			str = strcat(str, right->string.pointer);
			tree_t* str_node = create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string), &expr->node->common.location);
			str_node->string.length = len;
			str_node->string.pointer = str;
			str_node->common.print_node = print_string;
			new_node = str_node;
			expr->final_type = CONST_STRING_TYPE;
		}
		else if ((left->common.type == UNDEFINED_TYPE) &&
				(right->common.type == UNDEFINED_TYPE)){
			new_node = (tree_t*)create_node("undefined", UNDEFINED_TYPE, sizeof(struct tree_common), &expr->node->common.location);
			new_node->common.print_node = print_undefined;
			expr->final_type = UNDEFINED_TYPE;
		}
		else {
			if (get_left_right_value(left, right, &new_node, expr, op_type) != 0) {
				expr->final_type = charge_type(left_type, right_type);
				expr->node = *node;
				return expr;
			}
		}
		new_node->common.sibling = (*node)->common.sibling;
		new_node->common.child = (*node)->common.child;
		assgin_sibling_child_null(node);
		free(*node);
		*node = new_node;
		expr = get_const_expr_value(*node, expr);
	}
	else {
		expr->final_type = charge_type(left_type, right_type);
		expr->node = *node;
	}

	DEBUG_EXPR("In %s, line = %d, node type: %s, expr final_type: %d\n",
			__func__, __LINE__, (*node)->common.name, expr->final_type);

	return expr;
}

expression_t* binary_expr_common(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	tree_t* left = NULL;
	tree_t* right = NULL;
	int left_type, right_type;
	operator_type_t op_type = 0;
	left_type = right_type = 0;

	if ((*node)->binary.type) {
		cal_expression(&((*node)->binary.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->binary.right), table, expr);
		right_type = expr->final_type;

		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->binary.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->binary.right), table, expr);
		}

		left = (*node)->binary.left;
		right = (*node)->binary.right;
		op_type = (*node)->binary.type;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;

		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->expr_assign.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->expr_assign.right), table, expr);
		}

		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
		op_type = (*node)->expr_assign.type;
	}
	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	if (charge_node_is_const(left) && (charge_node_is_const(right))) {
		 tree_t* new_node = NULL;
		if ((left->common.type == CONST_STRING_TYPE) || (left->common.type == UNDEFINED_TYPE)
				|| (right->common.type == CONST_STRING_TYPE) || (right->common.type == UNDEFINED_TYPE)) {
//			fprintf(stderr, "The left(%s) or right(%s) node type is wrong!\n",
//					left->common.name, right->common.name);
			PWARN("the left(%s, %s) or right(%s, %s) node type is wrong",
					(*node)->common.location,
					left->common.name, TYPENAME(left->common.type),
					right->common.name, TYPENAME(right->common.type));
			expr->node = *node;
			/* TODO: handle the error */
			return expr;
		}
		else {
			if (get_left_right_value(left, right, &new_node, expr, op_type) != 0) {
				expr->final_type = charge_type(left_type, right_type);
				expr->node = *node;
				return expr;
			}
		}
		new_node->common.sibling = (*node)->common.sibling;
		new_node->common.child = (*node)->common.child;
		assgin_sibling_child_null(node);
		free(*node);
		*node = new_node;
		expr = get_const_expr_value(*node, expr);
	}
	else {
		expr->final_type = charge_type(left_type, right_type);
		expr->node = *node;
	}

	DEBUG_EXPR("In %s, line = %d, node type: %s, expr final_type: %d\n",
			__func__, __LINE__, (*node)->common.name, expr->final_type);

	return expr;
}

expression_t* binary_expr_int(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	tree_t* left = NULL;
	tree_t* right = NULL;
	operator_type_t op_type = 0;
	int left_type, right_type;
	left_type = right_type = 0;

	if ((*node)->binary.type) {
		cal_expression(&((*node)->binary.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->binary.right), table, expr);
		right_type = expr->final_type;
		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->binary.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->binary.right), table, expr);
		}

		left = (*node)->binary.left;
		right = (*node)->binary.right;
		op_type = (*node)->binary.type;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;
		if (left_type == CONSTANT_TYPE) {
			left_type = change_node_to_const(&((*node)->expr_assign.left), table, expr);
		}
		if (right_type == CONSTANT_TYPE) {
			right_type = change_node_to_const(&((*node)->expr_assign.right), table, expr);
		}

		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
		op_type = (*node)->expr_assign.type;
	}

	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	if (charge_node_is_const(left) && charge_node_is_const(right)) {
		if ((left->common.type == INTEGER_TYPE) &&
				(right->common.type == INTEGER_TYPE)) {
			int left_value, right_value, final_value;
			left_value = right_value = final_value = 0;
			if ((left->int_cst.out_64bit) || (right->int_cst.out_64bit)){
				PWARN("the binary operator have problems.(left: %s)", left->common.location, left->int_cst.int_str);
				expr->final_type = INTEGER_TYPE;
				expr->node = *node;
				return expr;
			}
			else {
				left_value = left->int_cst.value;
				right_value = right->int_cst.value;

				switch(op_type) {
					case MOD_TYPE:
					case MOD_ASSIGN_TYPE:
						final_value = (left_value % right_value);
						break;
					case LEFT_OP_TYPE:
					case LEFT_ASSIGN_TYPE:
						final_value = (left_value << right_value);
						break;
					case RIGHT_OP_TYPE:
					case RIGHT_ASSIGN_TYPE:
						final_value = (left_value >> right_value);
						break;
					case OR_TYPE:
					case OR_ASSIGN_TYPE:
						final_value = (left_value | right_value);
						break;
					case XOR_TYPE:
					case XOR_ASSIGN_TYPE:
						final_value = (left_value ^ right_value);
						break;
					case AND_TYPE:
					case AND_ASSIGN_TYPE:
						final_value = (left_value & right_value);
						break;
					default:
						PWARN("the binary operator is other type(%s, %s)",
								(*node)->common.location,
								(*node)->common.name,
								TYPENAME((*node)->common.type));
						expr->node = *node;
						return expr;
				}

				char* value_str = (char*)gdml_zmalloc(sizeof(int) * 64);
				sprintf(value_str, "%d", final_value);
				tree_t* new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &expr->node->common.location);
				new_node->int_cst.int_str = value_str;
				new_node->int_cst.value = final_value;
				new_node->common.print_node = print_interger;
				new_node->common.sibling = (*node)->common.sibling;
				new_node->common.child = (*node)->common.child;
				if ((left->common.sibling) || (left->common.child)
						|| (right->common.sibling) || (right->common.child)) {
//					fprintf(stderr, "The left or right node has sibling or child\n");
					PWARN("the left(%s) or right(%s) node has sibling or child",
							(*node)->common.location, left->common.name, right->common.name);
				}
				free((*node)->binary.left);
				free((*node)->binary.right);
				assgin_sibling_child_null(node);
				free(*node);
				*node = new_node;
				expr = get_const_expr_value(*node, expr);
			}
		}
		else {
//			fprintf(stderr, "The binary operation's type is wrong, left: %s, right: %s\n",
//					left->common.name, right->common.name);
			PWARN("the binary operation's type is wrong.(left: %s, right: %s)",
					(*node)->common.location, left->common.name, right->common.name);
			expr->final_type = charge_type(left_type, right_type);
			expr->node = *node;
		}
	}
	else {
		expr->final_type = charge_type(left_type, right_type);
		expr->node = *node;
	}
	DEBUG_EXPR("In %s, line = %d, node type: %s, expr final_type: %d\n",
			__func__, __LINE__, (*node)->common.name, expr->final_type);

	return expr;
}

expression_t* cal_binary_expr(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	operator_type_t type = 0;

	if ((*node)->binary.type) {
		type = (*node)->binary.type;
	}
	else {
		type = (*node)->expr_assign.type;
	}
	switch (type) {
			/* + */
		case ADD_TYPE:
			/*a += b : a = a + b */
		case ADD_ASSIGN_TYPE:
			expr = cal_add_expr(node, table, expr);
			break;
			/* - */
		case SUB_TYPE:
			/* a -= b : a = a - b */
		case SUB_ASSIGN_TYPE:
			/* * */
		case MUL_TYPE:
			/* a *= b : a = a * b */
		case MUL_ASSIGN_TYPE:
			/* / */
		case DIV_TYPE:
			/* a /= b : a = a / b */
		case DIV_ASSIGN_TYPE:
			expr = binary_expr_common(node, table, expr);
			break;
			/* == */
		case EQ_OP_TYPE:
			/* != */
		case NE_OP_TYPE:
			/* < */
		case LESS_TYPE:
			/* > */
		case GREAT_TYPE:
			/* <= */
		case LESS_EQ_TYPE:
			/* >= */
		case GREAT_EQ_TYPE:
			/* || */
		case OR_OP_TYPE:
			/* && */
		case AND_OP_TYPE:
			expr = binary_expr_common(node, table, expr);
			expr->final_type = BOOL_TYPE;
			break;
			/* % */
		case MOD_TYPE:
			/* %= */
		case MOD_ASSIGN_TYPE:
			/* << */
		case LEFT_OP_TYPE:
			/* <<= */
		case LEFT_ASSIGN_TYPE:
			/* >> */
		case RIGHT_OP_TYPE:
			/* >>== */
		case RIGHT_ASSIGN_TYPE:
			/* | */
		case OR_TYPE:
			/* |= */
		case OR_ASSIGN_TYPE:
			/* ^ */
		case XOR_TYPE:
			/* ^= */
		case XOR_ASSIGN_TYPE:
			/* & */
		case AND_TYPE:
			/* &= */
		case AND_ASSIGN_TYPE:
			expr = binary_expr_int(node, table, expr);
			break;
		default:
//			fprintf(stderr, "The binary operation is other type : %s\n", (*node)->common.name);
			PWARN("the binary operation is other type(%s, %s)",
					(*node)->common.location, (*node)->common.name, TYPENAME((*node)->common.type));
			expr->node = *node;
			break;
	}
	DEBUG_EXPR("In %s, line = %d, node->type: %s, expr->final_type: %d\n",
			__func__, __LINE__, (*node)->common.name, expr->final_type);
	return expr;
}

expression_t* unary_bit_non(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	cal_expression(&((*node)->unary.expr), table, expr);
	tree_t* unary_expr = (*node)->unary.expr;

	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	if (expr->final_type == CONSTANT_TYPE) {
		expr->final_type = change_node_to_const(&((*node)->unary.expr), table, expr);
	}

	if (charge_node_is_const(unary_expr)) {
		if (unary_expr->common.type != INTEGER_TYPE) {
//			fprintf(stderr, "The bit non operation expression should int\n");
			PERROR("the bit non operation expression(%s) require integer",
					(*node)->common.location, TYPENAME(unary_expr->common.type));
			/* FIXME: should handle the error */
//			exit(-1);
		}
		else {
			const int int_length = 64;
			int value = ~(unary_expr->int_cst.value);
			char* str = (char*)gdml_zmalloc(int_length);
			tree_t* new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &expr->node->common.location);
			new_node->int_cst.value = value;
			sprintf(str, "%d", value);
			new_node->int_cst.int_str = str;
			new_node->common.sibling = (*node)->common.sibling;
			new_node->common.child = (*node)->common.child;
			new_node->common.print_node = print_interger;
			new_node->common.translate = translate_integer_literal;
			expr = get_const_expr_value(new_node, expr);
			free((*node)->unary.expr);
			free_sibling(node);
			free_child(node);
			*node = new_node;
		}
	}
	else {
		if (charge_type_int(expr->final_type) == 0) {
			PERROR("(line: %d) invalid operands to binary",
					(*node)->common.location, __LINE__);
			/* TODO: handle the error */
//			exit(-1);
		}
		expr->node = *node;
	}

	expr->node = *node;
	return expr;
}

expression_t* unary_expr_common(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	cal_expression(&((*node)->unary.expr), table, expr);
	tree_t* unary_expr = (*node)->unary.expr;

	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	if (expr->final_type == CONSTANT_TYPE) {
		expr->final_type = change_node_to_const(&((*node)->unary.expr), table, expr);
	}

	if (charge_node_is_const(unary_expr)) {
		if ((unary_expr->common.type == CONST_STRING_TYPE)
				|| (unary_expr->common.type == UNDEFINED_TYPE)) {
//			fprintf(stderr, "The unary operation expression can not be the type: %s\n",
//					unary_expr->common.name);
			PERROR("the unary expression can't be type(%s)",
					(*node)->common.location, TYPENAME(unary_expr->common.type));
			/* TODO: handle the error */
//			exit(-1);
		}
		else {
			float value, final_value;
			int final_type;
			value = final_value = final_type = 0;
			if (unary_expr->common.type == INTEGER_TYPE) {
				if (unary_expr->int_cst.out_64bit) {
					fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", unary_expr->int_cst.int_str);
//					expr->node = *node;
					PERROR("the binary operator have problem.(left: %s, %s)",
							(*node)->common.location, unary_expr->int_cst.int_str, TYPENAME(unary_expr->common.type));
					/* TODO: handle the error */
//					exit(-1);
				}
				value = unary_expr->int_cst.value;
				final_type = INTEGER_TYPE;
			}
			else {
				value = unary_expr->float_cst.value;
				final_type = FLOAT_TYPE;
			}

			switch((*node)->unary.type) {
				case NEGATIVE_TYPE:
					final_value = -value;
					break;
				case CONVERT_TYPE:
					final_value = +value;
					break;
				case NON_OP_TYPE:
					final_value = !value;
					final_type = INTEGER_TYPE;
					break;
				case PRE_INC_OP_TYPE:
					final_value = ++value;
					break;
				case PRE_DEC_OP_TYPE:
					final_value = --value;
					break;
				case AFT_INC_OP_TYPE:
					final_value = value++;
					break;
				case AFT_DEC_OP_TYPE:
					final_value = value--;
					break;
				default:
					break;
			}

			tree_t* new_node = NULL;
			if (final_type == FLOAT_TYPE) {
				char* float_str = (char*)gdml_zmalloc(sizeof(char) * 64);
				sprintf(float_str, "%f", final_value);
				new_node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst), &expr->node->common.location);
				new_node->float_cst.float_str = float_str;
				new_node->float_cst.value = final_value;
				new_node->common.print_node = print_float_literal;
				expr->final_type = final_type;
			}
			if (final_type == INTEGER_TYPE) {
				char* int_str = (char*)gdml_zmalloc(sizeof(char) * 64);
				sprintf(int_str, "%f", final_value);
				new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst), &expr->node->common.location);
				new_node->int_cst.value = (int)final_value;
				new_node->int_cst.int_str = int_str;
				new_node->common.print_node = print_interger;
				expr->final_type = final_type;
			}

			new_node->common.sibling = (*node)->common.sibling;
			new_node->common.child = (*node)->common.child;

			free((*node)->unary.expr);
			free((*node)->unary.operat);
			assgin_sibling_child_null(node);
			free(*node);
			expr->node = new_node;
			*node = new_node;
			expr = get_const_expr_value(*node, expr);
		}
	}
	else {
		if ((*node)->unary.type == NON_OP_TYPE) {
			expr->final_type = INT_TYPE;
		}
		expr->node = *node;
	}

	return expr;
}

expression_t* cal_unary_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	switch((*node)->unary.type) {
			/* - */
		case NEGATIVE_TYPE:
			/* + */
		case CONVERT_TYPE:
			/* ! */
		case NON_OP_TYPE:
			/* ++a */
		case PRE_INC_OP_TYPE:
			/* --a */
		case PRE_DEC_OP_TYPE:
			/* a++ */
		case AFT_INC_OP_TYPE:
			/* a-- */
		case AFT_DEC_OP_TYPE:
			expr = unary_expr_common(node, table, expr);
			break;
			/* ~ */
		case BIT_NON_TYPE:
			expr = unary_bit_non(node, table, expr);
			break;
			/* & */
		case ADDR_TYPE:
			/* * */
		case POINTER_TYPE:
			/* defined */
		case DEFINED_TYPE:
			cal_expression(&((*node)->unary.expr), table, expr);
			if (expr->is_undeclare) {
				expr->node = *node;
				break;
			}
			if (expr->final_type == CONSTANT_TYPE) {
				expr->final_type = change_node_to_const(&((*node)->unary.expr), table, expr);
			}
			if (charge_node_is_const((*node)->unary.expr)) {
//				fprintf(stderr, "The unary operation expression can not constant\n");
				PERROR("the unary operation expression isn't constant", (*node)->common.location);
					/* FIXME: should handle the error */
//					exit(-1);
			}
			expr->node = *node;
			break;
			/* # */
		case EXPR_TO_STR_TYPE:
			if (charge_node_is_const(*node)) {
				expr = get_const_expr_value(*node, expr);
			}
			else {
				expr->node = *node;
			}
			break;
		default:
//			fprintf(stderr, "Wrong unary type: %s\n", (*node)->common.name);
			PWARN("wrong unary type(%s, %s)", (*node)->common.location,
					(*node)->common.name, TYPENAME((*node)->common.type));
			break;
	}
	DEBUG_EXPR("In %s, line = %d, node type: %s, expr->final_type: %d\n",
			__FUNCTION__, __LINE__, (*node)->common.name, expr->final_type);

	return expr;
}

expression_t* cal_cast_expr(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	cal_expression(&((*node)->cast.expr), table, expr);

	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	/* FIXME: should find the type symbol from symbol table */
	(*node)->cast.decl = parse_ctypedecl((*node)->cast.ctype, table);
	expr->node = *node;

	return expr;
}

expression_t* cal_sizeof_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	tree_t* node_expr = (*node)->sizeof_tree.expr;
	/* The sizeof operator can only be used on expressions
	 * To take the size of a datatype,
	 * the sizeoftype operator must be used.
	 * */
	if ((node_expr->common.type == CDECL_TYPE)
			&& (node_expr->cdecl.is_data)) {
//		fprintf(stderr, "sizeof type is datatype\n");
		PERROR("sizeof type is datatype", (*node)->common.location);
		/* should handle the error */
//		exit(-1);
	}
	cal_expression(&((*node)->sizeof_tree.expr), table, expr);
	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}
	expr->final_type = INT_TYPE;

	expr->node = *node;

	return expr;
}

expression_t* cal_quote_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	DEBUG_EXPR("IN %s, line = %d, node->type: %s\n",
			__func__, __LINE__, (*node)->common.name);

    if (table->no_check) {
        expr->node = *node;
        expr->final_type = NO_TYPE;
        return expr;
    }

	if (charge_node_is_const(*node)) {
		PERROR("left value(%s) required as unary \'&\' operand",
				(*node)->common.location, (*node)->common.name);
		/* TODO: handle the error */
//		exit(-1);
	}

	tree_t* ident = (*node)->quote.ident;

    /*to reference something in the DML object structure
     * the reference must be prefixed by '$' character*/
    int is_dml_struct = check_refer_dml_struct(ident, table);
    if (is_dml_struct == 0) {
        expr->is_undeclare = 1;
        expr->undecl_name = ident->ident.str;

        fprintf(stderr, "reference to unknown object '%s'\n", ident->ident.str);
        /* TODO: handle the error */
        //exit(-1);
    }

	cal_expression(&((*node)->quote.ident), table, expr);

	expr->node = *node;

	DEBUG_EXPR("IN %s, line = %d, node->type: %s, final_type: %d\n",
			__func__, __LINE__, (*node)->common.name, expr->final_type);

	return expr;
}

int is_c_type(int type) {
	if ((type == NO_TYPE) || (type == CHAR_TYPE) || (type == DOUBLE_TYPE)
			|| (type == INT_TYPE) || (type == INTEGER_TYPE)
			|| (type == SIGNED_TYPE) || (type == UNSIGNED_TYPE)
			|| (type == FLOAT_TYPE) || (type == POINTER_TYPE)
			|| (type == BOOL_TYPE) || (type == CONST_STRING_TYPE)) {
		return 1;
	}
	else
		return 0;
}

int get_method_param_type(symtab_t table, symbol_t symbol) {
	assert(symbol != NULL);
	int type = 0;

	params_t* param = (params_t*)(symbol->attr);
	 if (param->is_notype) {
		 return NO_TYPE;
	 }
	 else {
		 type = get_decl_type(param->decl);
		 if (type == TYPEDEF_TYPE) {
			 type = get_typedef_type(table, param->decl->type->typedef_name);
		 }

		 return type;
	 }

	return 0;
}

int get_parameter_type(symbol_t symbol) {
	assert(symbol != NULL);

	parameter_attr_t* parameter = (parameter_attr_t*)(symbol->attr);
	paramspec_t* spec = parameter->spec;

	if (spec == NULL) {
		return NO_TYPE;
	}

	if (spec->str)
		return CONST_STRING_TYPE;
	else
		return (spec->type);
}

int get_constant_type(symbol_t symbol) {
	assert(symbol != NULL);

	constant_attr_t* attr = (constant_attr_t*)(symbol->attr);

	return (attr->value->final_type);
}

int get_foreach_type(symbol_t symbol) {
	assert(symbol != NULL);

	foreach_attr_t* attr = (foreach_attr_t*)(symbol->attr);
	expression_t* expr = attr->expr;

	return (expr->final_type);
}

int get_typedef_type(symtab_t table, char* name) {
	assert(table != NULL);
	assert(name != NULL);

	int type = 0;

	symbol_t symbol = symbol_find(table, name, TYPEDEF_TYPE);

	if ((symbol == NULL) && (table->no_check == 1)) {
		symbol = get_symbol_from_root_table(name, TYPEDEF_TYPE);
	}


	if (symbol == NULL) {
//		fprintf(stderr, "%s not typdef\n", name);
		error("%s is not typedef", name);
		/* TODO: handle the error */
//		exit(-1);
	}

	typedef_attr_t* attr = symbol->attr;

	if (attr->base_type == TYPEDEF_TYPE) {
		char* type_name = attr->decl->type->typedef_name;
		type = get_typedef_type(table, type_name);
	}
	else {
		type = attr->base_type;
	}

	return type;
}

int get_allocate_type(symbol_t symbol) {
	assert(symbol != NULL);

	const char* str = NULL;
	int type = 0;
	parameter_attr_t* attr = symbol->attr;
	paramspec_t* spec = attr->spec;

	if (spec == NULL) {
//		fprintf(stderr, "Warning: '%s' no initialized value\n", symbol->name);
		warning("\'%s\' is not initialized", symbol->name);
		return NO_TYPE;
	}
	else if (spec->type != CONST_STRING_TYPE) {
//		fprintf(stderr, "error: allocate_type of attribute should be string\n");
		error("allocate_type of attribute(%s) require a string", TYPENAME(spec->type));
		/* TODO: handle the error */
//		exit(-1);
	}

	str = spec->str;

	if ((strcmp(str, "\"double\"") == 0) ||
				(strcmp(str, "\"f\"") == 0)) {
		type = DOUBLE_TYPE;
	}
	else if((strstr(str, "int")) != NULL) {
		if (((strstr(str, "uint") != NULL) && (strcmp(str, "\"uint32\"") > 0)) ||
				((strstr(str, "uint") == NULL) && (strcmp(str, "\"int32\"") > 0))) {
			type = LONG_TYPE;
		}
		else {
			type = INT_TYPE;
		}
	}
	else if (strcmp(str, "\"i\"") == 0) {
		type = INT_TYPE;
	}
	else if ((strcmp(str, "\"string\"") == 0) ||
			(strcmp(str, "\"s\"") == 0)) {
		type = CONST_STRING_TYPE;
	}
	else if ((strcmp(str, "\"bool\"") == 0) ||
			(strcmp(str, "\"b\"") == 0)) {
		type = BOOL_TYPE;
	}
	else if (strcmp(str, "\"d\"") == 0) {
		type = DATA_TYPE;
	}
	else if (strcmp(str, "\"o\"") == 0) {
		type = OBJECT_TYPE;
	}
	else if (strcmp(str, "\"D\"") == 0) {
		type = DICTIONARY_TYPE;
	}
	else if (strcmp(str, "\"n\"") == 0) {
		type = NIL_TYPE;
	}
	else if (strcmp(str, "\"a\"") == 0) {
		type = NO_TYPE;
	}
	else {
//		fprintf(stderr, "Warning: other attribute type: %s\n", str);
		error("other attribute type(%s)", str);
		/* TODO: handle the error, this is only for debugging */
//		exit(-1);
	}

	return type;
}

int get_attribute_type(symbol_t symbol) {
	assert(symbol);
	int type = 0;

	attribute_attr_t* attr = symbol->attr;
	symtab_t tmp_table = attr->common.table;
	symbol_t type_symbol = symbol_find_curr(tmp_table, "type", PARAMETER_TYPE);
	symbol_t allocate_type = symbol_find_curr(tmp_table, "allocate_type", PARAMETER_TYPE);
	if (allocate_type) {
		type  = get_allocate_type(allocate_type);
	}
	else if (type_symbol){
		type = get_allocate_type(type_symbol);
	}
	else {
//		fprintf(stderr, "The attribute has no type\n");
		error("the attribute has no type");
		/* TODO: The attribute is ok with no type? */
//		exit(-1);
	}

	return type;
}

int get_c_type(symtab_t table, symbol_t symbol) {
	assert(table != NULL);
	assert(symbol != NULL);
	DEBUG_EXPR("In %s, line = %d, symbol name: %s, type: %d\n",
			__FUNCTION__, __LINE__, symbol->name, symbol->type);
	int type = 0;

	switch(symbol->type) {
		case PARAM_TYPE:
			type = get_method_param_type(table, symbol);
			DEBUG_EXPR("IN %s, line = %d, param type: %d\n",
					__func__, __LINE__, type);
			break;
		case PARAMETER_TYPE:
			type = get_parameter_type(symbol);
			DEBUG_EXPR("IN %s, line = %d, parameter type: %d\n",
					__func__, __LINE__, type);
			break;
		case CONSTANT_TYPE:
			//type = get_constant_type(symbol);
			type = CONSTANT_TYPE;
			DEBUG_EXPR("\nIN %s, line = %d, constant type: %d\n",
					__func__, __LINE__, type);
			break;
		case FOREACH_TYPE:
			type = get_foreach_type(symbol);
			DEBUG_EXPR("\nIN %s, line = %d, foreach type: %d\n",
					__func__, __LINE__, type);
			break;
		case IDENT_TYPE:
			if (strcmp(symbol->name, "NULL") == 0) {
				type = POINTER_TYPE;
			}
			else {
				fprintf(stderr, "other identifier: %s\n", symbol->name);
			}
			break;
		case FUNCTION_TYPE:
			type = FUNCTION_TYPE;
			break;
		case FUNCTION_POINTER_TYPE:
			type = FUNCTION_POINTER_TYPE;
			break;
		case TYPEDEF_TYPE:
			{
				decl_t* decl = symbol->attr;
				type = get_typedef_type(table, decl->type->typedef_name);
				DEBUG_EXPR("In %s, line = %d, symbol name : %s, typedef_name: %s, type: %d\n",
					__func__, __LINE__, symbol->name, decl->type->typedef_name, type);
			}
			break;
		case METHOD_TYPE:
		case LOGGROUP_TYPE:
		case TEMPLATE_TYPE:
			type = NO_TYPE;
			break;
		case STRUCT_TYPE:
			DEBUG_EXPR("symbol name: %s\n", symbol->name);
			decl_t* decl = (decl_t*)(symbol->attr);
			if (strcmp(decl->type->struct_name, "cycles_t") == 0) {
				type = INT_TYPE;
			}
			else {
				type = STRUCT_TYPE;
			}
			break;
		case ATTRIBUTE_TYPE:
			type = get_attribute_type(symbol);
			break;
		case REGISTER_TYPE:
		case FIELD_TYPE:
			/* FIXME: register may be other type */
			type = INT_TYPE;
			break;
		case INTERFACE_TYPE:
			type = INTERFACE_TYPE;
			break;
		default:
			error("In %s, line = %d, other dml %s(%s)\n",
					__FUNCTION__, __LINE__, symbol->name, TYPENAME(symbol->type));
			/* FIXME: only for debugging */
			//exit(-1);
			break;
	}

	return type;
}

tree_t* bool_expression(tree_t** node, expression_t* expr, int value) {
	assert(*node != NULL);
	assert(expr != NULL);
	char* str = gdml_zmalloc(sizeof(int)); // only store '0' and '1'
	sprintf(str, "%d", value);
	expr->final_type = BOOL_TYPE;
	expr->is_const = 1;

	tree_t* new_node = (tree_t*)create_node("bool", BOOL_TYPE, sizeof(struct tree_int_cst), &expr->node->common.location);
	new_node->int_cst.value = value;
	new_node->int_cst.int_str = str;

	new_node->common.child = (*node)->common.child;
	new_node->common.sibling = (*node)->common.sibling;
	new_node->common.print_node = print_interger;

	assgin_sibling_child_null(node);
	free(*node);

	return new_node;
}

expression_t* get_ident_value(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_IDENT_VALUE("In %s, line = %d, node type: %s, table type: %d, ident: %s\n",
			__func__, __LINE__, (*node)->common.name, table->type, (*node)->ident.str);
	/* FIXME: should find the symbol, if it is in the symbol table,
	 * should insert it*/
	symbol_t symbol = symbol_find_notype(table, (*node)->ident.str);
	if ((symbol == NULL) && (table->no_check == 1)) {
		symbol = get_symbol_from_root_table((*node)->ident.str, 0);
	}
	pre_parse_symbol_t* pre_symbol =  pre_symbol_find((*node)->ident.str);
	if ((symbol != NULL)) {
		DEBUG_EXPR("symbol name: %s, type: %d\n", symbol->name, symbol->type);
		if (is_c_type(symbol->type) == 1) {
			expr->final_type = symbol->type;
		}
		else {
			/* TODO: should get the c type */
			expr->final_type = get_c_type(table, symbol);
			if (expr->final_type == FUNCTION_TYPE) {
				expr->func = symbol->attr;
			}
		}
	}
	else if (pre_symbol != NULL) {
			expr->final_type = pre_symbol->type;
	}
	else {
		if (strcmp((*node)->ident.str, "false") == 0) {
			*node = bool_expression(node, expr, 0);
		}
		else if (strcmp((*node)->ident.str, "true") == 0) {
			*node = bool_expression(node, expr, 1);
		}
		else if (strcmp((*node)->ident.str, "NULL") == 0) {
			expr->final_type = POINTER_TYPE;
		}
		else if (table->no_check) {
			DEBUG_TEMPLATE_SYMBOL("warning: %s no undeclared in template\n", (*node)->ident.str);
			//symbol_insert(table, node->ident.str, NO_TYPE, NULL);
			expr->final_type = NO_TYPE;
		}
		else {
			expr->is_undeclare = 1;
			expr->undecl_name = (*node)->ident.str;
		}
	}
	expr->node = *node;

	return expr;
}

reference_t*  get_bit_slic_ref(tree_t* node, reference_t* ref) {
	assert(node != NULL);
	assert(ref != NULL);

	tree_t* expr_node = node->bit_slic.expr;
	tree_t* ident_node = NULL;

	switch(expr_node->common.type) {
		case IDENT_TYPE:
		case DML_KEYWORD_TYPE:
			ref->name = node->ident.str;
			break;
		case QUOTE_TYPE:
			ident_node = expr_node->quote.ident;
			ref->name = ident_node->ident.str;
			break;
		default:
			error("line: %d, other node type: %s\n", __LINE__, expr_node->common.name);
			/* FIXME handle the error */
//			exit(-1);
			break;
	}

	return ref;
}

reference_t* get_reference(tree_t* node, reference_t* ref) {
	assert(node != NULL);
	assert(ref != NULL);

	tree_t* ident_node = NULL;
	reference_t* new = (reference_t*)gdml_zmalloc(sizeof(reference_t));

	switch (node->common.type) {
		case COMPONENT_TYPE:
			ident_node = node->component.ident;
			new->name = ident_node->ident.str;
			if (node->component.type == COMPONENT_POINTER_TYPE) {
				new->is_pointer = 1;
			}
			/* insert it in head */
			new->next = ref;
			new = get_reference(node->component.expr, new);
			break;
		case BIT_SLIC_EXPR_TYPE:
			new->next = ref;
			new = get_bit_slic_ref(node, new);
			break;
		case QUOTE_TYPE:
			ident_node = node->quote.ident;
			new->name = ident_node->ident.str;
			new->next = ref;
			break;
		case IDENT_TYPE:
		case DML_KEYWORD_TYPE:
			new->name = node->ident.str;
			new->next = ref;
			break;
		default:
			error("other node type: %s\n", node->common.name);
			/* FIXME: handle the error */
//			exit(-1);
			break;
	}
	return new;
}

void print_reference(reference_t* ref) {
	assert(ref != NULL);

	reference_t* tmp = ref;

	while (tmp) {
		DEBUG_EXPR("In %s, line = %d, reference name: %s---------------\n",
				__func__, __LINE__, tmp->name);

		tmp = tmp->next;
	}

	return;
}

symtab_t get_default_symbol_tale(symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;

	if (strcmp(symbol->name, "dev") == 0) {
		table = get_root_table();
	} // device
	else {
		error("other default symbol: %s\n", symbol->name);
		/* TODO: handle the error */
//		exit(-1);
	}

	return table;
}

symtab_t get_symbol_table(symbol_t symbol) {
	assert(symbol != NULL);

	symtab_t table = NULL;
	void* attr = symbol->attr;

	switch(symbol->type) {
		case TEMPLATE_TYPE:
			table = ((template_attr_t*)attr)->table;
			break;
		case STRUCT_TYPE:
			table = ((struct_attr_t*)attr)->table;
			break;
		case BITFIELDS_TYPE:
			table = ((bitfield_attr_t*)attr)->table;
			break;
		case LAYOUT_TYPE:
			table = ((layout_attr_t*)attr)->table;
			break;
		case BANK_TYPE:
		case REGISTER_TYPE:
		case FIELD_TYPE:
		case CONNECT_TYPE:
		case INTERFACE_TYPE:
		case ATTRIBUTE_TYPE:
		case EVENT_TYPE:
		case GROUP_TYPE:
		case PORT_TYPE:
		case IMPLEMENT_TYPE:
			table = ((struct object_common*)attr)->table;
			break;
		case PARAMETER_TYPE:
			table = get_default_symbol_tale(symbol);
			break;
		default:
			error("Othe symbol %s(%d)\n", symbol->name, symbol->type);
			/* FIXME: handle the error */
//			exit(-1);
	}

	return table;
}

static char* get_interface_name(const char* name) {
	assert(name != NULL);
	int name_len = strlen(name) + strlen("_interface_t") + 1;
	char* new_name = (char*)gdml_zmalloc(name_len);
	strcpy(new_name, name);
	strcat(new_name, "_interface_t");

	return new_name;
}

int check_reference(symtab_t table, reference_t* ref, expression_t* expr) {
	assert(table != NULL);
	assert(ref != NULL);

	reference_t* tmp = ref;
	symbol_t symbol = NULL;
	symbol_t ref_symbol = NULL;
	symtab_t ref_table = NULL;
	const int name_len = 256;
	int type = 0;

	if (table->no_check) {
		return NO_TYPE;
	}

	while (tmp->next) {
		symbol = symbol_find_notype(table, tmp->name);
		if (symbol != NULL) {
			if (symbol->type == INTERFACE_TYPE) {
				tmp->name = get_interface_name(symbol->name);
				continue;
			}
			else {
				ref_table = get_symbol_table(symbol);
			}
		}
		else {
			expr->is_undeclare = 1;
			expr->undecl_name = tmp->name;
			break;
		}

		if (ref_table == NULL) {
			expr->is_undeclare = 1;
			expr->undecl_name = tmp->name;
			break;
		}

		ref_symbol = symbol_find_notype(ref_table, tmp->next->name);
		if (ref_symbol == NULL) {
			expr->is_undeclare = 1;
			expr->undecl_name = tmp->next->name;
			break;
		}

		/* get the type about symbol */
		if (is_c_type(ref_symbol->type) == 1) {
			type = ref_symbol->type;
		}
		else {
			type = get_c_type(ref_table, ref_symbol);
		}

		if (type == INTERFACE_TYPE) {
			tmp->next->name = get_interface_name(tmp->next->name);
		}
		if (type == FUNCTION_POINTER_TYPE) {
			expr->final_type = FUNCTION_POINTER_TYPE;
			expr->func = ref_symbol->attr;
		}
		tmp = tmp->next;
	}

	return type;
}

expression_t* get_component_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	if (expr->is_undeclare) {
		return expr;
	}
	reference_t* reference = (reference_t*)gdml_zmalloc(sizeof(reference_t));
	/* TODO: should find the member, check it */
	if ((*node)->component.type == COMPONENT_POINTER_TYPE) {
		reference->is_pointer = 1;
	}
	tree_t* ident = (*node)->component.ident;
	reference->name = ident->ident.str;

	reference = get_reference((*node)->component.expr, reference);
	print_reference(reference);
	expr->final_type = check_reference(table, reference, expr);
	expr->node = *node;
	if (table->no_check) {
		//printf("\ninogore check about component!\n");
		return expr;
	}
	return expr;
}

expression_t* get_sizeoftype_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	/* FIXME: should charge the identifier type */
	expr->node = *node;
	return expr;
}

expression_t* get_new_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	parse_ctypedecl((*node)->new_tree.type, table);
	if ((*node)->new_tree.count) {
		cal_expression(&((*node)->new_tree.count), table, expr);
	}
	expr->node = *node;
	return expr;
}

decl_t* clear_decl_type(decl_t* decl) {
	assert(decl != NULL);
	decl_type_t* new_type = (decl_type_t*)gdml_zmalloc(sizeof(decl_type_t));
	free(decl->type);
	decl->type =  new_type;

	return decl;
}

type_t get_func_ret_type(symtab_t table, decl_t* decl) {
	assert(table != NULL);
	assert(decl != NULL);

	type_t type = get_decl_type(decl);
	if (type == TYPEDEF_TYPE) {
		type = get_typedef_type(table, decl->type->typedef_name);
		DEBUG_EXPR("In %s, line = %d, ret_decl: %s, type: %d\n",
				__func__, __LINE__, decl->type->typedef_name, type);
	}

	return type;
}

void charge_func_param(tree_t** node, symtab_t table, function_t* func) {
	/* function has no parameters */
	if (*node == NULL) {
		return;
	}
	tree_t* save_node = *node;
	func_param_t* param = func->param;

	int arg_num = get_param_num(*node);

	expression_t* expr = (expression_t*)gdml_zmalloc(sizeof(expression_t));
	int type = 0;
	int i = 0;
	while (*node) {
		expr = cal_expression(node, table, expr);
		type = get_decl_type(param->decl);
		if (type == TYPEDEF_TYPE) {
			type = get_typedef_type(table, param->decl->type->typedef_name);
		}
		if (expr->final_type == CONSTANT_TYPE) {
			constant_attr_t* attr = get_constant_attr(node, table);
			expr->final_type =  attr->value->final_type;
		}
		charge_type(expr->final_type, type);
		*node = (*node)->common.sibling;
		param = param->next;
		i++;
	}

	*node = save_node;
	arg_num = get_param_num(*node);

	return;
}

expression_t* get_brack_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	/* (expression)*/
	if ((*node)->expr_brack.expr_in_brack) {
		tree_t* in_node = (*node)->expr_brack.expr_in_brack;
		if (charge_node_is_const(in_node)) {
			expr = get_const_expr_value(in_node, expr);
		}
		else {
			expr = cal_expression(&((*node)->expr_brack.expr_in_brack), table, expr);
			if (expr->final_type == CONSTANT_TYPE) {
				expr->final_type = change_node_to_const(&((*node)->expr_brack.expr_in_brack), table, expr);
			}
			if (charge_node_is_const(in_node)) {
				expr = get_const_expr_value(in_node, expr);
			}
			else {
				expr->node = in_node;
			}
		}
	}

	/* expression (expression_list)*/
	if ((*node)->expr_brack.expr) {
		tree_t* out_node = (*node)->expr_brack.expr;
		if (charge_node_is_const(out_node)) {
			fprintf(stderr, "The expression is wrong!\n");
			/* FIXME: handle the error */
			exit(-1);
		}
		else {
			cal_expression(&((*node)->expr_brack.expr), table, expr);
		}
		if ((expr->final_type == FUNCTION_TYPE) ||
				(expr->final_type == FUNCTION_POINTER_TYPE)) {
			function_t* func = (function_t*)(expr->func);
			int arg_num = get_param_num((*node)->expr_brack.expr_in_brack);

			if (arg_num != (func->argc)) {
				fprintf(stderr, "error: too %s arguments to function : %s\n",
						(arg_num < func->argc) ? "few": "many", func->func_name);
				/* FIXME: handle the error */
				exit(-1);
			}
			else {
				charge_func_param(&(*node)->expr_brack.expr_in_brack, table, func);
			}

			arg_num = get_param_num((*node)->expr_brack.expr_in_brack);


			expr->final_type = get_func_ret_type(table, func->ret_decl);
		}
	}

	expr->node = *node;
	return expr;
}

expression_t* get_array_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	tree_t* node_arr = (*node)->array.expr;
	tree_t* pre_node = (*node)->array.expr;

	cal_expression(&((*node)->array.expr), table, expr);

	while(node_arr->common.sibling) {
		node_arr = node_arr->common.sibling;
		cal_expression(&node_arr, table, expr);
		/* node changet or node */
		if (node_arr != (pre_node->common.sibling)) {
			pre_node->common.sibling = node_arr;
		}
		pre_node = node_arr;
	}
	expr->node = *node;

	return expr;
}

expression_t* get_bit_slic_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);

	if (expr->is_undeclare) {
		return expr;
	}
	cal_expression(&((*node)->bit_slic.expr), table, expr);
	if (expr->is_const) {
		error("The bit slic declare should not const!\n");
//		exit(-1);
	}
	cal_expression(&((*node)->bit_slic.bit), table, expr);
	if ((*node)->bit_slic.bit_end) {
		cal_expression(&((*node)->bit_slic.bit_end), table, expr);
	}

	expr->is_const = 0;
	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}

	tree_t* endian = (*node)->bit_slic.endian;
	if (endian) {
		if ((strcmp(endian->ident.str, "le") != 0)
				|| (strcmp(endian->ident.str, "be") != 0)) {
			error("unknow bitorde: %s\n", endian->ident.str);
			/* TODO: handle the error */
//			exit(-1);
		}
	}
	expr->node = *node;

	return expr;
}

expression_t* cal_expression(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	if (expr->is_undeclare) {
		expr->node = *node;
		return expr;
	}
	switch((*node)->common.type) {
		case EXPR_ASSIGN_TYPE:
			expr = cal_assign_expr(node, table, expr);
			break;
		case TERNARY_TYPE:
			expr = cal_ternary_expr(node, table, expr);
			break;
		case BINARY_TYPE:
			expr = cal_binary_expr(node, table, expr);
			break;
		case CAST_TYPE:
			expr = cal_cast_expr(node, table, expr);
			break;
		case SIZEOF_TYPE:
			expr = cal_sizeof_expr(node, table, expr);
			break;
		case UNARY_TYPE:
			expr = cal_unary_expr(node, table, expr);
			break;
		case INTEGER_TYPE:
			expr = get_const_expr_value(*node, expr);
			break;
		case FLOAT_TYPE:
			expr = get_const_expr_value(*node, expr);
			break;
		case CONST_STRING_TYPE:
			expr = get_const_expr_value(*node, expr);
			break;
		case UNDEFINED_TYPE:
			expr = get_const_expr_value(*node, expr);
			break;
		case QUOTE_TYPE:
			expr = cal_quote_expr(node, table, expr);
			break;
		case IDENT_TYPE:
		case DML_KEYWORD_TYPE:
			expr = get_ident_value(node, table, expr);
			break;
		case COMPONENT_TYPE:
			expr = get_component_expr(node, table, expr);
			break;
		case SIZEOFTYPE_TYPE:
			expr = get_sizeoftype_expr(node, table, expr);
			break;
		case NEW_TYPE:
			expr = get_new_expr(node, table, expr);
			break;
		case EXPR_BRACK_TYPE:
			expr = get_brack_expr(node, table, expr);
			break;
		case ARRAY_TYPE:
			expr = get_array_expr(node, table, expr);
			break;
		case BIT_SLIC_EXPR_TYPE:
			expr = get_bit_slic_expr(node, table, expr);
			break;
		default:
			error("ther may be other type expression: %s\n", (*node)->common.name);
			/* FIXME: Pay attention: The exit function is only for debugging */
//			exit(-1);
			break;
	}

	expr->node = *node;
	return expr;
}

expression_t* parse_expression(tree_t** node, symtab_t table) {
	if (*node == NULL) {
		return NULL;
	}
	assert(table != NULL);

	DEBUG_EXPR("In %s, table num: %d\n", __FUNCTION__, table->table_num);

	expression_t* expr = (expression_t*)gdml_zmalloc(sizeof(expression_t));
	cal_expression(node, table, expr);
	if (expr->final_type == CONSTANT_TYPE) {
		expr->final_type = change_node_to_const(node, table, expr);
	}
	expr->node = *node;

	if (expr->is_undeclare) {
		undef_var_insert(table, *node);
	}

	DEBUG_EXPR("In %s, line = %d, expr final_type: %d\n",
			__func__, __LINE__, expr->final_type);

	return expr;
}

void parse_comma_expression(tree_t** node, symtab_t table) {
	if (*node == NULL) {
		return ;
	}
	assert(table != NULL);

	tree_t* expr_node = *node;
	while ((*node) != NULL) {
		DEBUG_EXPR("In %s, line = %d, node->type: %s\n",
				__func__, __LINE__, (*node)->common.name);
		parse_expression(node, table);
		*node = (*node)->common.sibling;
	}
	*node = expr_node;

	return ;
}

void parse_log_args(tree_t** node, symtab_t table) {
	if (*node == NULL) {
		return;
	}
	assert(table != NULL);

	tree_t** arg_node = node;

	parse_expression(node, table);

	while (((*arg_node)->common.sibling) != NULL) {
		*arg_node = (*arg_node)->common.sibling;
		parse_expression(arg_node, table);
	}

	return;
}
