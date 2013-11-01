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
	case OBJECT_TYPE:
		fprintf(stderr, "In %s, line = %d, object type\n", __FUNCTION__, __LINE__);
		exit(-1);
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
	//tree_t* new_node = copy_data_from_constant(attr->value->node);
	tree_t* new_node = NULL;
	new_node->common.sibling = (*node)->common.sibling;
	new_node->common.child = (*node)->common.child;
	assgin_sibling_child_null(node);
	free(*node);
	*node = new_node;
	cal_expression(node, table, expr);
	//return attr->value->final_type;
	return 0;
}

expression_t* get_const_expr_value(tree_t* node, expression_t* expr) {
	assert(node != NULL);
	/* FIXME: assert is noly for debugging */
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
	if (expr->is_constant_op) {
		fprintf(stderr, "Non constant expression '='\n");
		/* TODO: handle the error*/
		exit(-1);
	}
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
	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant exprsion: cast\n");
		/* TODO: handle the error*/
		exit(-1);
	}

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
	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : sizeof\n");
		/* TODO: handle the error*/
		exit(-1);
	}

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

	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : $%s\n", ident->ident.str);
		/* TODO: handle the error */
		exit(-1);
	}

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

#if 0
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
#endif

	return 0;
}

int get_parameter_type(symbol_t symbol) {
	assert(symbol != NULL);

	parameter_attr_t* parameter = (parameter_attr_t*)(symbol->attr);
	paramspec_t* spec = parameter->param_spec;

#if 0
	if (spec == NULL) {
		return NO_TYPE;
	}

	if (spec->str)
		return CONST_STRING_TYPE;
	else
		return (spec->type);
#endif

	return 0;
}

int get_constant_type(symbol_t symbol) {
	assert(symbol != NULL);

	constant_attr_t* attr = (constant_attr_t*)(symbol->attr);

	//return (attr->value->final_type);
	return 0;
}

int get_foreach_type(symbol_t symbol) {
	assert(symbol != NULL);

	foreach_attr_t* attr = (foreach_attr_t*)(symbol->attr);
	expr_t* expr = attr->expr;

	//return (expr->final_type);
	return 0;
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
	int type = 0;

#if 0
	const char* str = NULL;
	int type = 0;
	parameter_attr_t* attr = symbol->attr;
	//paramspec_t* spec = attr->spec;
	paramspec_t* spec = NULL;

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
#endif

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
	int *a = NULL;
	*a = 10;
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
//			exit(-1);
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
	if (expr->is_constant_op) {
		if (symbol != NULL) {
			if (symbol->type != CONSTANT_TYPE) {
				fprintf(stderr, "error: none-constant expression : %s\n", symbol->name);
				/* TODO: handle the error */
				exit(-1);
			}
			expr->final_type = CONSTANT_TYPE;
		}
		else {
			expr->is_undeclare = 1;
			expr->undecl_name = symbol->name;
		}
		expr->node = *node;
		return expr;
	}
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

static int check_dml_obj_refer(tree_t* node, symtab_t table);
reference_t*  get_bit_slic_ref(tree_t* node, reference_t* ref, expr_t* expr, symtab_t table) {
	assert(node != NULL); assert(ref != NULL);
	assert(expr != NULL); assert(table != NULL);

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
			if (check_dml_obj_refer(ident_node, table) == 0) {
				fprintf(stderr, "reference to unknown object '%s', table_num: %d, line: %d\n",
						ident_node->ident.str, table->table_num, __LINE__);
				/* TODO: handle the error */
				exit(-1);
			}
			else {
				expr->is_obj = 1;
			}
			break;
		default:
			error("line: %d, other node type: %s\n", __LINE__, expr_node->common.name);
			/* FIXME handle the error */
//			exit(-1);
			break;
	}

	return ref;
}

reference_t* get_reference(tree_t* node, reference_t* ref, expr_t* expr, symtab_t table) {
	assert(node != NULL); assert(ref != NULL);
	assert(expr != NULL); assert(table != NULL);

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
			new = get_reference(node->component.expr, new, expr, table);
			break;
		case BIT_SLIC_EXPR_TYPE:
			new->next = ref;
			new = get_bit_slic_ref(node, new, expr, table);
			new->is_array = 1;
			break;
		case QUOTE_TYPE:
			ident_node = node->quote.ident;
			new->name = ident_node->ident.str;
			if (check_dml_obj_refer(ident_node, table) == 0) {
				fprintf(stderr, "reference to unknown object '%s', table_num: %d, line: %d\n",
						ident_node->ident.str, table->table_num, __LINE__);
				/* TODO: handle the error */
				exit(-1);
			}
			else {
				expr->is_obj = 1;
			}
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

symtab_t get_object_table(symbol_t symbol) {
	assert(symbol != NULL);
	object_t* obj = symbol->attr;
	tree_t* node = obj->node;
	object_attr_t* attr = node->common.attr;
	//return attr->common.table;
	return obj->symtab;
}

symtab_t get_data_table(symbol_t symbol) {
	assert(symbol != NULL);
	printf("In %s, line = %d, Pay attention: not implemented\n", __FUNCTION__, __LINE__);
	exit(-1);

	return NULL;
}

symtab_t get_symbol_table(symbol_t symbol) {
	assert(symbol != NULL);

	symtab_t table = NULL;
	void* attr = symbol->attr;

	switch(symbol->type) {
		case TEMPLATE_TYPE:
			table = ((template_attr_t*)attr)->table;
			break;
		case STRUCT_T:
			table = ((cdecl_t*)attr)->struc.table;
			break;
		case BITFIELDS_T:
			table = ((cdecl_t*)attr)->layout.table;
			break;
		case LAYOUT_T:
			table = ((cdecl_t*)attr)->bitfields.table;
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
		case OBJECT_TYPE:
			table = get_object_table(symbol);
			break;
		case DATA_TYPE:
			table = get_data_table(symbol);
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

	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : %s\n",
				reference->is_pointer ? "->": ".");
		/* TODO: handle the error */
		exit(-1);
	}

#if 0
	reference = get_reference((*node)->component.expr, reference, expr, table);
	print_reference(reference);
	expr->final_type = check_reference(table, reference, expr);
	expr->node = *node;
	if (table->no_check) {
		//printf("\ninogore check about component!\n");
		return expr;
	}
#endif
	return expr;
}

expression_t* get_sizeoftype_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : sizeoftype\n");
		/* TODO: handle the error */
		exit(-1);
	}
	/* FIXME: should charge the identifier type */
	expr->node = *node;
	printf("In %s, line = %d, can not implement the sizeoftype\n", __FUNCTION__, __LINE__);
	/* TODO: it is only for debugging, delete */
	exit(-1);
	return expr;
}

expression_t* get_new_expr(tree_t** node, symtab_t table,  expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d, node type: %s\n",
			__func__, __LINE__, (*node)->common.name);
	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : new\n");
		/* TODO: handle the error */
		exit(-1);
	}

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
			//expr->final_type =  attr->value->final_type;
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

	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : []\n");
		/* TODO: handle the error */
		exit(-1);
	}

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
	if (expr->is_constant_op) {
		fprintf(stderr, "error: none-constant expression : []\n");
		/* TODO: handle the error */
		exit(-1);
	}

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

void parse_log_args(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return;
	}
	assert(table != NULL);

	tree_t* arg_node = node;

	check_expr(node, table);

	while ((arg_node->common.sibling) != NULL) {
		arg_node = arg_node->common.sibling;
		check_expr(arg_node, table);
	}

	return;
}


/* ------------------------------- the new struct about check expression -----------*/
/* get the type's unqualified version */
cdecl_t* unqual(cdecl_t* type) {
	if (type->common.qual)
		type = type->common.bty;
	return type;
}

expr_t* adjust(expr_t* expr, int rvalue) {
	assert(expr != NULL);
	if (rvalue) {
		expr->type = unqual(expr->type);
		expr->lvalue = 0;
	}

	if (expr->type->common.categ = FUNCTION_T) {
		expr->type = pointer_to(expr->type);
		expr->isfunc = 1;
	}
	else if (expr->type->common.categ == ARRAY_T) {
		expr->type = pointer_to(expr->type->common.bty);
		expr->lvalue = 0;
		expr->isarray = 1;
	}

	return expr;
}

static int can_modify(expr_t* expr) {
	assert(expr != NULL);

	if (expr->lvalue) {
		if ((expr->type->common.qual & CONST_QUAL) || expr->is_const) {
			return 0;
		}
	}
	//return ((expr->lvaue && ((expr->type->common.qual & CONST_QUAL) || expr->is_const)) ? 0 : 1)

	return 1;
}

#define cal_muti_const_value(expr, op) \
		if (both_int_type(expr->kids[0]->type, expr->kids[1]->type)) { \
			expr->type->common.categ = (expr->kids[0]->type->common.categ > expr->kids[1]->type->common.categ) ? expr->kids[0]->type->common.categ : expr->kids[1]->type->common.categ; \
			expr->val->int_v.value =expr->kids[0]->val->int_v.value op expr->kids[1]->val->int_v.value; \
		} \
		else if (is_int_type(expr->kids[0]->type) && is_double_type(expr->kids[1]->type)) { \
			expr->type->common.categ = DOUBLE_T; \
			expr->val->d = expr->kids[0]->val->int_v.value op expr->kids[1]->val->d; \
		} \
		else if (is_double_type(expr->kids[0]->type) && is_int_type(expr->kids[1]->type)) { \
			expr->type->common.categ = DOUBLE_T; \
			expr->val->d = expr->kids[0]->val->d op expr->kids[1]->val->int_v.value;  \
		} \
		else { \
			expr->type->common.categ = DOUBLE_T; \
			expr->val->d = expr->kids[0]->val->d op expr->kids[1]->val->d; \
		}

#define cal_int_const(expr, op) \
	if ((is_double_type(expr->kids[0]->type)) || is_double_type(expr->kids[1]->type)) { \
		error("invalid operands to binary op (have ‘double’ and ‘int’)\n"); \
		expr->type->common.categ = NO_TYPE; \
	} \
	else { \
		expr->type->common.categ = INT_T; \
		expr->val->int_v.value = expr->kids[0]->val->int_v.value op expr->kids[1]->val->int_v.value; \
	}

#define cal_unary(expr, op) \
	if (is_double_type(expr->kids[0]->type)) { \
		expr->type->common.categ = DOUBLE_T; \
		expr->val->d = op(expr->kids[0]->val->d); \
	} \
	else { \
		expr->type->common.categ = INT_T; \
		expr->val->int_v.value = op(expr->kids[0]->val->int_v.value); \
	}

#define cal_unary_int(expr, op) \
	if (is_double_type(expr->kids[0]->type)) { \
		error("wrong type argument to bit-complement\n"); \
		expr->type->common.categ = NO_TYPE; \
	} \
	else { \
		expr->type->common.categ = INT_T; \
		expr->val->int_v.value = op(expr->kids[0]->val->int_v.value); \
	}

expr_t* cal_const_value(expr_t* expr) {
	assert(expr != NULL);
	/* binary operation */
	if (((expr->op >= ADD_TYPE) && (expr->op <= AND_TYPE)) &&
		(!((expr->kids[0]->is_const) && (expr->kids[1]->is_const)))) {
			int categ1 = expr->kids[0]->type->common.categ;
			int categ2 = expr->kids[1]->type->common.categ;
			expr->type = categ1 > categ2 ? expr->kids[0]->type: expr->kids[1]->type;
			return expr;
	}

	if ((expr->op >= ADD_ASSIGN_TYPE) && (expr->op <= RIGHT_ASSIGN_TYPE)) {
		if (expr->kids[0]->is_const) {
			error("lvalue required as left operand of assignment\n");
		}
		else {
			int categ1 = expr->kids[0]->type->common.categ;
			int categ2 = expr->kids[1]->type->common.categ;
			expr->type = categ1 > categ2 ? expr->kids[0]->type: expr->kids[1]->type;
		}
		return expr;
	}

	/* unary operation */
	if (((expr->op >= NEGATIVE_TYPE) && (expr->op <= BIT_NON_TYPE))
			&& (expr->kids[0]->is_const == 0)) {
		expr->type = expr->kids[0]->type;
		return expr;
	}

	if (((expr->op >= PRE_INC_OP_TYPE) && (expr->op <= AFT_DEC_OP_TYPE))
			&& (expr->kids[0]->is_const == 0)) {
		expr->type = expr->kids[0]->type;
		return expr;
	}

	/*--------------- calculate the constant value about expression-------------*/
	expr->val = (value_t*)gdml_zmalloc(sizeof(value_t));
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->is_const = 1;
	switch(expr->op) {
		case ADD_TYPE:
			cal_muti_const_value(expr, +);
			break;
		case SUB_TYPE:
			cal_muti_const_value(expr, -);
			break;
		case MUL_TYPE:
			cal_muti_const_value(expr, *);
			break;
		case DIV_TYPE:
			cal_muti_const_value(expr, /);
			break;
		case MOD_TYPE:
			cal_int_const(expr, %);
			break;
			/* << */
		case LEFT_OP_TYPE:
			cal_int_const(expr, <<);
			break;
			/* >> */
		case RIGHT_OP_TYPE:
			cal_int_const(expr, >>);
			break;
			/* == */
		case EQ_OP_TYPE:
			cal_muti_const_value(expr, ==);
			break;
			/* != */
		case NE_OP_TYPE:
			cal_muti_const_value(expr, !=);
			break;
			/* < */
		case LESS_TYPE:
			cal_muti_const_value(expr, <);
			break;
			/* > */
		case GREAT_TYPE:
			cal_muti_const_value(expr, >);
			break;
			/* <= */
		case LESS_EQ_TYPE:
			cal_muti_const_value(expr, <=);
			break;
			/* >= */
		case GREAT_EQ_TYPE:
			cal_muti_const_value(expr, >=);
			break;
			/* || */
		case OR_OP_TYPE:
			cal_muti_const_value(expr, ||);
			break;
			/* && */
		case AND_OP_TYPE:
			cal_muti_const_value(expr, &&);
			break;
			/* | */
		case OR_TYPE:
			cal_int_const(expr, |);
			break;
			/* ^ */
		case XOR_TYPE:
			cal_int_const(expr, ^);
			break;
			/* & */
		case AND_TYPE:
			cal_int_const(expr, ^);
			break;
			/* - */
		case NEGATIVE_TYPE:
			cal_unary(expr, -);
			break;
			/* + */
		case CONVERT_TYPE:
			cal_unary(expr, +);
			break;
			/* ! */
		case NON_OP_TYPE:
			cal_unary(expr, !);
			break;
			/* ~ */
		case BIT_NON_TYPE:
			cal_unary_int(expr, ~);
			break;
		case PRE_INC_OP_TYPE:
			cal_unary_int(expr, ++);
			break;
		case PRE_DEC_OP_TYPE:
			cal_unary_int(expr, --);
			break;
		default:
			fprintf(stderr, "other constant operator: %d\n", expr->op);
			int *a = NULL;
			*a = 1000;
			exit(-1);
	}

	return expr;
}

expr_t* check_binary_kids(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	if (node->common.type == BINARY_TYPE) {
		expr->kids[0] = check_expr(node->binary.left, table);
		expr->kids[1] = check_expr(node->binary.right, table);
	}
	else if (node->common.type == EXPR_ASSIGN_TYPE){
		expr->kids[0] = check_expr(node->expr_assign.left, table);
		expr->kids[1] = check_expr(node->expr_assign.right, table);
	}
	else {
		fprintf(stderr, "other node type: %s\n", node->common.name);
		/* FIXME: this is only for debug, delete it*/
		exit(-1);
	}

	return expr;
}

cdecl_t* get_arith_type(cdecl_t* type1, cdecl_t* type2, cdecl_t* type) {
	assert(type1 != NULL); assert(type2 != NULL);
	if (type1->common.categ == LONG_T || type2->common.categ == LONG_T) {
		type->common.categ = LONG_T;
		type->common.size = sizeof(long) * 8;
		return type;
	}

	if (type1->common.categ == DOUBLE_T || type2->common.categ == DOUBLE_T) {
		type->common.categ = DOUBLE_T;
		type->common.size = sizeof(double) * 8;
		return type;
	}

	if (type1->common.categ == FLOAT_T || type2->common.categ == FLOAT_T) {
		type->common.categ = FLOAT_T;
		type->common.size = sizeof(float) * 8;
		return type;
	}

	if ((type1->common.categ >= BOOL_T) && (type1->common.categ < INT_T)) {
		type->common.categ = INT_T;
		type->common.size = sizeof(int) * 8;
	}
	if ((type2->common.categ >= BOOL_T) && (type2->common.categ < INT_T)) {
		type->common.categ = INT_T;
		type->common.size = sizeof(int) * 8;
	}

	if (type1->common.categ == type2->common.categ) {
		type->common.categ = type1->common.categ;
		type->common.size = type1->common.size;
	}

	if (type1->common.categ >= type2->common.categ)  {
		type = type1;
		return type;
	}

	if (type2->common.size > type1->common.size) {
		type = type2;
		return type;
	}

	return type;
}

#define expr_to_int(expr) \
		if (expr->is_const) { \
			if (expr->type->common.categ == DOUBLE_T) { \
				expr->val->int_v.value = (int)(expr->val->d); \
			} \
		} \
		else { \
			expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
		} \
		expr->type->common.categ = INT_T; \
		expr->type->common.size = sizeof(int) * 8;

expr_t* check_logical_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL);
	expr = check_binary_kids(node, table, expr);
	if (both_scalar_type(expr->kids[0]->type, expr->kids[1]->type)) {
		expr = cal_const_value(expr);
		if (expr->is_const) {
			if (expr->type->common.categ == DOUBLE_T) {
				expr->val->int_v.value = (int)(expr->val->d);
			}
		}
		else {
			expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
		}
		expr->type->common.categ = INT_T;
		expr->type->common.size = sizeof(int) * 8;
		return (expr);
	}
	error("Line: %d, Invalid operands\n", __LINE__);
	return expr;
}

expr_t* check_bitwise_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	cdecl_t* type1 = expr->kids[0]->type;
	cdecl_t* type2 = expr->kids[1]->type;
	type1 = (type1->common.categ == ARRAY_T) ? type1->common.bty : type1;
	type2 = (type2->common.categ == ARRAY_T) ? type2->common.bty : type2;
	if (both_int_type(type1, type2)) {
		expr->kids[0]->type = type1;
		expr->kids[1]->type = type2;
		expr = cal_const_value(expr);
		return expr;
	}
	else if (is_parameter_type(type1) || is_parameter_type(type2) ||
		(is_no_type(type1) || is_no_type(type2))) {
		expr->type = type1->common.categ > type2->common.categ ? type1 : type2;
		return expr;
	}
	error("Line: %d, Invalid operands, type1: %d, type2: %d\n",
		__LINE__, expr->kids[0]->type->common.categ, expr->kids[1]->type->common.categ);
	return expr;
}

expr_t* check_equality_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	cdecl_t* type1 = NULL;
	cdecl_t* type2 = NULL;

	expr = check_binary_kids(node, table, expr);
	type1 = expr->kids[0]->type;
	type2 = expr->kids[1]->type;
	type1 = (type1->common.categ == ARRAY_T) ? type1->common.bty : type1;
	type2 = (type2->common.categ == ARRAY_T) ? type2->common.bty : type2;
	if (both_arith_type(type1, type2)) {
		expr = cal_const_value(expr);
		expr_to_int(expr);
		return expr;
	}

	if ((is_ptr_type(type1) && is_ptr_type(type2)) ||
			(is_ptr_type(type1) && is_null_ptr(expr->kids[0])) ||
			(is_ptr_type(type2) && is_null_ptr(expr->kids[1]))) {
		new_int_type(expr);
		return expr;
	}
	if (is_parameter_type(type1) || is_parameter_type(type2) ||
		is_no_type(type1) || is_no_type(type2) ||
		is_ptr_type(type1) || is_ptr_type(type2)) {
		expr->type = (type1->common.categ > type2->common.categ) ? type1 : type2;
		return expr;
	}

	error("Invalid operands to: type1: %d, type2: %d\n", type1->common.categ, type2->common.categ);

	return expr;
}

expr_t* check_relation_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	if (both_arith_type(expr->kids[0]->type, expr->kids[1]->type)) {
		expr = cal_const_value(expr);
		expr_to_int(expr);
		return expr;
	}
	else if (no_common_type(expr->kids[0]->type) || no_common_type(expr->kids[0]->type)){
		error("Invalid operands to </>\n");
	}
	else {
		new_int_type(expr);
		return expr;
	}

	return expr;
}

expr_t* check_shift_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	if (both_int_type(expr->kids[0]->type, expr->kids[1]->type)) {
		expr = cal_const_value(expr);
		expr_to_int(expr);
		return expr;
	}
	else {
		error("Invalid operands\n");
	}

	return expr;
}

expr_t* check_add_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	if (both_arith_type(expr->kids[0]->type, expr->kids[1]->type)) {
		return cal_const_value(expr);
	}
	else if (both_no_common_type(expr->kids[0]->type, expr->kids[1]->type)) {
		error("Invalid operands\n");
	}
	else {
		int categ1 = expr->kids[0]->type->common.categ;
		int categ2 = expr->kids[1]->type->common.categ;
		expr->type = categ1 > categ2 ? expr->kids[0]->type : expr->kids[1]->type;
	}

	return expr;
}

expr_t* check_sub_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	if (both_arith_type(expr->kids[0]->type, expr->kids[1]->type)) {
		return cal_const_value(expr);
	}
	else if (both_no_common_type(expr->kids[0]->type, expr->kids[1]->type)) {
		error("Invalid operands\n");
	}
	else {
		int categ1 = expr->kids[0]->type->common.categ;
		int categ2 = expr->kids[1]->type->common.categ;
		expr->type = categ1 > categ2 ? expr->kids[0]->type: expr->kids[1]->type;
	}

	return expr;
}

expr_t* check_multiplicative_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	if (both_arith_type(expr->kids[0]->type, expr->kids[1]->type)) {
		return cal_const_value(expr);
	}
	else if (both_no_common_type(expr->kids[0]->type, expr->kids[1]->type)) {
		error("Line : %d, Invalid operands\n", __LINE__);
	}
	else {
		int categ1 = expr->kids[0]->type->common.categ;
		int categ2 = expr->kids[1]->type->common.categ;
		expr->type = categ1 > categ2 ? expr->kids[0]->type: expr->kids[1]->type;
	}

	return expr;
}

static cdecl_t* get_array_type(cdecl_t* arr_type) {
	assert(arr_type!= NULL);
	cdecl_t* type = arr_type->common.bty;
	if (is_arith_type(type))
		return type;
}

static int array_type_compatible(cdecl_t* type1, cdecl_t* type2);
static int two_type_compatible(cdecl_t* type1, cdecl_t* type2) {
	assert(type1 != NULL); assert(type2 != NULL);
	if (type1->common.categ == type2->common.categ)
		return 1;
	if (both_arith_type(type1, type2))
		return 1;
	if (is_ptr_type(type1) && is_ptr_type(type2))
		return 1;
	if ((is_ptr_type(type1) && is_int_type(type2)) ||
		is_ptr_type(type2) && is_int_type(type1)) {
		warning("conversion between pointer and integer without a cast\n");
		return 1;
	}
	if (is_no_type(type1) || is_no_type(type2))
		return 1;
	if (is_array_type(type1) || is_array_type(type2))
		return array_type_compatible(type1, type2);

	return 0;
}

static int array_type_compatible(cdecl_t* type1, cdecl_t* type2) {
	assert(type1 != NULL); assert(type2 != NULL);
	cdecl_t* tmp_type1 = NULL;
	cdecl_t* tmp_type2 = NULL;
	tmp_type1 = (type1->common.categ == ARRAY_T) ? type1->common.bty : type1;
	tmp_type2 = (type2->common.categ == ARRAY_T) ? type2->common.bty : type2;
	return two_type_compatible(tmp_type1, tmp_type2);
}

static int can_assign(expr_t* l_expr, expr_t* r_expr) {
	assert(l_expr != NULL); assert(r_expr != NULL);
	cdecl_t* l_type = unqual(l_expr->type);
	cdecl_t* r_type = unqual(r_expr->type);

	if (is_ptr_type(l_type) && is_null_ptr(r_expr)) {
		return 1;
	} else if (is_parameter_type(l_type) || is_parameter_type(r_type)){
		return 1;
	} else {
		return two_type_compatible(l_type, r_type);
	}


	return 0;
}

void free_expr(expr_t* expr) {

	return;
}

expr_t* check_assign_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	expr->op = node->expr_assign.type;
	if (node->expr_assign.type != EXPR_ASSIGN_TYPE) {
		switch(node->expr_assign.type) {
			case ADD_ASSIGN_TYPE:
				expr = check_add_expr(node, table, expr);
				break;
			case SUB_ASSIGN_TYPE:
				expr = check_sub_expr(node, table, expr);
				break;
			case MUL_ASSIGN_TYPE:
			case DIV_ASSIGN_TYPE:
			case MOD_ASSIGN_TYPE:
				expr = check_multiplicative_expr(node, table, expr);
				break;
			case OR_ASSIGN_TYPE:
			case AND_ASSIGN_TYPE:
			case XOR_ASSIGN_TYPE:
				expr = check_bitwise_expr(node, table, expr);
				break;
			case LEFT_ASSIGN_TYPE:
			case RIGHT_ASSIGN_TYPE:
				expr = check_shift_expr(node, table, expr);
				break;
		}
	}
	else {
		expr->kids[0] = check_expr(node->expr_assign.left, table);
		expr->kids[1] = check_expr(node->expr_assign.right, table);
	}

	if (!can_modify(expr->kids[0])) {
		error("lvalue required as left operand of assignment\n");
	}
	if (!can_assign(expr->kids[0], expr->kids[1])) {
		error("wrong assignment\n");
	}
	expr->type = expr->kids[0]->type;
	expr->node = node;

	free_expr(expr->kids[0]); free_expr(expr->kids[1]);
	return expr;
}

expr_t* check_binary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL);
	expr->op = node->binary.type;
	switch(node->binary.type) {
		case OR_OP_TYPE:
		case AND_OP_TYPE:
			expr = check_logical_expr(node, table, expr);
			break;
		case OR_TYPE:
		case XOR_TYPE:
		case AND_TYPE:
			expr = check_bitwise_expr(node, table, expr);
			break;
		case EQ_OP_TYPE:
		case NE_OP_TYPE:
			expr = check_equality_expr(node, table, expr);
			break;
		case LESS_TYPE:
		case GREAT_TYPE:
		case LESS_EQ_TYPE:
		case GREAT_EQ_TYPE:
			expr = check_relation_expr(node, table, expr);
			break;
		case LEFT_OP_TYPE:
		case RIGHT_OP_TYPE:
			expr = check_shift_expr(node, table, expr);
			break;
		case ADD_TYPE:
			expr = check_add_expr(node, table, expr);
			break;
		case SUB_TYPE:
			expr = check_sub_expr(node, table, expr);
			break;
		case MUL_TYPE:
		case DIV_TYPE:
		case MOD_TYPE:
			expr = check_multiplicative_expr(node, table, expr);
			break;
		default:
			fprintf(stderr, "unkown type: %s\n", node->common.name);
			exit(-1);
			break;
	}
	expr->node = node;
	return expr;
}

expr_t* check_ternary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	expr_t* cond_expr = check_expr(node->ternary.cond, table);
	//expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	if (!is_scalar_type(cond_expr->type)) {
		error("used struct type value where scalar is required\n");
	}

	expr_t* true_expr = check_expr(node->ternary.expr_true, table);
	expr_t* false_expr = check_expr(node->ternary.expr_false, table);
	cdecl_t* type1 = true_expr->type;
	cdecl_t* type2 = false_expr->type;

	expr->type = type1->common.categ > type2->common.categ ? type1 : type2;
	expr->node = node;

	return expr;
}

static expr_t* check_expression(tree_t* node, symtab_t table, expr_t* expr);
expr_t* check_cast_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_expression(node->cast.expr, table, expr);

	cdecl_t* type = parse_ctype_decl(node->cast.ctype, table);
	expr->type = type;
	expr->node = node;

	return expr;
}

expr_t* check_sizeof_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr = check_expression(node->sizeof_tree.expr, table, expr);
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;

	return expr;
}

expr_t* check_unary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr->op = node->unary.type;
	//expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	switch(node->unary.type) {
		case NEGATIVE_TYPE:
		case CONVERT_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (is_arith_type(expr->kids[0]->type)) {
				expr = cal_const_value(expr);
			}
			else if (no_common_type(expr->kids[0]->type)){
				error("wrong type argument to decrement\n");
			}
			else {
				expr->type = expr->kids[0]->type;
			}
			break;
		case NON_OP_TYPE:
		case BIT_NON_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (is_int_type(expr->kids[0]->type)) {
				expr = cal_const_value(expr);
			}
			else if (no_common_type(expr->kids[0]->type)){
				error("wrong type argument to decrement\n");
			}
			else {
				expr->type = expr->kids[0]->type;
			}
			break;
		case ADDR_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (expr->kids[0]->is_const) {
				error("lvalue required as unary ‘&’ operand\n");
			}
			else {
				expr->type = pointer_to(expr->kids[0]->type);
			}
			break;
		case POINTER_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (expr->kids[0]->type->common.categ != POINTER_T) {
				error("invalid type argument of ‘unary *’\n");
			}
			else {
				expr->type = expr->kids[0]->type->common.bty;
			}
			break;
		case DEFINED_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			//expr->type = expr->kids[0]->type;
			expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			expr->type->common.categ = BOOL_T;
			expr->type->common.size = sizeof(int) * 8;
			break;
		case EXPR_TO_STR_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (expr->kids[0]->is_const) {
				expr->type->common.categ = STRING_T;
				if (expr->type->common.categ == INT_T) {
					expr->val->p = gdml_zmalloc(sizeof(long)*8);
					sprintf(expr->val->p, "%lld", (expr->val->int_v.value));
				}
				else if (expr->type->common.categ == DOUBLE_T) {
					expr->val->p = gdml_zmalloc(sizeof(double)*8);
					sprintf(expr->val->p, "%lf", expr->val->d);
				}
			}
			else {
				error("The '#' operator must be constant value\n");
			}
			break;
		case PRE_INC_OP_TYPE:
		case PRE_DEC_OP_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (is_int_type(expr->kids[0]->type)) {
				expr = cal_const_value(expr);
			}
			else if (is_array_type(expr->kids[0]->type)){
				expr->type = expr->kids[0]->type;
			}
			else {
				error("lvalue required as increment/decrement operand\n");
			}
			break;
		case AFT_INC_OP_TYPE:
		case AFT_DEC_OP_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if ((is_int_type(expr->kids[0]->type) || is_array_type(expr->kids[0]->type))) {
				expr->type = expr->kids[0]->type;
			}
			else {
				error("lvalue required as increment/decrement operand\n");
			}
			break;
		default:
			fprintf(stderr, "other unary type: %s(%d)\n", node->common.name, node->unary.type);
			exit(-1);
	}

	expr->node = node;
	return expr;
}

expr_t* check_const_expr(tree_t* node, expr_t* expr) {
	assert(node != NULL); assert(expr != NULL);
	value_t* value = (value_t*)gdml_zmalloc(sizeof(value_t));
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->is_const = 1;
	expr->type = type;
	switch (node->common.type) {
		case INTEGER_TYPE:
			expr->type->common.categ = INT_T;
			expr->type->common.size = sizeof(long long int) * 8;
			expr->is_const = 1;
			expr->val = value;
			if (node->int_cst.out_64bit) {
				value->int_v.out_64bit = 1;
				value->int_v.int_str = strdup(node->int_cst.int_str);
			}
			else {
				value->int_v.value = node->int_cst.value;
			}
			break;
		case FLOAT_TYPE:
			expr->type->common.categ = DOUBLE_T;
			expr->type->common.size = sizeof(double) * 8;
			expr->val = value;
			expr->is_const = 1;
			value->d = node->float_cst.value;
			break;
		case CONST_STRING_TYPE:
			expr->type->common.categ = STRING_T;
			expr->type->common.size = sizeof(char*) * 8;
			expr->val = value;
			expr->is_const = 1;
			value->p = strdup(node->string.pointer);
			break;
		case UNDEFINED_TYPE:
			expr->is_const = 0;
			expr->is_undefined = 1;
			expr->type->common.categ = INT_T;
			free(value); value = NULL;
			break;
		default:
			fprintf(stderr, "other const type: %s\n", node->common.name);
			/* TODO: only for debugging*/
			exit(-1);
			break;
	}

	expr->node = node;
	return expr;
}

static int dml_obj_type(symbol_t symbol) {
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
	case OBJECT_TYPE:
            refer_type = type;
            break;
        default:
            refer_type = 0;
            break;
    }
    return refer_type;
}

static int find_dml_obj(symtab_t table, const char* name) {
	assert(table != NULL); assert(name != NULL);
	symbol_t symbol = symbol_find_notype(table, name);
	int obj_type = 0;
	if (symbol)
	if (symbol) {
		obj_type = dml_obj_type(symbol);
		if (obj_type) {
		    return obj_type;
		}
		else {
			symtab_t root_table = get_root_table();
			symbol = symbol_find_notype(root_table, name);
			obj_type = dml_obj_type(symbol);
			if (obj_type) {
			    return obj_type;
			}
		}
	}

	return obj_type;
}

static int check_dml_obj_refer(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	int obj_type = 0;
	char* refer_name = (char*)(node->ident.str);
	if (strcmp(refer_name, "this") == 0) {
		obj_type = 1;
	} else {
		obj_type = find_dml_obj(table, refer_name);
	}

	return obj_type;

}

expr_t* check_quote_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	tree_t* ident = node->quote.ident;

    /*to reference something in the DML object structure
     * the reference must be prefixed by '$' character*/
	int is_obj = check_dml_obj_refer(ident, table);
	if (is_obj == 0) {
		fprintf(stderr, "reference to unknown object '%s', table_num: %d, line: %d\n",
			ident->ident.str, table->table_num, __LINE__);
		/* TODO: handle the error */
		exit(-1);
	}

	expr->node = node;
	expr = check_expression(node->quote.ident, table, expr);

	return expr;
}

static cdecl_t* check_parameter_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL);
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->val = (value_t*)gdml_zmalloc(sizeof(value_t));
	parameter_attr_t* parameter = (parameter_attr_t*)(symbol->attr);
	paramspec_t* spec = parameter->param_spec;
	param_type_t param_type = spec->value->type;
	if (parameter->is_original) {
		if (spec == NULL) {
			type->common.categ = NO_TYPE;
			return type;
		}
		param_type = spec->value->type;
	} else {
		param_type = ((param_value_t*)parameter)->type;
	}

	if (param_type == PARAM_TYPE_NONE || param_type == PARAM_TYPE_LIST ||
		param_type == PARAM_TYPE_REF) {
		type->common.categ = PARAMETER_TYPE;
	}
	else if (param_type == PARAM_TYPE_BOOL) {
		type->common.categ = BOOL_T;
		expr->is_const = 1;
		expr->val->int_v.value = ((param_value_t*)parameter)->u.boolean;
	}
	else if (param_type == PARAM_TYPE_STRING) {
		type->common.categ = STRING_T;
		if (parameter->is_original) {
			if (spec->value->is_const) {
				expr->is_const = 1;
				expr->val->p = strdup(spec->value->u.string);
			}
		} else {
			if (((param_value_t*)parameter)->u.string) {
				expr->val->p = (void*)(((param_value_t*)parameter)->u.string);
			}
		}
	}
	else if (param_type == PARAM_TYPE_UNDEF) {
		type->common.categ = PARAMETER_TYPE;
		expr->is_const = 1;
		expr->is_undefined = 1;
	}
	else if (param_type == PARAM_TYPE_INT) {
		type->common.categ = INT_T;
		if (parameter->is_original) {
			expr->is_const = spec->value->is_const;
			expr->val->int_v.value = spec->value->u.integer;
		} else {
			expr->val->int_v.value = ((param_value_t*)parameter)->u.integer;
		}
	}
	else if (param_type == PARAM_TYPE_FLOAT) {
		type->common.categ = FLOAT_T;
		if (parameter->is_original) {
			expr->is_const = spec->value->is_const;
			expr->val->d = spec->value->u.floating;
		} else {
			expr->val->d = ((param_value_t*)parameter)->u.floating;
		}
	}
	else {
		type->common.categ = NO_TYPE;
		/* Pay attention: this is only for debugging */
		fprintf(stderr, "other parameter type\n");
		exit(-1);
	}
	return type;
}

static cdecl_t* check_constant_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	constant_attr_t* attr = (constant_attr_t*)(symbol->attr);
	cdecl_t* type = attr->value->type;
	expr->is_const = attr->value->is_const;
	expr->is_null = attr->value->is_null;
	expr->val = attr->value->val;
	if (expr->is_const == 0) {
		error("non-constant value for constant\n");
	}

	return type;
}

static cdecl_t* check_method_param_type(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	params_t* param = (params_t*)(symbol->attr);
	return param->decl;
}

static cdecl_t* check_foreach_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	foreach_attr_t* attr = (foreach_attr_t*)(symbol->attr);
	return attr->expr->type;
}

static cdecl_t* check_register_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	register_attr_t* attr = symbol->attr;
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = INT_T;
	type->common.size = sizeof(int) * 8;

	return type;
}

static cdecl_t* check_attribute_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	attribute_attr_t* attr = (attribute_attr_t*)(symbol->attr);
	attribute_t* attr_obj = (attribute_t*)(attr->attr_obj);
	if (attr_obj->alloc_type) {
		if (attr_obj->alloc == INT_T) {
			new_int_type(expr);
		} else if (attr_obj->alloc == LONG_T) {
			new_long_type(expr);
		} else if (attr_obj->alloc == DOUBLE_T) {
			new_doule_type(expr);
		} else {
			new_char_type(expr);
			expr->type = pointer_to(expr->type);
		}
	}
	else if (attr_obj->type) {
		if (attr_obj->ty == INT_T) {
			new_int_type(expr);
		} else if (attr_obj->ty == FLOAT_T) {
			new_doule_type(expr);
		} else if (attr_obj->ty == STRING_T) {
			new_char_type(expr);
			expr->type = pointer_to(expr->type);
		} else if (attr_obj->ty == BOOL_T) {
			new_int_type(expr);
			expr->type->common.categ == BOOL_T;
		}
		else {
			fprintf(stderr, "other attribute type: %s\n", attr_obj->type);
			exit(-1);
		}
	}
	else {
		new_int_type(expr);
	}
	return expr->type;
}

static cdecl_t* get_common_type(symtab_t table, symbol_t symbol, expr_t* expr);
static cdecl_t* get_obj_type(symbol_t symbol, symtab_t table, expr_t* expr) {
	assert(symbol != NULL); assert(table != NULL); assert(expr != NULL);

	object_t* obj = symbol->attr;
	tree_t* node = obj->node;
	cdecl_t* type = NULL;
	if ((node == NULL) && (!strcmp(obj->obj_type, "field"))) {
		type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
		type->common.categ = INT_T;
		return type;
	}

	object_attr_t* attr = node->common.attr;
	symbol_t new_symbol = (symbol_t)gdml_zmalloc(sizeof(struct symbol));
	new_symbol->name = symbol->name;
	new_symbol->type = attr->common.obj_type;
	new_symbol->attr = attr;
	type = get_common_type(table, new_symbol, expr);
	free(new_symbol);

	return type;
}

static cdecl_t* get_common_type(symtab_t table, symbol_t symbol, expr_t* expr) {
	assert(table != NULL); assert(symbol != NULL);

	cdecl_t* type = NULL;
	switch(symbol->type) {
		case DATA_TYPE:
			type = (cdecl_t*)(symbol->attr);
			break;
		case PARAM_TYPE:
			type = ((params_t*)(symbol->attr))->decl;
			break;
		case PARAMETER_TYPE:
			type = check_parameter_type(symbol, expr);
			break;
		case CONSTANT_TYPE:
			type = check_constant_type(symbol, expr);
			break;
		case FOREACH_TYPE:
			type = check_foreach_type(symbol, expr);
			break;
		case FUNCTION_T:
			type = (cdecl_t*)(symbol->attr);
			break;
		case METHOD_TYPE:
		case LOGGROUP_TYPE:
		case TEMPLATE_TYPE:
			printf("IN %s, line = %d, method type: %d, not implement(exit)\n",
					__func__, __LINE__, symbol->type);
			exit(-1);
			break;
		case ATTRIBUTE_TYPE:
			type = check_attribute_type(symbol, expr);
			break;
		case REGISTER_TYPE:
			type = check_register_type(symbol, expr);
			break;
		case FIELD_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			break;
		case INTERFACE_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INTERFACE_T;
			break;
		case NO_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = NO_TYPE;
			break;
		case OBJECT_TYPE:
			type = get_obj_type(symbol, table, expr);
			break;
		default:
			error("In %s, line = %d, other dml %s(%s)\n",
					__FUNCTION__, __LINE__, symbol->name, TYPENAME(symbol->type));
			/* FIXME: only for debugging */
//			exit(-1);
			break;
	}
	return type;
}

expr_t* check_ident_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	symbol_t symbol = symbol_find_notype(table, node->ident.str);
	if (symbol == NULL) {
		symtab_t root_table = get_root_table();
		symbol = symbol_find_notype(root_table, node->ident.str);
	}
	if (symbol != NULL) {
		DEBUG_EXPR("symbol name: %s, type: %d\n", symbol->name, symbol->type);
		if (is_common_type(symbol->type)) {
			expr->type = symbol->attr;
		}
		else {
			expr->type = get_common_type(table, symbol, expr);
		}
	}
	else {
		const char* str = node->ident.str;
		cdecl_t* type = NULL;
		if ((strcmp(str, "false") == 0) || (strcmp(str, "true") == 0)) {
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = BOOL_T;
			type->common.size = sizeof(int) * 8;
			expr->type = type;
			expr->is_const = 1;
		}
		else if (strcmp(str, "NULL") == 0) {
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = POINTER_T;
			type->common.size = sizeof(void*) * 8;
			expr->type = type;
			expr->is_const = 1;
			expr->is_null = 1;
		}
		else if (strcmp(str, "this") == 0) {
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			/* This may be some problems */
			type->common.categ = INT_T;
			expr->type = type;
		}
		else {
			error("%s no undeclared in template\n", str);
		}
	}

	expr->node = node;
	return expr;
}

#define object_is_array(attr, type) \
	if (((type*)attr)->is_array) { \
		return 1; \
	} else { \
		fprintf(stderr, "subscripted value is neither array nor vector\n"); \
		exit(-1); \
	}

static int object_array(symbol_t symbol) {
        assert(symbol != NULL);
        object_t* obj = symbol->attr;
        void* attr = obj->node->common.attr;
        if (!strcmp(obj->obj_type, "register")) {
                object_is_array(attr, register_attr_t);
        } else if (!strcmp(obj->obj_type, "field")) {
                object_is_array(attr, field_attr_t);
        } else if (!strcmp(obj->obj_type, "attribute")) {
                object_is_array(attr, attribute_attr_t);
        } else if (!strcmp(obj->obj_type, "group_attr_t")) {
                object_is_array(attr, group_attr_t);
        } else if (!strcmp(obj->obj_type, "connect")) {
                object_is_array(attr, connect_attr_t);
        }

        return 0;
}

static int check_array_symbol(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	void* attr = symbol->attr;
	switch (symbol->type) {
		case REGISTER_TYPE:
			object_is_array(attr, register_attr_t);
		break;
		case FIELD_TYPE:
			object_is_array(attr, field_attr_t);
		break;
		case ATTRIBUTE_TYPE:
			object_is_array(attr, attribute_attr_t);
		break;
		case GROUP_TYPE:
			object_is_array(attr, group_attr_t);
		break;
		case CONNECT_TYPE:
			object_is_array(attr, connect_attr_t);
		break;
		case  ARRAY_T:
			return 1;
		break;
	        case OBJECT_TYPE:
                        return object_array(symbol);
		default:
			fprintf(stderr, "symbol'%s' not array\n", symbol->name);
			exit(-1);
	}

	return 0;
}

extern void check_reg_table(void);
expr_t* check_refer(symtab_t table, reference_t* ref, expr_t* expr) {
	assert(table != NULL); assert(ref != NULL); assert(expr != NULL);

	symbol_t symbol = NULL;
	symbol_t ref_symbol = NULL;
	symtab_t ref_table = NULL;
	reference_t* tmp = ref;

	if (table->no_check == 1) {
		expr->type->common.categ = INT_T;
		return expr;
	}

	symbol = symbol_find_notype(table, tmp->name);
	//symbol = (symbol == NULL) ? get_symbol_from_root_table(tmp->name, 0) : symbol;
	while (tmp->next) {
		if ((symbol->type == INTERFACE_TYPE) ||
			(symbol->type == OBJECT_TYPE && !strcmp(((object_t*)(symbol->attr))->obj_type, "interface"))) {
			tmp->name = get_interface_name(symbol->name);
			//symbol = get_symbol_from_root_table(tmp->name, 0);
			symbol = symbol_find_notype(table, tmp->name);
		}
		if (symbol != NULL) {
			if (symbol->type == TYPEDEF_T) {
				cdecl_t* type = (cdecl_t*)(symbol->attr);
				if (record_type(type)) {
					switch (type->common.categ) {
						case STRUCT_T:
							ref_table = type->struc.table;
							break;
						case LAYOUT_T:
							ref_table = type->layout.table;
							break;
						default:
							ref_table = type->bitfields.table;
							break;
					}
				}
				else  {
					ref_table = NULL;
				}
			}
			else {
				ref_table = get_symbol_table(symbol);
			}
			if (tmp->is_array) {
				check_array_symbol(table, symbol);
			}
		}
		else {
			fprintf(stderr, "Undefined reference------ '%s'\n", tmp->name);
			exit(-1);
			break;
		}

		if (ref_table == NULL) {
			fprintf(stderr, "'%s' is not struct or dml object\n", tmp->name);
			exit(-1);
			break;
		}

		const char* obj_type = NULL;
		if (symbol->type == OBJECT_TYPE) {
			obj_type = ((object_t*)(symbol->attr))->obj_type;
		}
		ref_symbol = symbol_find_notype(ref_table, tmp->next->name);
		if ((symbol->type == CONNECT_TYPE && !strcmp(tmp->next->name, "obj")) ||
			((symbol->type == OBJECT_TYPE) && !(strcmp(obj_type, "connect")) && (!strcmp(tmp->next->name, "obj")))) {
			cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			type->common.size = sizeof(int) * 8;
			expr->type = type;
			tmp = tmp->next;
			if (tmp->next) {
				fprintf(stderr, "Undefined reference '%s' Line: %d\n", tmp->next->name, __LINE__);
				exit(-1);
			}
			return expr;
		}
		else if (ref_symbol == NULL) {
			fprintf(stderr, "Undefined reference '%s' Line : %d\n", tmp->next->name, __LINE__);
			exit(-1);
			break;
		}

		if (ref_symbol->type >= BOOL_T && ref_symbol->type <= FUNCTION_T) {
			expr->type = ref_symbol->attr;
		}
		else {
			cdecl_t* decl = get_common_type(ref_table, ref_symbol, expr);
			expr->type = decl;
		}

		table = ref_table; symbol = ref_symbol;
		tmp = tmp->next;
	}
	if (expr->type->common.categ == INTERFACE_TYPE) {
		error("the last elment about compent can not be interface\n");
	}
	if (tmp->is_array) {
		check_array_symbol(table, ref_symbol);
	}
	return expr;
}

expr_t* check_component_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	reference_t* reference = (reference_t*)gdml_zmalloc(sizeof(reference_t));
	if (node->component.type == COMPONENT_POINTER_TYPE) {
		reference->is_pointer = 1;
	}
	tree_t* ident = node->component.ident;
	reference->name = ident->ident.str;
	reference = get_reference(node->component.expr, reference, expr, table);
	print_reference(reference);
	expr = check_refer(table, reference, expr);
	expr->node = node;

	return expr;
}

expr_t* check_sizeoftype_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	cdecl_t* type = parse_typeoparg(node, table);

	if (record_type(type) == 0) {
		fprintf(stderr, "sizeoftype not record type\n");
		exit(-1);
	}
	/* TODO: should free the type */
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;

	return expr;
}

expr_t* check_new_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	fprintf(stderr, "Pay attention: not implement the new expression\n");
	exit(-1);

	expr->node = node;
	return expr;
}

void check_func_param(tree_t* node, symtab_t table, signature_t* sig, int param_start) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = NULL;
	vector_t* vect = sig->params;
	param_t* param = NULL;

	int i = param_start;
	while (node) {
		param = (param_t*)(vect->data[i]);
		if (param->is_ellipsis) {
			return;
		}
		expr = check_expr(node, table);
		if (both_scalar_type(param->ty, expr->type) ||
				(both_array_type(param->ty, expr->type)) ||
				(is_same_type(param->ty, expr->type)) ||
				(is_parameter_type(expr->type) && is_scalar_type(param->ty))) {
			node = node->common.sibling;
			i++;
		}
		else {
			error("incompatible type for argument\n");
			break;
		}
	}

	return;
}

static int first_param_is_obj(signature_t* sig) {
	if (sig == NULL)
		return 0;
	vector_t* vect = sig->params;
	param_t* param = vect->data[0];
	cdecl_t* type = param->ty;
	if (type->common.categ == POINTER_T) {
		cdecl_t* new_type = type->common.bty;
		if ((new_type->typedef_name) && !strcmp(new_type->typedef_name, "conf_object_t")) {
			return 1;
		}
	}
	return 0;
}

expr_t* check_function_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr_t* func = check_expr(node->expr_brack.expr, table);
	if (func->is_const) {
		error("The expression is wrong\n");
	}
	signature_t* sig = NULL;
	int argc = 0;
	int param_start = 0;
	if (func->type->common.categ == FUNCTION_T) {
		sig = func->type->function.sig;
		expr->type = func->type->common.bty;
		argc = sig->params->len;
	}
	else if (func->type->common.categ == POINTER_T) {
		sig = func->type->common.bty->function.sig;
		cdecl_t* bty = func->type->common.bty;
		expr->type = bty->common.bty;
		argc = sig->params->len;
		if (func->is_obj && first_param_is_obj(sig)) {
			argc -= 1;
			param_start = 1;
		}
	}
	int arg_num = get_param_num(node->expr_brack.expr_in_brack);
	DEBUG_EXPR("argc: %d, argnum: %d\n", arg_num, argc);
	if (arg_num != argc) {
		fprintf(stderr, "error: too %s arguments to function\n",
				(arg_num < argc) ? "few": "many");
		/* FIXME: handle the error */
		exit(-1);
	}
	else {
		if (arg_num != 0)
			check_func_param(node->expr_brack.expr_in_brack, table, sig, param_start);
	}

	return expr;
}

expr_t* check_brack_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	/* in simics, it support cast like this (void**)(a)
	 * if one expreesion like expression (expression_list),
	 * it will be function call or method call*/
	if (node->expr_brack.expr) {
		expr = check_function_expr(node, table, expr);
	}/* expression (expression_list) */
	else {
		expr = check_expression(node->expr_brack.expr_in_brack, table, expr);
	} /* (expression) */

	expr->node = node;
	return expr;
}

expr_t* check_array_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	fprintf(stderr, "Pay attention: not implement the check array expression\n");
	exit(-1);

	expr->node = node;
	return expr;
}

expr_t* check_bit_slic_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	/* Bit Slicing Expressions : expr[e1:e2, bitorder]
	 *							 expr[e1, bitorder]
	 * expr is of integer type
	 * Both e1 and e2 must be integers
	 */
	expr = check_expression(node->bit_slic.expr, table, expr);
	expr_t* e1 = check_expr(node->bit_slic.bit, table);
	expr_t* e2 = node->bit_slic.bit_end ? check_expr(node->bit_slic.bit_end, table) : NULL;
	if ((!is_int_type(expr->type)) && !is_int_type(e1->type) &&
			!is_no_type(expr->type) && !is_no_type(e1->type)) {
		fprintf(stderr, "the bit slicing must be int type\n");
		exit(-1);
	}
	if (e2 && (!is_int_type(e2->type) && !is_no_type(e2->type))) {
		fprintf(stderr, "the bit slicing must be int type\n");
		exit(-1);
	}

	tree_t* endian = node->bit_slic.endian;
	if (endian) {
		const char* str = endian->ident.str;
		if ((strcmp(str, "le") != 0) || (strcmp(str, "be") != 0)) {
			error("unknow bitorde: %s\n", str);
		}
	}

	DEBUG_EXPR("check bit slic, expr type: %d, type bty: 0x%p\n",
		expr->type->common.categ, expr->type->common.bty);
	expr->node = node;
	return expr;
}

expr_t* check_expression(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	switch (node->common.type) {
		case EXPR_ASSIGN_TYPE:
			expr = check_assign_expr(node, table, expr);
			break;
		case TERNARY_TYPE:
			expr = check_ternary_expr(node, table, expr);
			break;
		case BINARY_TYPE:
			expr = check_binary_expr(node, table, expr);
			break;
		case CAST_TYPE:
			expr = check_cast_expr(node, table, expr);
			break;
		case SIZEOF_TYPE:
			expr = check_sizeof_expr(node, table, expr);
			break;
		case UNARY_TYPE:
			expr = check_unary_expr(node, table, expr);
			break;
		case INTEGER_TYPE:
			expr = check_const_expr(node, expr);
			break;
		case FLOAT_TYPE:
			expr = check_const_expr(node, expr);
			break;
		case CONST_STRING_TYPE:
			expr = check_const_expr(node, expr);
			break;
		case UNDEFINED_TYPE:
			expr = check_const_expr(node, expr);
			break;
		case QUOTE_TYPE:
			expr = check_quote_expr(node, table, expr);
			break;
		case IDENT_TYPE:
		case DML_KEYWORD_TYPE:
			expr = check_ident_expr(node, table, expr);
			break;
		case COMPONENT_TYPE:
			expr = check_component_expr(node, table, expr);
			break;
		case SIZEOFTYPE_TYPE:
			expr = check_sizeoftype_expr(node, table, expr);
			break;
		case NEW_TYPE:
			expr = check_new_expr(node, table, expr);
			break;
		case EXPR_BRACK_TYPE:
			expr = check_brack_expr(node, table, expr);
			break;
		case ARRAY_TYPE:
			expr = check_array_expr(node, table, expr);
			break;
		case BIT_SLIC_EXPR_TYPE:
			expr = check_bit_slic_expr(node, table, expr);
			break;
		default:
			error("there may be other type expression: %s\n", node->common.name);
			/* FIXME: Pay attention: The exit function is only for debugging */
//			exit(-1);
			break;
	}
	return expr;
}

expr_t* check_expr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = (expr_t*)gdml_zmalloc(sizeof(expr_t));
	check_expression(node, table, expr);

	return expr;
}

cdecl_t* get_typeof_type(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = check_expr(node->typeof_tree.expr, table);

	return expr->type;
}

void check_comma_expr(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return ;
	}
	assert(table != NULL);

	tree_t* expr_node = node;
	while (node != NULL) {
		DEBUG_EXPR("In %s, line = %d, node->type: %s\n",
				__func__, __LINE__, node->common.name);
		check_expr(node, table);
		node = node->common.sibling;
	}
	node = expr_node;

	return ;
}
