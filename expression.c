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
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				/* FIXME: handle the warning */
				return type2;
			}
			else if ((type1 == NO_TYPE) && (type2 == CONST_STRING_TYPE)) {
				return type2;
			}
			else {
				fprintf(stderr, "Line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				/* FIXME: handle the warning */
				return type2;
			}
			else {
				fprintf(stderr, "line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
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
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				return type2;
			}
			else {
				fprintf(stderr, "line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				return type2;
			}
			else {
				fprintf(stderr, "line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
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
				fprintf(stderr, "Line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
				return -1;
			}
			break;
		case POINTER_TYPE:
			if ((type2 == CHAR_TYPE) || (type2 == INT_TYPE) || (type2 == UNDEFINED_TYPE)
					|| (type2 == INTEGER_TYPE) || (type2 == LONG_TYPE)
					|| (type2 == SHORT_TYPE) || (type2 == SIGNED_TYPE)
					|| (type2 == UNSIGNED_TYPE) || (type2 == BOOL_TYPE)
					|| (type2 == POINTER_TYPE) || (type2 == NO_TYPE)) {
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				return type1;
			}
			else {
				fprintf(stderr, "Line: %d, error: incompatible types when assigning to type: %d\n",
						__LINE__, type2);
				/* FIXME: handle the error */
				exit(-1);
				return -1;
			}
			break;
		case CONST_STRING_TYPE:
			if (type2 == CONST_STRING_TYPE) {
				return type2;
			}
			else if (type2 == POINTER_TYPE) {
				fprintf(stderr, "warning: assignment makes integer from pointer without a cast\n");
				return type2;
			}
			else if ((type2 == NO_TYPE) || (type2 == UNDEFINED_TYPE)) {
				return type1;
			}
			else {
				fprintf(stderr, "Line: %d, error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME: handle the error */
				exit(-1);
				return -1;
			}
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
				fprintf(stderr, "Line: %d, error: incompatible types when assigning to type %d\n", __LINE__, type2);
				/* FIXME: handle the error */
				exit(-1);
				return -1;
			}
		default:
			/* TODO: the UNDEFINED_TYPE */
			fprintf(stderr, "In %s, line = %d, other type: %d\n", __func__, __LINE__, type1);
			/*FIXME: handle the error */
			exit(-1);
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

	left_expr = cal_expression(&((*node)->expr_assign.left), table, left_expr);

	tree_t* left_node = (*node)->expr_assign.left;
	if (left_expr->is_const) {
		fprintf(stderr, "error: value required as left operand of assignment\n");
		/* FIXME: should handle the error */
		exit(-1);
	}

	if ((left_expr->is_undeclare) || (right_expr->is_undeclare)) {
		return ;
	}

	/* FIXME: there is some problems */
	int type = -1;
	if (left_node->common.type == IDENT_TYPE) {
		symbol_t symbol = symbol_find_notype(table, left_node->ident.str);
		if (symbol == NULL) {
			if (table->no_check) {
				DEBUG_TEMPLATE_SYMBOL("line : %d warning: %s no undeclared in template\n", __LINE__, left_node->ident.str);
				symbol_insert(table, left_node->ident.str, right_expr->final_type, NULL);
			}
		}
		else if (symbol->type == PARAM_TYPE) {
			params_t* param = (params_t*)(symbol->attr);
			if (param->is_notype) {
				set_decl_type(param->decl->type, right_expr->final_type);
				param->decl->value = right_expr;
			}
			else {
				decl_t* decl = (decl_t*)(param->decl);
				int type = get_decl_type(decl);
				type = charge_type(type, right_expr->final_type);
				if (type < 0) {
					/* FIXME: handle  the error */
					exit(-1);
				}
				set_decl_type(decl->type, type);
				decl->value = right_expr;
			}
			DEBUG_EXPR("In %s, line = %d, right_expr is_notype : %d\n", __func__, __LINE__, right_expr->final_type);
		}
		else if (symbol->type == IDENT_TYPE) {
			decl_t* decl = (decl_t*)(symbol->attr);
			int type = get_decl_type(decl);
			type = charge_type(type, right_expr->final_type);
			if (type < 0) {
				/* FIXME: handle  the error */
				exit(-1);
			}
			set_decl_type(decl->type, type);
			decl->value = right_expr;
		}
		else if (symbol->type == PARAMETER_TYPE)  {
			parameter_attr_t* parameter = (parameter_attr_t*)(symbol->attr);
			paramspec_t* spec = parameter->spec;
			int type = charge_type(spec->type, right_expr->final_type);
			if (type < 0) {
				fprintf(stderr, "Line : %d error: incompatible types when assigning to type\n", __LINE__);
				/* FIXME : handle the error */
				exit(-1);
			}
			spec->type = type;
			spec->expr = right_expr;
		}
		else if (((symbol->type) >= BITFIELDS_TYPE) &&
				((symbol->type) <= STRUCT_TYPE)){
			int type = charge_type(symbol->type, right_expr->final_type);
			if (type > 0) {
				decl_t* decl = (decl_t*)(symbol->attr);
				decl->value = right_expr;
				right_expr->final_type = type;
			}
		}
		else {
			DEBUG_EXPR("In %s, line = %d, other type: %d\n",
					__func__, __LINE__, symbol->type);
			/* FIXME: handle the error */
			exit(-1);
		}
	}
	else if (left_node->common.type == QUOTE_TYPE) {
		tree_t* node = left_node->quote.ident;
		if (charge_node_is_const(node)) {
			fprintf(stderr, "the quote can not the const!\n");
			/* TODO: hann  the error */
			exit(-1);
		}
		else if (strcmp(node->ident.str, "this") == 0) {
			/* do nothing */
		}
		else {
			get_ident_value(&(left_node->quote.ident), table, left_expr);
			type = charge_left_right_expr_type(left_expr, right_expr);
			right_expr->final_type = type;
		}
	}
	else {
		DEBUG_EXPR("other left node type : %d\n", left_node->common.type);
		type = charge_left_right_expr_type(left_expr, right_expr);
		right_expr->final_type = type;
	}
	right_expr->node = *node;

	return;
}

expression_t* cal_common_assign(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
	DEBUG_EXPR("In %s, line = %d\n", __FUNCTION__, __LINE__);

	expr = cal_expression(&((*node)->expr_assign.right), table, expr);
	expression_t* left_expr = (expression_t*)gdml_zmalloc(sizeof(expression_t));
	cal_assign_left(node, table, left_expr, expr);
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
		fprintf(stderr, " The ternary need expression\n");
		/* FIXME: handle the error */
		exit(-1);
	}

	int true_type, false_type;
	cal_expression(&((*node)->ternary.cond), table, expr);
	cal_expression(&((*node)->ternary.expr_true), table,  expr);
	true_type = expr->final_type;
	cal_expression(&((*node)->ternary.expr_false), table, expr);
	false_type = expr->final_type;
	if (expr->is_undeclare) {
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
			fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", left->int_cst.int_str);
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
		fprintf(stderr, "line: %d, error: invalid operands to binary\n", __LINE__);
		/* FXIME: handle the error */
		exit(-1);
	}

	if (right->common.type == INTEGER_TYPE) {
		if (right->int_cst.out_64bit) {
			fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", left->int_cst.int_str);
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
		fprintf(stderr, "The left(%s) or right(%s) node type is invalid operands\n",
				left->common.name, right->common.name);
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
		char* float_str = (char*)gdml_zmalloc(sizeof(char) * 64);
		sprintf(float_str, "%f", final_value);
		*new_node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst));
		(*new_node)->float_cst.float_str = float_str;
		(*new_node)->float_cst.value = final_value;
		(*new_node)->common.print_node = print_float_literal;
		expr->final_type = final_type;
	}
	if ((final_type == INTEGER_TYPE) || (final_type == BOOL_TYPE)) {
		char* int_str = (char*)gdml_zmalloc(sizeof(char) * 64);
		sprintf(int_str, "%d", final_type);
		*new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
		(*new_node)->int_cst.value = (int)final_value;
		(*new_node)->int_cst.int_str = int_str;
		(*new_node)->common.print_node = print_interger;
		expr->final_type = final_type;
	}

	if ((left->common.sibling) || (left->common.child)
			|| (right->common.sibling) || (right->common.child)) {
		fprintf(stderr, "In %s, the left or right node have sibling or child\n", __FUNCTION__);
		/* FIXME: only for debugging */
		exit(-1);

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
		left = (*node)->binary.left;
		right = (*node)->binary.right;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;
		op_type = (*node)->expr_assign.type;
		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
	}

	if (expr->is_undeclare) {
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
			tree_t* str_node = create_node("string_literal", CONST_STRING_TYPE, sizeof(struct tree_string));
			str_node->string.length = len;
			str_node->string.pointer = str;
			str_node->common.print_node = print_string;
			new_node = str_node;
			expr->final_type = CONST_STRING_TYPE;
		}
		else if ((left->common.type == UNDEFINED_TYPE) &&
				(right->common.type == UNDEFINED_TYPE)){
			new_node = (tree_t*)create_node("undefined", UNDEFINED_TYPE, sizeof(struct tree_common));
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

		left = (*node)->binary.left;
		right = (*node)->binary.right;
		op_type = (*node)->binary.type;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;

		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
		op_type = (*node)->expr_assign.type;
	}
	if (expr->is_undeclare) {
		return expr;
	}

	if (charge_node_is_const(left) && (charge_node_is_const(right))) {
		 tree_t* new_node = NULL;
		if ((left->common.type == CONST_STRING_TYPE) || (left->common.type == UNDEFINED_TYPE)
				|| (right->common.type == CONST_STRING_TYPE) || (right->common.type == UNDEFINED_TYPE)) {
			fprintf(stderr, "The left(%s) or right(%s) node type is wrong!\n",
					left->common.name, right->common.name);
			expr->node = *node;
			/* TODO: handle the error */
			return expr;
		}
		else {
			if (get_left_right_value(left, right, &new_node, expr, op_type) == 0) {
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

		left = (*node)->binary.left;
		right = (*node)->binary.right;
		op_type = (*node)->binary.type;
	}
	else {
		cal_expression(&((*node)->expr_assign.left), table, expr);
		left_type = expr->final_type;
		cal_expression(&((*node)->expr_assign.right), table, expr);
		right_type = expr->final_type;

		left = (*node)->expr_assign.left;
		right = (*node)->expr_assign.right;
		op_type = (*node)->expr_assign.type;
	}

	if (expr->is_undeclare) {
		return expr;
	}

	if (charge_node_is_const(left) && charge_node_is_const(right)) {
		if ((left->common.type == INTEGER_TYPE) &&
				(right->common.type == INTEGER_TYPE)) {
			int left_value, right_value, final_value;
			left_value = right_value = final_value = 0;
			if ((left->int_cst.out_64bit) || (right->int_cst.out_64bit)){
				fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", left->int_cst.int_str);
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
						fprintf(stderr, "The binary operator is other type: %s\n", (*node)->common.name);
						expr -> node = *node;
						return expr;
				}

				char* value_str = (char*)gdml_zmalloc(sizeof(int) * 64);
				sprintf(value_str, "%d", final_value);
				tree_t* new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
				new_node->int_cst.int_str = value_str;
				new_node->int_cst.value = final_value;
				new_node->common.print_node = print_interger;
				new_node->common.sibling = (*node)->common.sibling;
				new_node->common.child = (*node)->common.child;
				if ((left->common.sibling) || (left->common.child)
						|| (right->common.sibling) || (right->common.child)) {
					fprintf(stderr, "The left or right node has sibling or child\n");
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
			fprintf(stderr, "The binary operation's type is wrong, left: %s, right: %s\n",
					left->common.name, right->common.name);
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
			fprintf(stderr, "The binary operation is other type : %s\n", (*node)->common.name);
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
		return expr;
	}

	if (charge_node_is_const(unary_expr)) {
		if (unary_expr->common.type != INTEGER_TYPE) {
			fprintf(stderr, "The bit non operation expression should int\n");
			/* FIXME: should handle the error */
			exit(-1);
		}
		else {
			const int int_length = 64;
			int value = ~(unary_expr->int_cst.value);
			char* str = (char*)gdml_zmalloc(int_length);
			tree_t* new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
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
			fprintf(stderr, "Line: %d, error: invalid operands to binary\n", __LINE__);
			/* TODO: handle the error */
			exit(-1);
		}
		expr->node = *node;
	}

	return expr;
}

expression_t* unary_expr_common(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);

	cal_expression(&((*node)->unary.expr), table, expr);
	tree_t* unary_expr = (*node)->unary.expr;

	if (expr->is_undeclare) {
		return expr;
	}

	if (charge_node_is_const(unary_expr)) {
		if ((unary_expr->common.type == CONST_STRING_TYPE)
				|| (unary_expr->common.type == UNDEFINED_TYPE)) {
			fprintf(stderr, "The unary operation expression can not be the type: %s\n",
					unary_expr->common.name);
			/* TODO: handle the error */
			exit(-1);
		}
		else {
			float value, final_value;
			int final_type;
			value = final_value = final_type = 0;
			if (unary_expr->common.type == INTEGER_TYPE) {
				if (unary_expr->int_cst.out_64bit) {
					fprintf(stderr, "Pay attention: the binary operator have problems: left: %s\n", unary_expr->int_cst.int_str);
					expr->node = *node;
					/* TODO: handle the error */
					exit(-1);
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
				new_node = (tree_t*)create_node("float_literal", FLOAT_TYPE, sizeof(struct tree_float_cst));
				new_node->float_cst.float_str = float_str;
				new_node->float_cst.value = final_value;
				new_node->common.print_node = print_float_literal;
				expr->final_type = final_type;
			}
			if (final_type == INTEGER_TYPE) {
				char* int_str = (char*)gdml_zmalloc(sizeof(char) * 64);
				sprintf(int_str, "%d", final_value);
				new_node = (tree_t*)create_node("integer_literal", INTEGER_TYPE, sizeof(struct tree_int_cst));
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
			if (charge_node_is_const((*node)->unary.expr)) {
				fprintf(stderr, "The unary operation expression can not constant\n");
					/* FIXME: should handle the error */
					exit(-1);
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
			fprintf(stderr, "Wrong unary type: %s\n", (*node)->common.name);
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
		fprintf(stderr, "sizeof type is datatype\n");
		/* should handle the error */
		exit(-1);
	}
	cal_expression(&((*node)->sizeof_tree.expr), table, expr);
	if (expr->is_undeclare) {
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

	if (charge_node_is_const(*node)) {
		fprintf(stderr, "error: lvalue required as unary ‘&’ operand\n");
		/* TODO: handle the error */
		exit(-1);
	}

	tree_t* ident = (*node)->quote.ident;
	/* FIXME: this may something wrong */
	if (strcmp(ident->ident.str, "this") == 0) {
		expr->node = *node;
		expr->final_type = NO_TYPE;
		return expr;
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

int get_method_param_type(symbol_t symbol) {
	assert(symbol != NULL);

	params_t* param = (params_t*)(symbol->attr);
	 if (param->is_notype) {
		 return NO_TYPE;
	 }
	 else {
		 return get_decl_type(param->decl);
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


	if (symbol == NULL) {
		fprintf(stderr, "%s not typdef\n", name);
		/* TODO: handle the error */
		exit(-1);
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

	char* str = NULL;
	int type = 0;
	parameter_attr_t* attr = symbol->attr;
	paramspec_t* spec = attr->spec;

	if (spec->type != CONST_STRING_TYPE) {
		fprintf(stderr, "error: allocate_type of attribute should be string\n");
		/* TODO: handle the error */
		exit(-1);
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
		fprintf(stderr, "Warning: other attribute type: %s\n", str);
		/* TODO: handle the error, this is only for debugging */
		exit(-1);
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
		fprintf(stderr, "The attribute has no type\n");
		/* TODO: The attribute is ok with no type? */
		exit(-1);
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
			type = get_method_param_type(symbol);
			DEBUG_EXPR("IN %s, line = %d, param type: %d\n",
					__func__, __LINE__, type);
			break;
		case PARAMETER_TYPE:
			type = get_parameter_type(symbol);
			DEBUG_EXPR("IN %s, line = %d, parameter type: %d\n",
					__func__, __LINE__, type);
			break;
		case CONSTANT_TYPE:
			type = get_constant_type(symbol);
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
			fprintf(stderr, "In %s, line = %d, other dml %s(%d)\n",
					__FUNCTION__, __LINE__, symbol->name, symbol->type);
			/* FIXME: only for debugging */
			exit(-1);
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

	tree_t* new_node = (tree_t*)create_node("bool", BOOL_TYPE, sizeof(struct tree_int_cst));
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
		if (pre_symbol->type == AUTO_TYPE) {
			expr->final_type = NO_TYPE;
		}
		else {
			expr->final_type = pre_symbol->type;
		}
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
			/* insert the undeclared variable to table list */
			//fprintf(stderr, "%s no undeclared (first use)\n", (*node)->ident.str);
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
			fprintf(stderr, "line: %d, other node type: %s\n", __LINE__, expr_node->common.name);
			/* FIXME handle the error */
			exit(-1);
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
			fprintf(stderr, "other node type: %s\n", node->common.name);
			/* FIXME: handle the error */
			exit(-1);
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
		default:
			fprintf(stderr, "Othe symbol %s(%d)\n", symbol->name, symbol->type);
			/* FIXME: handle the error */
			exit(-1);
	}

	return table;
}

symtab_t get_default_symbol_tale(symtab_t table, const char* name) {
	assert(name != NULL);
	symtab_t tmp_table = NULL;
	symbol_t symbol = NULL;
	void* attr = NULL;

	if (strcmp(name, "dev") == 0) {
		symbol = symbol_find(table, "device", TEMPLATE_TYPE);
		attr = symbol->attr;
		table = ((template_attr_t*)attr)->table;
	} // device
	else {
		fprintf(stderr, "Othe default name: %s\n", name);
		/* FIXME: handle the error */
		exit(-1);
	}

	return table;
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
		if (symbol == NULL) {
			/* some default symbol */
			ref_table = get_default_symbol_tale(table, tmp->name);
		}
		else {
			ref_table = get_symbol_table(symbol);
		}

		if (ref_table == NULL) {
			fprintf(stderr, "%s not object or struct!\n", symbol->name);
			/* FIXME: handle the error */
			exit(-1);
		}
		ref_symbol = symbol_find_notype(ref_table, tmp->next->name);
		if (ref_symbol == NULL) {
			fprintf(stderr, "%s not element of %s\n", tmp->next->name, tmp->name);
			/* FIXME: handle the error */
			exit(-1);
		}

		/* get the type about symbol */
		if (is_c_type(ref_symbol->type) == 1) {
			type = ref_symbol->type;
		}
		else {
			type = get_c_type(ref_table, ref_symbol);
		}

		if (type == INTERFACE_TYPE) {
			char* name = (char*)gdml_zmalloc(name_len);
			strcpy(name, tmp->next->name);
			strcat(name, "_interface_t");
			tmp->next->name = name;
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
	tree_t* tmp_node = (*node);
	func_param_t* param = func->param;

	expression_t* expr = (expression_t*)gdml_zmalloc(sizeof(expression_t));
	int type = 0;
	int i = 0;
	while (tmp_node) {
		expr = cal_expression(&tmp_node, table, expr);
		type = get_decl_type(param->decl);
		if (type == TYPEDEF_TYPE) {
			type = get_typedef_type(table, param->decl->type->typedef_name);
		}
		charge_type(expr->final_type, type);
		tmp_node = tmp_node->common.sibling;
		param = param->next;
	}

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

			expr->final_type = get_func_ret_type(table, func->ret_decl);
		}
	}

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

	cal_expression(&((*node)->bit_slic.expr), table, expr);
	if (expr->is_const) {
		fprintf(stderr, "The bit slic declare should not const!\n");
		exit(-1);
	}
	cal_expression(&((*node)->bit_slic.bit), table, expr);
	cal_expression(&((*node)->bit_slic.bit_end), table, expr);

	expr->is_const = 0;
	if (expr->is_undeclare) {
		return expr;
	}

	tree_t* endian = (*node)->bit_slic.endian;
	if (endian) {
		if ((strcmp(endian->ident.str, "le") != 0)
				|| (strcmp(endian->ident.str, "be") != 0)) {
			fprintf(stderr, "unknow bitorde: %s\n", endian->ident.str);
			/* TODO: handle the error */
			exit(-1);
		}
	}
	expr->node = *node;

	return expr;
}

expression_t* cal_expression(tree_t** node, symtab_t table, expression_t* expr) {
	assert(*node != NULL);
	assert(table != NULL);
	assert(expr != NULL);
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
			printf("Pay attention: ther may be other type expression: %s\n", (*node)->common.name);
			/* FIXME: Pay attention: The exit function is only for debugging */
			exit(-1);
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

	parse_expression(node, table);
	tree_t* arg_node = *node;
	tree_t* pre_node = *node;

	parse_expression(node, table);

	while ((arg_node->common.sibling) != NULL) {
		arg_node = arg_node->common.sibling;
		parse_expression(&arg_node, table);
		if (arg_node != (pre_node->common.sibling)) {
			pre_node->common.sibling = arg_node;
		}
		pre_node = arg_node;
	}

	return;
}
