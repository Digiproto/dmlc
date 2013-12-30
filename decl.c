/*
 * decl.c:
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
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "symbol.h"
#include "tree.h"
#include "types.h"
#include "ast.h"
#include "decl.h"
#include "expression.h"
#include "info_output.h"
#include "object.h"

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table);
params_t* get_param_decl(tree_t* node, symtab_t table) {
	assert(node != NULL);
	if (node->common.type != CDECL_TYPE) {
		error("params type is : %s\n", node->common.name);
	}
	params_t* param = (params_t*)gdml_zmalloc(sizeof(params_t));
	table->pass_num = 0;
	cdecl_t* decl = parse_cdecl(node, table);
	table->pass_num = 1;
	if (decl->common.no_decalare) {
		param->var_name = decl->var_name;
		decl->common.categ = NO_TYPE;
		param->decl = decl;
	}
	else {
		param->var_name = decl->var_name;
		param->decl = decl;
	}

	return param;
}

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table);
void parse_data_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node->cdecl.decl, table);

	if (symbol_defined(table, decl->var_name))
		error("name collision on '%s'\n", decl->var_name);
	symbol_insert(table, decl->var_name, DATA_TYPE, decl);

	return;
}

extern int record_type(cdecl_t* type);
static void insert_record_elems(cdecl_t* type);
void parse_local_decl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node->local_tree.cdecl, table);

	if (node->local_tree.is_static) {
		decl->common.is_static = 1;
	}

	if (node->local_tree.local_keyword) {
		tree_t* keyword = node->local_tree.local_keyword;
		char* keyword_name = (char*)(keyword->local_keyword.name);
		if (strcmp(keyword_name, "auto") == 0) {
			decl->common.is_auto = 1;
		}
		else if (strcmp(keyword_name, "local") == 0) {
			decl->common.is_local = 1;
		}
		else {
			fprintf(stderr, "Other local keyword: %s\n", keyword_name);
			/* FIXME: handle the error */
			exit(-1);
		}
	}

	if (node->local_tree.expr) {
		expr_t* expr = check_expr(node->local_tree.expr, table);
		insert_no_defined_expr(table, node, expr);
		cdecl_t* type = expr->type;
		if (!both_scalar_type(decl, expr->type) &&
				!(both_array_type(decl, expr->type)) &&
				!(is_same_type(decl, expr->type)) &&
				!(is_no_type(decl)) && !is_parameter_type(decl) &&
				!is_no_type(expr->type) && !is_parameter_type(expr->type)) {
			error("incompatible types when initializing '%s'\n", decl->var_name);
		}
	}

	if (record_type(decl)) {
		insert_record_elems(decl);
	}
	symbol_insert(table, decl->var_name, decl->common.categ, decl);

	return;
}

static element_t* add_element(element_t* head, element_t* new) {
	assert(new != NULL);
	if (head == NULL) {
		head = new;
	}
	else {
		element_t* tmp = head;
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new;
	}
	return head;
}

void parse_undef_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* decl = parse_cdecl(node, table);
	if (decl->common.no_decalare == 0) {
		symbol_insert(table, decl->var_name, decl->common.categ, decl);
	}

	return;
}

static cdecl_t* parse_struct(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = STRUCT_T;
	type->struc.table = node->struct_tree.table;

	tree_t* elem_node = node->struct_tree.block;
	cdecl_t* decl = NULL;
	element_t* elem = NULL;
	symtab_t struct_table = type->struc.table;
	while(elem_node) {
		elem = (element_t*)gdml_zmalloc(sizeof(element_t));
		decl = parse_cdecl(elem_node, struct_table);
		if (decl->common.no_decalare == 1) {
			elem_node->common.parse = parse_undef_cdecl;
			undef_var_insert(struct_table, elem_node);
			free(elem); free(decl);
			elem = NULL; decl = NULL;
		}
		else {
			elem->decl = decl;
			type->struc.elem = add_element(type->struc.elem, elem);
		}
		elem_node = elem_node->common.sibling;
	}

	return type;
}

static cdecl_t* parse_layout(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	const char* bitorder = node->layout.desc;
	if (strcmp(bitorder, "\"little-endian\"") && (strcmp(bitorder, "\"big-endian\""))) {
		error("%s not one of \"big-endian\" or \"little-endian\"\n", bitorder);
		return NULL;
	}

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = LAYOUT_T;
	type->layout.table = node->layout.table;
	type->layout.bitorder = strdup(bitorder);

	tree_t* elem_node = node->layout.block;
	cdecl_t* decl = NULL;
	element_t* elem = NULL;
	symtab_t layout_table = type->layout.table;
	/* parse the element about layout */
	while (elem_node) {
		elem = (element_t*)gdml_zmalloc(sizeof(element_t));
		decl = parse_cdecl(elem_node, layout_table);
		if (decl->common.no_decalare == 1) {
			elem_node->common.parse = parse_undef_cdecl;
			undef_var_insert(layout_table, elem_node);
			free(elem); free(decl);
			elem = NULL; decl = NULL;
		}
		else {
			elem->decl = decl;
			type->layout.elem = add_element(type->layout.elem, elem);
		}
		elem_node = elem_node->common.sibling;
	}

	return type;
}

static void parse_bit_expr(tree_t* node, symtab_t table, bit_element_t* elem) {
	assert(node != NULL); assert(table != NULL); assert(elem != NULL);
	cdecl_t* decl = elem->decl;
	/* TODO: calculate the expression */
	/* In simic the bit style: a @ [end : start]*/
	expr_t* start = check_expr(node->bitfields_dec.start, table);
	expr_t* end = check_expr(node->bitfields_dec.end, table);
	if (start->no_defined || end->no_defined) {
		undef_var_insert(table, node);
		return;
	}
	if (start->type->common.categ != INT_T || end->type->common.categ != INT_T) {
		error("the bit filelds's start/end bit should be int\n");
	}
	if (start->is_const && end->is_const) {
		elem->start = start->val->int_v.value;
		elem->end =  end->val->int_v.value;
		elem->size = elem->end - elem->start;
	} else {
		error("the bitfiled '%s' should be constant\n", decl->var_name);
	}

	return;
}

void parse_undef_bit_elem(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	bit_element_t* elem = (bit_element_t*)gdml_zmalloc(sizeof(bit_element_t));
	cdecl_t* decl = parse_cdecl(node->bitfields_dec.decl, table);
	if (decl->common.no_decalare == 0) {
		if (decl->common.categ != INT_T) {
			error("illegal bitfields definition: non-integer field");
			free(decl); decl = NULL;
		}
		else {
			elem->decl = decl;
			parse_bit_expr(node, table, elem);
			symbol_insert(table, elem->decl->var_name, elem->decl->common.categ, elem);
		}
	}

	return;
}

static bit_element_t* parse_bit_element(tree_t* node, symtab_t table) {
	assert(table != NULL);
	bit_element_t* elem = (bit_element_t*)gdml_zmalloc(sizeof(bit_element_t));
	cdecl_t* decl = parse_cdecl(node->bitfields_dec.decl, table);
	if (decl->common.no_decalare == 1) {
		node->common.parse = parse_undef_bit_elem;
		undef_var_insert(table, node);
		free(decl); free(elem);
		decl = NULL; elem = NULL;
	}
	else if (decl->common.categ != INT_T) {
		free(decl); free(elem);
		decl = NULL; elem = NULL;
		error("illegal bitfields definition: non-integer field");
	}
	else {
		elem->decl = decl;
		parse_bit_expr(node, table, elem);
	}
	return elem;
}

static bit_element_t* add_bit_element(bit_element_t* head, bit_element_t* new) {
	assert(new != NULL);
	if (head == NULL) {
		head = new;
	} else {
		bit_element_t* tmp = head;
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new;
	}

	return head;
}

static cdecl_t* parse_bitfields(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	int size = node->bitfields.size;
	if (size > 64) {
		error("illegal bitfields definition: bitfields width is > 64\n");
		return NULL;
	}

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = BITFIELDS_T;
	type->bitfields.size = size;
	type->bitfields.table = node->bitfields.table;

	tree_t* elem_node = node->bitfields.block;
	bit_element_t* elem = NULL;
	symtab_t bitfield_table = type->bitfields.table;
	/* TODO: parse the elements about bitfields */
	while (elem_node) {
		elem = parse_bit_element(elem_node, bitfield_table);
		if (elem != NULL) {
			type->bitfields.elem = add_bit_element(type->bitfields.elem, elem);
		}
		elem_node = elem_node->common.sibling;
	}

	return type;
}

static cdecl_t* parse_typeof(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = get_typeof_type(node, table);

	return type;
}

static int get_int_size(const char* int_str) {
	assert(int_str);
	int size = 0;
	if (strcmp(int_str, "int") == 0) {
		size = sizeof(int) * 8;
	}
	else if ((!strcmp(int_str, "uint") || !strcmp(int_str, "int0") ||
		!strcmp(int_str, "uint0"))) {
		error("unknown type: '%s'\n", int_str);
	}
	else {
		const char* data = NULL;
		if (strstr(int_str, "uint"))
			data = &int_str[4];
		else
			data = &int_str[3];
		size = atoi(data);
		if (size > 128)
			error("unknown type: '%s'\n", int_str);
	}

	return size;
}

static cdecl_t* parse_type_ident(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	if (node->common.type == C_KEYWORD_TYPE) {
		switch(node->ident.type) {
			case BOOL_TYPE:
				type->common.categ = BOOL_T;
				type->common.size = sizeof(char) * 8;
				break;
			case CHAR_TYPE:
				type->common.categ = CHAR_T;
				type->common.size = sizeof(char) * 8;
				break;
			case DOUBLE_TYPE:
				type->common.categ = DOUBLE_T;
				type->common.size = sizeof(double) * 8;
				break;
			case FLOAT_TYPE:
				type->common.categ = FLOAT_T;
				type->common.size = sizeof(float) * 8;
				break;
			case INT_TYPE:
				type->common.categ = INT_T;
				type->common.size = get_int_size(node->ident.str);
				break;
			case LONG_TYPE:
				type->common.categ = LONG_T;
				type->common.size = sizeof(long) * 8;
				break;
			case SHORT_TYPE:
				type->common.categ = SHORT_T;
				type->common.size = sizeof(short) * 8;
				break;
			case VOID_TYPE:
				type->common.categ = VOID_T;
				break;
			default:
				error("unknown type: '%s'\n", node->ident.str);
				break;
		}
	}
	else {
		symbol_t symbol = symbol_find_notype(table, node->ident.str);
		if (symbol == NULL) {
			symtab_t root_table = get_root_table();
			symbol = symbol_find_notype(root_table, node->ident.str);
		}
		/* In simics, some method parameter can not have type*/
		if (symbol) {
			if ((symbol->type == TYPEDEF_T) || (symbol->type == STRUCT_T)) {
				memcpy(type, symbol->attr, sizeof(cdecl_t));
				type->var_name = NULL;
			} else if (symbol->type == STRUCT_TYPE) {
				/* some struct defined like this:
					 struct abc {
						struct abc ab;
					}
				but in the "struct abc ab", the "struct abc" do not have
				type block, so should mark 'struct abc ab" undefined, and
				for the second time to parse it*/
				struct_attr_t* attr = symbol->attr;
				if (attr->decl) {
					memcpy(type, ((struct_attr_t*)(symbol->attr))->decl, sizeof(cdecl_t));
					type->var_name = NULL;
				} else {
					type->var_name = node->ident.str;
					type->common.no_decalare = 1;
				}
			} else if (symbol) {
				type->var_name = node->ident.str;
				type->common.categ = NO_TYPE;
			} else {
				error("unknown type : %s, type: %d, ATTRIBUTE: %d\n", symbol->name, symbol->type, ATTRIBUTE_TYPE);
			}
		}
		else {
			if (table->pass_num == 0) {
				type->var_name = node->ident.str;
				type->common.no_decalare = 1;
				DEBUG_DECL("unknown symbol: %s\n", type->var_name);
			}
			else {
				error("'%s' isn't declared (first use)\n", node->ident.str);
			}
		}
	}

	return type;
}

static cdecl_t* parse_base_type(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = NULL;
	switch(node->common.type) {
		case STRUCT_TYPE:
			type = parse_struct(node, table);
			break;
		case LAYOUT_TYPE:
			type = parse_layout(node, table);
			break;
		case BITFIELDS_TYPE:
			type = parse_bitfields(node, table);
			break;
		case TYPEOF_TYPE:
			type = parse_typeof(node, table);
			break;
		default:
			type = parse_type_ident(node, table);
			break;

	}
	return type;
}

static void parse_cdecl2(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_point(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);

	int qual = type->common.qual;
	type->common.qual = 0;
	/* in the declare about c function, it's parameter
	 * can not have name, only have it's type */
	if (node->cdecl.decl)
		parse_cdecl2(node->cdecl.decl, table, type);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = POINTER_TO;
	drv->qual = type->common.qual;
	drv->next = type->common.drv;
	type->common.drv = drv;
	type->common.qual = qual;

	return;
}

static void parse_cdecl3(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_c_array(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	parse_cdecl3(node->array.decl, table, type);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = ARRAY_OF;
	drv->len = check_expr(node->array.expr, table);
	drv->next = type->common.drv;
	if (((expr_t*)(drv->len))->no_defined) {
		type->common.no_decalare = 1;
		return;
	}
	type->common.drv = drv;

	return;
}

static void free_param(void** data, int i) {
	assert(data != NULL);
	while (i) {
		free(data[--i]);
	}
	return;
}

static signature_t* parse_function_param(tree_t* node, symtab_t table) {
	assert(table != NULL);
	if (node == NULL) return NULL;

	signature_t* sig = (signature_t*)gdml_zmalloc(sizeof(signature_t));
	int param_num = get_param_num(node);
	vector_t* vect = (vector_t*)gdml_zmalloc(sizeof(vector_t));
	vect->data = (void**)gdml_zmalloc(param_num * sizeof(void*));
	vect->len = param_num;
	sig->params = vect;

	tree_t* tmp = node;
	param_t* param = NULL;
	cdecl_t* ty = NULL;
	int i = 0;
	while (tmp) {
		param = (param_t*)gdml_zmalloc(sizeof(param_t));
		if (tmp->common.type == ELLIPSIS_TYPE) {
			sig->has_ellipse = 1;
			param->is_ellipsis = 1;
			break;
		}
		tree_t* basetype = tmp->cdecl.basetype;
		int type = basetype->common.type;
		ty = parse_cdecl(tmp, table);
		param->id = ty->var_name;
		param->ty = ty;
		vect->data[i++] = param;
		if (ty->common.no_decalare) {
			sig->has_no_decalare = 1;
			free_param(vect->data, i);
			free(vect->data); free(vect);
			break;
		}
		if((ty->common.categ == VOID_T) && (tmp->cdecl.decl == NULL)) {
			if (vect->len == 1) {
				free_param(vect->data, i);
				vect->len = 0;
			} else {
				free_param(vect->data, i);
				error("void’ must be the only parameter\n");
			}
		}
		tmp = tmp->common.sibling;
	}
	return sig;
}

static void parse_function(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	parse_cdecl3(node->cdecl_brack.cdecl, table, type);
	signature_t* sig = parse_function_param(node->cdecl_brack.decl_list, table);
	if (sig && (sig->has_no_decalare)) {
		type->common.no_decalare = 1;
		free(sig);
		return;
	}
	DEBUG_DECL("function name: %s\n", type->var_name);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	drv->ctor = FUNCTION_RETURN;
	drv->sig = sig;
	drv->next = type->common.drv;
	type->common.drv = drv;

	return;
}

static int is_c_keyword(tree_t* node) {
	assert(node != NULL);
	if (node->common.type == C_KEYWORD_TYPE)
		return 1;
	else
		return 0;
}

static void parse_ident(tree_t* node, symtab_t table, cdecl_t* decl) {
	assert(node != NULL); assert(table != NULL); assert(decl != NULL);
	if (is_c_keyword(node) && (decl->common.is_typedef == 0)) {
		error("c keyword can not be the identifier name\n");
	}
	else {
		decl->var_name = node->ident.str;
	}

	return;
}

static void parse_cdecl3(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL) return;

	if (node->common.type == ARRAY_TYPE) {
		parse_c_array(node, table, type);
	}
	else if (node->common.type == CDECL_BRACK_TYPE) {
		if (node->cdecl_brack.cdecl) {
			parse_function(node, table, type);
		} else {
			parse_cdecl2(node->cdecl_brack.decl_list, table, type);
		}
	}
	else {
		parse_ident(node, table, type);
	}

	return;
}

static void parse_cdecl2(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL) return;
	if (node->common.type != CDECL_TYPE)
		parse_cdecl3(node, table, type);

	if (node->cdecl.is_const == 1) {
		type->common.qual |= CONST_QUAL;
		parse_cdecl2(node->cdecl.decl, table, type);
	}
	if (node->cdecl.is_vect == 1) {
		type->common.qual |= VECT_QUAL;
		parse_cdecl2(node->cdecl.decl, table, type);
	}
	if (node->cdecl.is_point == 1) {
		parse_point(node, table, type);
	}

	return;
}

static cdecl_t* qualify(int qual, cdecl_t* type) {
	assert(type != NULL);
	if ((qual == 0) || (qual == type->common.qual))
		return type;

	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	*ty = *type;
	ty->common.qual |= qual;
	if (ty->common.qual != 0) {
		ty->common.bty = ty->common.bty;
	} else {
		ty->common.bty = ty;
	}

	return ty;
}

cdecl_t* pointer_to(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = POINTER_T;
	ty->common.qual = 0;
	ty->common.size = sizeof(void*);
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

cdecl_t* array_of(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = ARRAY_T;
	ty->common.qual = 0;
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

static cdecl_t* function_return(cdecl_t* type, signature_t* sig) {
	assert(type != NULL);
	cdecl_t* ty = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	ty->common.categ = FUNCTION_T;
	ty->common.qual = 0;
	ty->common.size = sizeof(void*);
	ty->function.sig = sig;
	ty->common.bty = type;
	ty->var_name = type->var_name;

	return ty;
}

static void free_drv(type_deriv_list_t* head) {
	if (head == NULL) return;

	type_deriv_list_t* tmp = head;
	type_deriv_list_t* drv = head;
	while (drv) {
		tmp = drv->next;
		free(drv);
		drv = tmp;
	}
	return;
}

static cdecl_t* derive_type(cdecl_t* type) {
	assert(type != NULL);
	cdecl_t* ty = type;
	type_deriv_list_t* drv = type->common.drv;
	if (type->common.no_decalare == 1) {
		free_drv(type->common.drv);
		return ty;
	}
	while (drv != NULL) {
		if (drv->ctor == POINTER_TO) {
			cdecl_t* tmp = pointer_to(ty);
			ty = qualify(drv->qual, tmp);
		}
		else if (drv->ctor == ARRAY_OF) {
			if (type->common.categ == FUNCTION_T)
				return NULL;
			ty = array_of(ty);
		}
		else {
			if (ty->common.categ == ARRAY_T || ty->common.categ == FUNCTION_T)
				return NULL;
			ty = function_return(ty, drv->sig);
		}
		drv = drv->next;
	}
	return ty;
}

static cdecl_t* parse_base(tree_t* node, symtab_t table, int is_const) {
	assert(node != NULL); assert(table != NULL);
	/* parse the decal base type*/
	tree_t* base = node;
	cdecl_t* type = parse_base_type(base, table);
	if (is_const)
		type->common.qual |= CONST_QUAL;

	return type;
}

static cdecl_t* parse_decl(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(table != NULL); assert(type != NULL);
	if (node == NULL)
		return type;
	cdecl_t* ty = type;
	parse_cdecl2(node, table, ty);
	ty = derive_type(ty);
	if (ty == NULL)
		error("illegal type\n");

	return ty;
}

static cdecl_t* parse_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* parse the decal base type*/
	tree_t* base = node->cdecl.basetype;
	cdecl_t* type = parse_base(base, table, node->cdecl.is_const);

	/* parse the indentifier, but the simics, there can be some
	 * extern c functions with parameters that no have name, but
	 * only have type; and the method about simics, that can only
	 * have name, not have type */
	tree_t* decl = node->cdecl.decl;
	type = parse_decl(decl, table, type);

	return type;
}

void parse_extern_cdecl_or_ident(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* decl = parse_cdecl(node->cdecl.decl, table);
	decl->common.is_extern = 1;
	/* as some type and identifier can be used before declares
	 * so when the first pass times, insert the undefined node
	 * into table so as the second passing time the check them*/
	if (table->pass_num == 0) {
		if (decl->common.no_decalare == 1) {
			tree_t* tmp = node->cdecl.decl;
			/* in simics there can like "extern VNULL "*/
			if ((decl->var_name) &&
					((tmp->cdecl.basetype) && (tmp->cdecl.decl == NULL))) {
				if (symbol_insert(table, decl->var_name, decl->common.categ, decl)) {
					//wa("duplicate '%s'\n", decl->var_name);
					return;
				}
			}
			else {
				free(decl);
				undef_var_insert(table, node);
			}
		}
		else {
			if (decl->var_name) {
				symbol_insert(table, decl->var_name, decl->common.categ, decl);
			}
			else {
				error("expected identifier about extern\n");
			}
		}
	}
	else {
		if ((decl->var_name) && (decl->common.no_decalare == 0)) {
				int rt = symbol_insert(table, decl->var_name, decl->common.categ, decl);
				if (rt) {
					//error("duplicate '%s'\n", decl->var_name);
					return;
				}
		}
	}

	return;
}

int record_type(cdecl_t* type) {
	assert(type != NULL);
	int is_record_type = 0;
	switch(type->common.categ) {
		case STRUCT_T:
		case LAYOUT_T:
		case BITFIELDS_T:
			is_record_type = 1;
			break;
		default:
			is_record_type = 0;
			break;
	}
	return is_record_type;
}

static void insert_record_elems(cdecl_t* type) {
	assert(type != NULL);
	if (type->common.categ == STRUCT_T) {
		element_t* elem = type->struc.elem;
		symtab_t table = type->struc.table;
		while(elem) {
			int categ = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, categ, elem->decl);
			if (record_type(elem->decl)) {
				insert_record_elems(elem->decl);
			}
			elem = elem->next;
		}
	}
	else if (type->common.categ == LAYOUT_T) {
		element_t* elem = type->layout.elem;
		symtab_t table = type->layout.table;
		while(elem) {
			int categ = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, categ, elem->decl);
			if (record_type(elem->decl))
				insert_record_elems(elem->decl);
			elem = elem->next;
		}
	}
	else {
		bit_element_t* elem = type->bitfields.elem;
		symtab_t table = type->bitfields.table;
		while(elem) {
			int catag = elem->decl->common.categ;
			symbol_insert(table, elem->decl->var_name, BITFIELDS_ELEM_TYPE, elem);
			if (record_type(elem->decl))
				insert_record_elems(elem->decl);
			elem = elem->next;
		}
	}

	return;
}

void parse_typedef_cdecl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	//cdecl_t* type = parse_cdecl(node->cdecl.decl, table);
	tree_t* decl = node->cdecl.decl;
	cdecl_t* type =  parse_base(decl->cdecl.basetype, table, decl->cdecl.is_const);
	type->common.is_typedef = 1;
	type = parse_decl(decl->cdecl.decl, table, type);
	if (type == NULL)
		return;

	const char* name = type->var_name;
	/* insert the elements that in struct layout, bitfield
	 * into it's table */
	if (record_type(type)) {
		insert_record_elems(type);
	}

	if (table->pass_num == 0) {
		if (type->common.no_decalare == 1) {
			undef_var_insert(table, node);
			free(type);
		}
		else {
			if (type->var_name) {
				type->typedef_name = strdup(type->var_name);
				symbol_insert(table, type->var_name, TYPEDEF_T, type);
			} else {
				error("useless type name in empty declaration\n");
			}
		}
	}
	else {
		if ((type->var_name) && (type->common.no_decalare == 0)) {
			type->typedef_name = strdup(type->var_name);
			if (symbol_insert(table, type->var_name, TYPEDEF_T, type))
				error("duplicate defined '%s'\n", type->var_name);
		}
	}

	return;
}

void parse_top_struct_cdecl(tree_t* node, symtab_t table, void* attr) {
	assert(node != NULL); assert(table != NULL); assert(attr != NULL);

	cdecl_t* decl = parse_struct(node, table);
	insert_record_elems(decl);
	((struct_attr_t*)attr)->decl = decl;

	return;
}

static void parse_star(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	type_deriv_list_t* drv = (type_deriv_list_t*)gdml_zmalloc(sizeof(type_deriv_list_t));
	if (node->stars.is_const)
		drv->qual |= CONST_QUAL;
	drv->ctor = POINTER_TO;
	drv->next = type->common.drv;
	type->common.drv = drv;

	if (node->stars.stars) {
		parse_star(node->stars.stars, table, type);
	}

	return;
}

static void parse_ctype_decl_prt(tree_t* node, symtab_t table, cdecl_t* type);
static void parse_ctypedecl_arr(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	if (node->cdecl_brack.decl_list) {
		parse_ctype_decl_prt(node->cdecl_brack.decl_list, table, type);
	}

	return;
}

static void parse_ctype_decl_prt(tree_t* node, symtab_t table, cdecl_t* type) {
	assert(node != NULL); assert(table != NULL); assert(type != NULL);
	if (node->ctypedecl_ptr.stars) {
		parse_star(node->ctypedecl_ptr.stars, table, type);
	}

	if (node->ctypedecl_ptr.array) {
		parse_ctypedecl_arr(node->ctypedecl_ptr.array, table, type);
	}

	return;
}

cdecl_t* parse_ctype_decl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t* type = parse_base(node->ctypedecl.basetype, table, 0);
	if (node->ctypedecl.const_opt)
		type->common.qual |= CONST_QUAL;

	if (node->ctypedecl.ctypedecl_ptr) {
		parse_ctype_decl_prt(node->ctypedecl.ctypedecl_ptr, table, type);
	}

	type = derive_type(type);
	if (type == NULL)
		error("illegal type\n");

	return type;
}

cdecl_t* parse_typeoparg(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	tree_t* tmp = node->typeoparg.ctypedecl;
	tree_t* type_node = tmp ? tmp : node->typeoparg.ctypedecl_brack;
	cdecl_t* type = parse_ctype_decl(type_node, table);

	return type;
}

