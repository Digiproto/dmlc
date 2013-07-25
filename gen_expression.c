/*
 * gen_expression.c: 
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
#include "gen_expression.h" 
static int expr_is_bit_slic(tree_t *t);
void translate_assign(tree_t *t) {
	tree_t *node;

	node = t->expr_assign.left;
	if(expr_is_bit_slic(node)) {
		translate_bit_slic_assign(t);
		return;
	}
	translate(node);
	D(" %s ",t->expr_assign.assign_symbol);
	node = t->expr_assign.right;
	translate(node);
}
/*
static void translate_cond_expr(tree_t *t) {
	tree_t *node;

	node = t->ternay.cond;
	translate(node);
	D(" ? ");
	node = t->ternay.expr_true;
	translate(node);
	D(" : ");
	node = t->ternay.expr_false;
	translate(node);
}
*/
void translate_binop_expr(tree_t *t) {
	tree_t *node;

	node = t->binary.left;
	translate(node);
	D(" %s ",t->binary.operat);
	node = t->binary.right;
	translate(node);
}

void translate_bit_slic(tree_t *t) {
	tree_t *node;

	node = t->bit_slic.expr;
	translate(node);
	D(" >> ");
	node = t->bit_slic.bit_end;
	translate(node);
	D(" & ");
	D("MASK(");
	node = t->bit_slic.bit;
	translate(node);
	D(" - ");
	node = t->bit_slic.bit_end;
	translate(node);
	D(")");	
}
/*
static void translate_cast(tree_t *t) {
	tree_t *node;

	node = t->cast.ctype;
	D("(");
	translate(node);
	node = t->cast.expr;
	translate(node);
	D(")");
}

static void translate_sizeof(tree_t *t) {
	tree_t *node;
	
	node = t->sizeof_tree.expr;
	D("sizeof ");
	translate(node);
}

static void translate_unary_expr(tree_t *t) {
	tree_t *node;

	D("%s",t->unary.operat);
	node = t->unary.expr;
	translate(node);
}

static void translate_defined(tree_t *t) {
	tree_t *node;

	node = t->unary.expr;
}

static void translate_pre_inc(tree_t *t) {
	tree_t *node;

	node = t->unary.expr;
	D("++");
	translate(node);
}

static void translate_pre_dec(tree_t *t) {
	tree_t *node;

	node = t->unary.expr;
	D("--");
	translate(node);
}

static void translate_post_inc(tree_t *t) {
	tree_t *node;

	node = t->unary.expr;
	translate(node);
	D("++");
}

static void translate_post_dec(tree_t *t) {
	tree_t *node;

	node = t->unary.expr;
	translate(node);
	D("--");
}

static void translate_integer(tree_t *t) {
	const char *value;

	value = t->int_cst.int_str;
	D("%s",value);
}

static void translate_float(tree_t *t) {
	const char *value;
	
	value = t->float_cst.float_str;
	D("%s",value);
}

static void translate_string(tree_t *t) {
	const char *value;
	
	value = t->string.pointer;
	D("\"%s\"",value);
}
*/
void translate_expr_brack_direct(tree_t *t) {
	tree_t *node;

	node = t->expr_brack.expr;
	translate(node);
	
	D("(");
	node = t->expr_brack.expr_in_brack;
	translate_expr_list(node, NULL);
	D(")");
}

static int expr_is_bit_slic(tree_t *t) {

	if(t && t->common.type == BIT_SLIC_EXPR_TYPE) {
		if(t->bit_slic.bit && t->bit_slic.bit_end) {
			return 1;
		}
	}
	return 0;
}

/*
static int expr_is_array_ref(tree_t *t) {
	const type_t *type;

	if(t && t->common.type == BIT_SLIC_EXPR_TYPE) {
		if(t->bit_slic.bit && !t->bit_slic.bit_end) {
			type = expr_type(t->bit_slic.expr);
			if(type->kind == ARRAY_TYPE) {
				return 1;
			}
		}
	}
	return 0;
}

static void translate_integer(tree_t *t) {
	tree_t *node = t;
	D("%s",node->int_cst.int_str);
}

static void translate_string(tree_t *t) {
	tree_t *node = t;
	D("%s",node->string.pointer);
}

static void translate_float(tree_t *t) {
	tree_t *node = t;
	D("%s",node->float_cst.float_str);
}

static void translate_bit_slice(tree_t *t) {
	tree_t *node;
	tree_t *index;
	int start_index;
	int end_index = -1;
	int tmp;
	int single_bit = 0;
	tree_t *start_expr;
	tree_t *end_expr;
	int const_index = 0;
	int pos;
	int mask;
	const char *type;
	int is_array;
	expression_t *expr;


	node = t->bit_slic.expr;
	is_array = !t->bit_slic.bit_end && !t->bit_slic.endian;
	if(!is_array) {
		if(t->bit_slic.bit && t->bit_slic.bit_end) {
			expr = parse_expression(t->bit_slic.bit, current_table);
			if(expr->is_const) {
				start_index = expr->final_value;
				const_index++;
			} else {
				start_expr = t->bit_slic.bit;
			}
			expr = parse_expression(t->bit_slic.bit_end, current_table);
			if(expr->is_const) {
				const_index++;
				end_index = expr->final_value;
			} else {
				end_expr = t->bit_slic.bit_end; 
			}
			if(const_index == 2) {
				goto const_bit;
			}
		} else if (t->bit_slic.bit && !t->bit_slic.bit_end) {
			expr = parse_expression(t->bit_slic.bit, current_table);
			if(expr->is_const) {
				start_index = expr->final_value;
				const_index++;
				goto const_bit;
			} else {
				single_bit = 1;
		    	start_expr = t->bit_slic.bit;	
			}
		}
	} else {
		index = t->bit_slic.bit;	
	}

	if(!is_array) {
		if(single_bit) {
			D("(");
			D("(%s)",type);
			D("(");
			translate(node);
			D(" >> ")
			translate(start_expr);
			D(" & 1");
			D(")");
			D(")");
			return;
		}
		if(end_index == 0) {
			D("(");
			D("(%s)",type);
			D("(");
			translate(node);
			D(" & ");
			D("MASK")
			D("(");
			translate(start_expr);
			D(")");
			D(")");
			D(")");
			return;
		} else if (end_index > 0) {
			D("(");
			D("(%s)",type);
			D("(");
			translate(node);
			D(" >> %d",end_index);
			D(" & ");
			D("MASK");
			D("(");
			translate(start_expr);
			D(" - ");
			D("%d", end_index);
			D(")");
			D(")");
			D(")");
			return;
		} else {
			D("(");
			D("(%s)",type);
			D("(");
			translate(node);
			D(" >> ");
			translate(end_expr);
			D(" & ");
			D("MASK");
			D("(");
			translate(start_expr);
			D(" - ");
			translate(end_expr);
			D(")");
			D(")");
			D(")");
			return;

		}
	} else {
		translate(node);
		D("[");
		transalte(index);
		D("]");
		return;
	}
const_bit:
	if(const_index == 2) {
		tmp = start_index - end_index;
		tmp = 1 << (tmp + 1);
		mask = tmp - 1;
	} else {
		mask = 1;
	}
	if(end_index == 0) {
		D("(");
		D("(%s)",type);
		D("(");
		translate(node);
		D(" & %d",mask);
		D(")");
		D(")");
	} else {
		D("(");
		D("(%s)",type);
		D("(");
		translate(node);
		D(" >> %d",end_index);
		D(" & %d",mask);
		D(")");
		D(")");
	}
}*/

void translate_bit_slic_assign(tree_t *t) {
	tree_t *sexpr;
	tree_t *eexpr = NULL;
	tree_t *node;
	tree_t *right;
	tree_t *expr;
	
	node = t->expr_assign.left;
	right = t->expr_assign.right;
	sexpr = node->bit_slic.bit;
	if(node->bit_slic.bit_end) {
		eexpr = node->bit_slic.bit_end;
	}
	expr = node->bit_slic.expr;
	translate(expr);
	D(" = ");
	D("MIX(");
	translate(expr);
	D(", ");
	D("(uint64 )");
	translate(right);
	D(" << ");
	translate(eexpr);
	D(", ");
	D("(uint64 )");
	D("MASK(");
	translate(sexpr);
	D(" - ");
	translate(eexpr);
	D(")");
	D(" << ");
	translate(eexpr);
	D(")");
}

void translate_brack_expr(tree_t *t) {
	tree_t *node = t;
	tree_t *expr = node->expr_brack.expr_in_brack;

	D("(");
	translate(expr);
	D(")");
}
