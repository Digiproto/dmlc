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
#include "node_type.h"
extern symtab_t current_table;
static int expr_is_bit_slic(tree_t *t);
static int expr_is_layout(tree_t *);
static void translate_layout_assign(tree_t *t, type_info_t *info);
static void translate_bit_slic_assign(tree_t *t, type_info_t *info);
static void translate_layout_bitslic_assign(tree_t *t, type_info_t *info);
	

/**
 * @brief translate_assign : translate the assign expression
 *
 * @param t : the syntax tree node
 */
void translate_assign(tree_t *t) {
	tree_t *node;

	node = t->expr_assign.left;
	/*special case field */
	type_info_t *info = NULL;
	check_expr_type(node, current_table, &info);
	if(info){
		enum type_info kind = info->kind;
		switch(kind) {
			case TYPE_BITFIELD:
				translate_bit_slic_assign(t, info);
				break;
			case TYPE_LAYOUT:
				translate_layout_assign(t, info);
				break;
			case TYPE_BITFIELD | TYPE_LAYOUT:
				translate_layout_bitslic_assign(t, info);
			default:
				fprintf(stderr, "unkown type kind %d\n", kind);
		}
		return;
	}
	fprintf(stderr, "node %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
	translate(node);
	D(" %s ",t->expr_assign.assign_symbol);
	node = t->expr_assign.right;
	translate(node);
}

void translate_cond_expr(tree_t *t) {
   tree_t *node;

   node = t->ternary.cond;
   translate(node);
   D(" ? ");
   node = t->ternary.expr_true;
   translate(node);
   D(" : ");
   node = t->ternary.expr_false;
   translate(node);
}

/**
 * @brief translate_cast_expr : translate the cast expression
 *
 * @param t : syntax tree node of cast
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
extern int in_sizeof;
/**
 * @brief translate_binop_expr : translate the binary operation expression
 *
 * @param t : the syntax tree node of expression
 */
void translate_binop_expr(tree_t *t) {
	tree_t *node;

	node = t->binary.left;
	if(t->binary.type == LEFT_OP_TYPE) {
		tree_t *shift = t->binary.right;
		if(shift->common.type == INTEGER_TYPE && shift->int_cst.value >= 32) {

			D("(");
			D("(unsigned long long)");
			translate(node);
			D(" %s ",t->binary.operat);
			node = t->binary.right;
			translate(node);
			D(")");
			return;
		}
	}
	D("(");
	translate(node);
	D(" %s ",t->binary.operat);
	node = t->binary.right;
	translate(node);
	D(")");
}

/**
 * @brief translate_bit_slic : translate bit slicing with many bits
 *
 * @param t : syntax tree node of bit slicing expression
 */
void translate_bit_slic(tree_t *t) {
	tree_t *snode;
	tree_t *enode;
	tree_t *node;
	int be = 0;
	
	if(t->bit_slic.endian) {
		if(!strcmp(t->bit_slic.endian->ident.str, "be")) {
			be = 1;
		} else {
			be = 0;
		}
	}
	node = t->bit_slic.expr;
	D("(");
	translate(node);
	D(" >> ");
	node = t->bit_slic.bit_end;
	if(node) {
		if(!be) {
			translate(node);
		} else {
			translate(t->bit_slic.bit);	
		}
		D(" & ");
		D("MASK1(");
		snode = t->bit_slic.bit;
		enode = t->bit_slic.bit_end;
		if(!be) {
			translate(snode);
			D(" - ");
			translate(enode);
		} else {
			translate(enode);
			D(" - ");
			translate(snode);
		}
		D(")");
	} else {
		node = t->bit_slic.bit;
		translate(node);
		D("& 0x1");
	}
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

/**
 * @brief translate_unary_expr : translate the unary operation expression
 *
 * @param t : syntax tree node of expression
 */
void translate_unary_expr(tree_t *t) {
	tree_t *node;

	D("%s",t->unary.operat);
	if(!strcmp(t->unary.operat, "~")) {
	}

	node = t->unary.expr;
	translate(node);
}

/**
 * @brief translate_pre_expr : translate the pre inc/reduce expression
 *
 * @param t : syntax tree node of expression
 */
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

/**
 * @brief translate_post_expr : translate the post inc/reduce expression
 *
 * @param t : syntax tree node of expression
 */
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

/**
 * @brief translate_expr_brack_direct : translate the expression with brack
 *
 * @param t : syntax tree node of expression
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
		//printf("obj %s\n", obj->name);
		if(!strcmp(obj->obj_type, "interface")) {
			if(!ref_ret.con) {
				tmp = obj->parent;
				ref_name = get_obj_ref(tmp);
				D("%s", ref_name);
				D(".%s", ref_ret.iface->name);
				D("->%s", ref_ret.method);
				D("(");
				D("%s", ref_name);
				//if(tmp->is_array) {
				//		D("[_idx0]");
				//}
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

/**
 * @brief expr_is_bit_slic : check the expression is bit slicing of not
 *
 * @param t : syntax tree node of expression
 *
 * @return : 1 - bit slicing expression
 *			0 - not bit slicing expression
 */
static int expr_is_bit_slic(tree_t *t) {

	type_info_t *info = NULL;

	check_expr_type(t, current_table, &info);
	if(info && info->kind == TYPE_BITFIELD) {
		return 1;
	}
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

/**
 * @brief translate_bit_slice2 : translate the bit slicing with only one bit
 *
 * @param t : syntax tree node of bit slicing
 */
void translate_bit_slice2(tree_t *t) {
	tree_t *expr;
	tree_t *index;
	type_info_t *info = NULL;

	check_expr_type(t, current_table, &info);
	if(info && info->kind == TYPE_BITFIELD) {
		//fprintf(stderr, "translate bitslic\n");
		translate_bit_slic(t);
		return;
	}
	expr = t->bit_slic.expr;
	index = t->bit_slic.bit;
	if(!t->bit_slic.endian) {
		//fprintf(stderr, "translate expr\n");
		translate(expr);
		D("[");
		translate(index);
		D("]");
	}
}

/**
 * @brief translate_bit_slic_assign : translate the bit slicing assign expression
 *
 * @param t : syntax tree node
 */
static void translate_bit_slic_assign(tree_t *t, type_info_t *info) {
	tree_t *sexpr;
	tree_t *eexpr = NULL;
	tree_t *left;
	tree_t *right;
	tree_t *expr;

	/*
	   node = t->expr_assign.left;
	   right = t->expr_assign.right;
	   sexpr = node->bit_slic.bit;
	   */
	sexpr = info->bt.start;
	eexpr = info->bt.end;
	expr = info->bt.expr;
	right = t->expr_assign.right;
	if(!eexpr) {
		eexpr = sexpr;
	}
	/*
	   if(node->bit_slic.bit_end) {
	   eexpr = node->bit_slic.bit_end;
	   } else {
	   eexpr = node->bit_slic.bit;
	   }*/
	//expr = node->bit_slic.expr;
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

static void translate_layout_assign(tree_t *t, type_info_t *info) {
	tree_t *right;
	int offset;
	const char *endian;

	if(!strcmp(info->layout.endian, "\"big-endian\"")) {
		endian = "be";
	} else {
		endian = "le";
	}
	offset = info->layout.offset;
	tree_t *expr;
	D("dml_store_%s_s32(((", endian);
	translate(info->layout.expr);
	D(").data + %d), ", offset);
	right = t->expr_assign.right;
	translate(right);
	D(")");
}

static void translate_layout_bitslic_assign(tree_t *t, type_info_t *info) {
	tree_t *right;
	int offset;
	const char *endian;
	int be;

	if(!strcmp(info->layout.endian, "\"big-endian\"")) {
		endian = "be";
		be = 1;
	} else {
		endian = "le";
		be = 0;
	}
	offset = info->layout.offset;
	tree_t *expr;
	D("dml_store_%s_s32((", endian);
	translate(info->layout.expr);
	D(").data + %d, ", offset);
	{
	tree_t *sexpr;
	tree_t *eexpr = NULL;
	tree_t *left;
	tree_t *expr;

	/*
	   node = t->expr_assign.left;
	   right = t->expr_assign.right;
	   sexpr = node->bit_slic.bit;
	   */
	sexpr = info->bt.start;
	eexpr = info->bt.end;
	expr = info->bt.expr;
	right = t->expr_assign.right;
	if(!eexpr) {
		eexpr = sexpr;
	}
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
	if(be) {
		translate(sexpr);
		D(" - ");
		translate(eexpr);
	} else {
		translate(eexpr);
		D(" - ");
		translate(sexpr);
	}
	D(")");
	D(" << ");
	if(be)
		translate(eexpr);
	else 
		translate(sexpr);
	D(")");
	}
	//right = t->expr_assign.right;
	//translate(right);
	D(")");
}

/**
 * @brief translate_brack_expr : translate the expression in brackc
 *
 * @param t : syntax tree node of expression
 */
void translate_brack_expr(tree_t *t) {
	tree_t *node = t;
	tree_t *expr = node->expr_brack.expr_in_brack;

	D("(");
	translate(expr);
	D(")");
}

void translate_layout(tree_t *t) {
	cdecl_t *type = t->common.cdecl;
	int size;

	type = t->common.cdecl;
	if(type) {
		size = type->layout.size;
		D("struct { char data[%d]; }", size/8);
	} else {
		fprintf(stderr, "fatal error\n");
		exit(-1);
	}
}

void translate_bitfields(tree_t *t) {
	cdecl_t *type;
	int size;

	type = t->common.cdecl;
	size = type->bitfields.bit_size;
	D("uint32");
}
