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
	/*special case field */
	if(expr_is_bit_slic(node)) {
		translate_bit_slic_assign(t);
		return;
	}
	/*need more special cases, such as layout etc*/
	translate(node);
	D(" %s ",t->expr_assign.assign_symbol);
	node = t->expr_assign.right;
	translate(node);
}

/*
void translate_cond_expr(tree_t *t) {
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

void translate_cast_expr(tree_t *t) {
        tree_t *node;
                
        D("((");
        node = t->cast.ctype;
        translate(node);
        D(")"); 
        node = t->cast.expr;
        translate(node);
        D(")");
}
void translate_binop_expr(tree_t *t) {
	tree_t *node;

	node = t->binary.left;
	D("(");
	translate(node);
	D(" %s ",t->binary.operat);
	node = t->binary.right;
	translate(node);
	D(")");
}

void translate_bit_slic(tree_t *t) {
	tree_t *node;

	node = t->bit_slic.expr;
	translate(node);
	D(" >> ");
	node = t->bit_slic.bit_end;
	translate(node);
	D(" & ");
	D("MASK1(");
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
} */

void translate_unary_expr(tree_t *t) {
	tree_t *node;

	D("%s",t->unary.operat);
	if(!strcmp(t->unary.operat, "~")) {
	}

	node = t->unary.expr;
	translate(node);
}

void translate_pre_expr(tree_t *t) {
	const char *op;

	op = t->unary.operat;
	if(!strcmp(op, "pre_dec")) {
		D("--");
	} else {
		D("++");
	}
	translate(t->unary.expr);
}


void translate_post_expr(tree_t *t) {
	const char *op;

	op = t->unary.operat;
	translate(t->unary.expr);
	if(!strcmp(op, "aft_inc")) {
		D("++");
	} else {
		D("--");
	}
}

/*
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
	const char *name;
	const char *ref_name;
	object_t *obj;
	symbol_t sym;
	ref_ret_t ref_ret;
	object_t *tmp;

	node = t->expr_brack.expr;
	sym = get_ref_sym(node, &ref_ret, NULL);
	if(sym && sym->type == OBJECT_TYPE) {
		obj = (object_t *)sym->attr;
		printf("obj %s\n", obj->name);
		if(!strcmp(obj->obj_type, "interface")) {
			if(!ref_ret.con) {
				tmp = obj->parent;
				ref_name = get_obj_ref(tmp);
				D("%s", ref_name);
				if(tmp->is_array) {
						D("[_idx0]");
				}
				D(".%s", ref_ret.iface->name);
				D("->%s", ref_ret.method);
				D("(");
				D("%s", ref_name);
				if(tmp->is_array) {
						D("[_idx0]");
				}
				//D(".obj", ref_ret.iface->name);
				D(".obj");
			} else {
				printf_ref(&ref_ret);
				D(".%s->%s",ref_ret.iface->name, ref_ret.method);
				D("(");
				//ref_name = get_obj_ref(tmp);
				printf_ref(&ref_ret);
				//D(".obj",ref_ret.iface->name);
				D(".obj");
			}
			node = t->expr_brack.expr_in_brack;
			if(node) {
				D(", ");
				translate_expr_list(node, NULL);
			}
			D(")");
		}else {
			my_DBG("object: other type %s", obj->obj_type);
		}
		
	} else {
		translate(node);
		D("(");
		node = t->expr_brack.expr_in_brack;
		translate_expr_list(node, NULL);
		D(")");
	}
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
} */

void translate_bit_slice2(tree_t *t) {
	tree_t *expr;
	tree_t *index;

	expr = t->bit_slic.expr;
	index = t->bit_slic.bit;
	if(!t->bit_slic.endian) {
		translate(expr);
		D("[");
		translate(index);
		D("]");
	}
}

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
	D("MASK1(");
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
