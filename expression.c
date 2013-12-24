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
#include "chk_common.h"

/* it's a temporary proposal.
 * use the function so frequently that I add this marco.
 * by eJim Lee 2013-11-30 */
extern obj_ref_t *OBJ;
inline static object_t* get_current_obj() {
	return OBJ->obj;
}

void set_current_obj(object_t* obj) {
	if (OBJ == NULL) {
		OBJ = (obj_ref_t*)gdml_zmalloc(sizeof(obj_ref_t));
	}
	OBJ->obj = obj;
}

void set_obj_array() {
	object_t* obj = get_current_obj();
	obj->is_array = 1;
	return;
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

extern device_t* get_device();
symbol_t get_symbol_from_banks(const char* name) {
	assert(name != NULL);
	struct list_head *p;
	object_t *tmp;
	bank_attr_t* attr = NULL;
	symbol_t symbol = NULL;
	device_t* dev = get_device();
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		symbol = defined_symbol(tmp->symtab->sibling, name, 0);
		if (symbol)
			return symbol;
	}
	return symbol;
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
			ref->name = expr_node->ident.str;
			break;
		case QUOTE_TYPE:
			ident_node = expr_node->quote.ident;
			ref->name = ident_node->ident.str;
			if (check_dml_obj_refer(ident_node, table) == 0) {
				PERRORN("reference to unknown object '%s', table_num: %d, line: %d", node,
						ident_node->ident.str, table->table_num, __LINE__);
			}
			else {
				expr->is_obj = 1;
			}
			break;
		case COMPONENT_TYPE:
			ident_node = expr_node->component.ident;
			ref->name = ident_node->ident.str;
			if (node->component.type == COMPONENT_POINTER_TYPE) {
				ref->is_pointer = 1;
			}
			break;
		default:
			break;
	}

	return ref;
}

reference_t* get_reference(tree_t* node, reference_t* ref, expr_t* expr, symtab_t table) {
	assert(node != NULL); assert(ref != NULL);
	assert(expr != NULL); assert(table != NULL);

	tree_t* ident_node = NULL;
	tree_t* expr_node = NULL;
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
			expr_node = node->bit_slic.expr;
			if (expr_node->common.type == COMPONENT_TYPE) {
				new = get_reference(expr_node->component.expr, new, expr, table);
			} else if (expr_node->common.type == BIT_SLIC_EXPR_TYPE) {
				new = get_reference(expr_node->bit_slic.expr, ref, expr, table);
			}
			break;
		case QUOTE_TYPE:
			ident_node = node->quote.ident;
			new->name = ident_node->ident.str;
			if (check_dml_obj_refer(ident_node, table) == 0) {
				PERRORN("reference to unknown object '%s'", node, ident_node->ident.str);
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
		case EXPR_BRACK_TYPE:
			free(new);
			new = get_reference(node->expr_brack.expr_in_brack, ref, expr, table);
			break;
		default:
			PERRORN("other node type: %s", node, node->common.name);
			/* FIXME: handle the error */
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

static symtab_t get_tempalte_table(const char* name) {
	assert(name != NULL);
	symbol_t symbol = get_symbol_from_root_table(name, TEMPLATE_TYPE);
	template_attr_t* attr = NULL;
	symtab_t table = NULL;
	if (symbol) {
		check_template_parsed(name);
		attr = symbol->attr;
		return attr->table;
	} else {
		fprintf(stderr, "Can not find temmplate '%s'\n", name);
		exit(-1);
	}
	return NULL;
}

extern symtab_t get_obj_param_table(symtab_t table, symbol_t symbol);
symtab_t get_default_symbol_tale(symtab_t table, symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t ret_table = NULL;

	if (strcmp(symbol->name, "dev") == 0) {
		ret_table = get_root_table();
	} // device
	else if (strcmp(symbol->name, "this") == 0) {
		ret_table = get_current_table();
	}
	else if (strcmp(symbol->name, "fields") == 0) {
		ret_table = get_tempalte_table("field");
	}
	else if (strcmp(symbol->name, "reg") == 0) {
		ret_table = get_tempalte_table("register");
	}
	else if (strcmp(symbol->name, "banks") == 0 ||
			strcmp(symbol->name, "bank") == 0 ||
			strcmp(symbol->name, "default_bank") == 0) {
		ret_table = get_tempalte_table("bank");
	}
	else if (strcmp(symbol->name, "interface") == 0) {
		ret_table = get_tempalte_table("interface");
	}
	else {
		ret_table = get_obj_param_table(table, symbol);
	}

	return ret_table;
}

symtab_t get_object_table(symbol_t symbol) {
	assert(symbol != NULL);
	object_t* obj = symbol->attr;
	tree_t* node = obj->node;
	return obj->symtab;
}

#define get_record_table(type) \
		do { \
			switch (type->common.categ) { \
				case STRUCT_T: \
					table = type->struc.table; \
					break; \
				case LAYOUT_T: \
					table = type->layout.table; \
					break; \
				default: \
					table = type->bitfields.table; \
					break; \
			} \
		} while(0)

symtab_t get_data_table(symbol_t symbol) {
	assert(symbol != NULL);
	cdecl_t* type = symbol->attr;
	symtab_t table = NULL;

	if (type->common.categ == POINTER_T) {
		symbol_t new_sym = (symbol_t)gdml_zmalloc(sizeof(struct symbol));
		new_sym->name = symbol->name;
		new_sym->attr = type->common.bty;
		table = get_data_table(new_sym);
		new_sym->name = NULL; new_sym->attr = NULL;
		free(new_sym);
	}
	else if (record_type(type)) {
		get_record_table(type);
	}
	else if (is_array_type(type)) {
		type = type->common.bty;
		if (record_type(type)) {
			get_record_table(type);
		}
		else if (is_array_type(type)) {
			type = type->common.bty;
			get_record_table(type);
		}
		else {
			error("other data '%s' type: %d\n", symbol->name, type->common.categ);
		}
	}
	else {
		error("other data '%s' type: %d\n", symbol->name, type->common.categ);
	}

	return table ;
}

static symtab_t get_point_table(symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	cdecl_t* attr = symbol->attr;
	cdecl_t* type = attr->common.bty;
	if (record_type(type)) {
		get_record_table(type);
	} else {
		fprintf(stderr, "other pointer type: %d\n", type->common.categ);
		exit(-1);
	}

	return table;
}

static symtab_t get_array_table(symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	cdecl_t* attr = symbol->attr;
	cdecl_t* type = attr->common.bty;
	symbol_t new_sym = NULL;
	if (type->common.categ == POINTER_T) {
		new_sym = (symbol_t)gdml_zmalloc(sizeof(symbol_t));
		new_sym->name = symbol->name;
		new_sym->attr = type->common.bty;
		table = get_array_table(new_sym);
		new_sym->name = NULL; new_sym->attr = NULL;
		free(new_sym);
	}
	else if (record_type(type)) {
		get_record_table(type);
	}
	else {
		fprintf(stderr, "other array type: %d\n", type->common.categ);
		exit(-1);
	}
	return table;
}


static symtab_t get_typedef_table(symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	cdecl_t* type = (cdecl_t*)(symbol->attr);
	if (record_type(type)) {
		get_record_table(type);
	}
	else  {
		table = NULL;
	}
	return table;
}

static symtab_t get_select_table(symtab_t sym_tab, symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	select_attr_t* attr = symbol->attr;
	if (!strcmp(symbol->name, "bank") && (attr->type == PARAMETER_TYPE)) {
		table = get_default_symbol_tale(sym_tab, symbol);
	} else {
		printf("othe select : %s\n", symbol->name);
		exit(-1);
	}
	return table;
}

extern symtab_t get_symbol_table(symtab_t sym_tab, symbol_t symbol);
static symtab_t get_param_table(symtab_t sym_tab, symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	params_t* param = symbol->attr;
	if (param->is_notype || param->decl == NULL) {
		return (void*)(0xFFFFFFFF);
	}
	cdecl_t* type = param->decl;
	if (type->common.categ == POINTER_T) {
		type = type->common.bty;
	}
	if (type->common.categ == NO_TYPE) {
		return (void*)0XFFFFFFFF;
	}
	if (record_type(type)) {
		get_record_table(type);
	}
	else {
		symbol = get_symbol_from_root_table(symbol->name, 0);
		if (symbol) {
			table = get_symbol_table(sym_tab->parent, symbol);
		} else {
			fprintf(stderr, "method param is other type '%d'\n", type->common.categ);
			exit(-1);
		}
	}

	return table;
}

symtab_t get_symbol_table(symtab_t sym_tab, symbol_t symbol) {
	assert(symbol != NULL);

	symtab_t table = NULL;
	void* attr = symbol->attr;

	int *a = NULL;
	switch(symbol->type) {
		case TEMPLATE_TYPE:
			table = ((template_attr_t*)attr)->table;
			break;
		case STRUCT_T:
			table = ((cdecl_t*)attr)->struc.table;
			break;
		case BITFIELDS_T:
			table = ((cdecl_t*)attr)->bitfields.table;
			break;
		case LAYOUT_T:
			table = ((cdecl_t*)attr)->layout.table;
			break;
		case TYPEDEF_T:
			table = get_typedef_table(symbol);
			break;
		case ARRAY_T:
			table = get_array_table(symbol);
			break;
		case POINTER_T:
			table = get_point_table(symbol);
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
			if (table->undef_temp) {
				check_undef_template(table);
			}
			break;
		case PARAMETER_TYPE:
			table = get_default_symbol_tale(sym_tab, symbol);
			break;
		case OBJECT_TYPE:
			table = get_object_table(symbol);
			break;
		case DATA_TYPE:
			table = get_data_table(symbol);
			break;
		case PARAM_TYPE:
			table = get_param_table(sym_tab, symbol);
			break;
		case SELECT_TYPE:
			table = get_select_table(sym_tab, symbol);
			break;
		case STRUCT_TYPE:
			table = ((struct_attr_t*)attr)->table;
			break;
		default:
			error("Othe symbol %s(%d)\n", symbol->name, symbol->type);
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

	if (expr->type->common.categ == FUNCTION_T) {
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

	return 1;
}

#define cal_muti_const_value(expr, op) \
		do { \
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
			} \
		} while (0)

#define cal_int_const(expr, op) \
	do { \
		if ((is_double_type(expr->kids[0]->type)) || is_double_type(expr->kids[1]->type)) { \
			error("invalid operands to binary op (have ‘double’ and ‘int’)\n"); \
			expr->type->common.categ = NO_TYPE; \
		} \
		else { \
			expr->type->common.categ = INT_T; \
			expr->val->int_v.value = expr->kids[0]->val->int_v.value op expr->kids[1]->val->int_v.value; \
		} \
	} while(0)

#define cal_unary(expr, op) \
	do { \
		if (is_double_type(expr->kids[0]->type)) { \
			expr->type->common.categ = DOUBLE_T; \
			expr->val->d = op(expr->kids[0]->val->d); \
		} \
		else { \
			expr->type->common.categ = INT_T; \
			expr->val->int_v.value = op(expr->kids[0]->val->int_v.value); \
		} \
	} while(0)

#define cal_unary_int(expr, op) \
	do { \
		if (is_double_type(expr->kids[0]->type)) { \
			error("wrong type argument to bit-complement\n"); \
			expr->type->common.categ = NO_TYPE; \
		} \
		else { \
			expr->type->common.categ = INT_T; \
			expr->val->int_v.value = op(expr->kids[0]->val->int_v.value); \
		} \
	} while (0)

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
			PERRORN("other constant operator: %d", expr->node, expr->op);
			break;
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
		PERRORN("other node type: %s", node, node->common.name);
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
		do { \
			if (expr->is_const) { \
				if (expr->type->common.categ == DOUBLE_T) { \
					expr->val->int_v.value = (int)(expr->val->d); \
				} \
			} \
			else { \
				expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t)); \
			} \
			expr->type->common.categ = INT_T; \
			expr->type->common.size = sizeof(int) * 8; \
		} while (0)

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
	else if (is_parameter_type(expr->kids[0]->type) || is_parameter_type(expr->kids[1]->type)
		|| (is_no_type(expr->kids[0]->type) || is_no_type(expr->kids[1]->type))) {
		expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
		expr->type->common.categ = INT_T;
		expr->type->common.size = sizeof(int) * 8;
		return (expr);
	}
	error("Invalid operands\n");
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
	else if (is_ptr_type(type1) || is_ptr_type(type2)
		|| is_array_type(type1) || is_array_type(type2)) {
		new_int_type(expr);
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
	cdecl_t* type1 = expr->kids[0]->type;
	cdecl_t* type2 = expr->kids[1]->type;
	if (both_int_type(type1, type2)) {
		expr = cal_const_value(expr);
		expr_to_int(expr);
		return expr;
	} else if ((is_parameter_type(type1) || is_parameter_type(type2)) ||
		(is_no_type(type1) || is_no_type(type2))) {
		expr->type = type1->common.categ > type2->common.categ ? type1 : type2;
		return expr;
	} else if (is_ptr_type(type1) || is_ptr_type(type2) ||
			is_array_type(type1) || is_array_type(type2)) {
		new_int_type(expr);
		return expr;
	} else {
		error("Invalid operands line : %d\n", __LINE__);
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
		error("Invalid operands line : %d\n", __LINE__);
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
		error("Invalid operands, line = %d\n", __LINE__);
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
		(is_ptr_type(type2) && is_int_type(type1))) {
		//warning("conversion between pointer and integer without a cast");
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

	if (is_same_type(l_type, r_type)) {
		return 1;
	} else if (is_ptr_type(l_type) && is_null_ptr(r_expr)) {
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
			PERRORN("unkown type: %s", node, node->common.name);
			break;
	}
	expr->node = node;
	return expr;
}

expr_t* check_ternary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	expr_t* cond_expr = check_expr(node->ternary.cond, table);
	if (!is_scalar_type(cond_expr->type) &&
		!is_no_type(cond_expr->type) &&
		!is_parameter_type(cond_expr->type)) {
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
	expr = check_expression(node->sizeof_tree.expr, table, expr);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;

	return expr;
}

expr_t* check_unary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr->op = node->unary.type;
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
			expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			expr->type->common.categ = BOOL_T;
			expr->type->common.size = sizeof(int) * 8;
			break;
		case EXPR_TO_STR_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (expr->kids[0]->is_const) {
				expr->type = expr->kids[0]->type;
				if (expr->type->common.categ == INT_T) {
					expr->val->p = gdml_zmalloc(sizeof(long)*8);
					sprintf(expr->val->p, "%lld", (expr->val->int_v.value));
				}
				else if (expr->type->common.categ == DOUBLE_T) {
					expr->val->p = gdml_zmalloc(sizeof(double)*8);
					sprintf(expr->val->p, "%lf", expr->val->d);
				}
				expr->type->common.categ = STRING_T;
			}
			else {
				expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
				expr->type->common.categ = STRING_T;
			}
			break;
		case PRE_INC_OP_TYPE:
		case PRE_DEC_OP_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if (is_int_type(expr->kids[0]->type)) {
				expr = cal_const_value(expr);
			}
			else if (is_array_type(expr->kids[0]->type) ||
				is_parameter_type(expr->kids[0]->type)){
				expr->type = expr->kids[0]->type;
			}
			else {
				error("lvalue required as increment/decrement operand\n");
			}
			break;
		case AFT_INC_OP_TYPE:
		case AFT_DEC_OP_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			if ((is_int_type(expr->kids[0]->type) ||
				is_array_type(expr->kids[0]->type) ||
				is_parameter_type(expr->kids[0]->type))) {
				expr->type = expr->kids[0]->type;
			}
			else {
				error("lvalue required as increment/decrement operand\n");
			}
			break;
		default:
			PERRORN("other unary type: %s(%d)", node, node->common.name, node->unary.type);
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
			PERRORN("other const type: %s", node, node->common.name);
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
	case PARAM_TYPE:
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
	if (symbol) {
		obj_type = dml_obj_type(symbol);
		if (obj_type) {
		    return obj_type;
		}
		else {
			symbol = get_symbol_from_root_table(name, 0);
			if (symbol)
				obj_type = dml_obj_type(symbol);
			else {
				symbol = get_symbol_from_banks(name);
				if(symbol)
					obj_type = dml_obj_type(symbol);
			}
			return obj_type;
		}
	}
	else {
		symbol = get_symbol_from_root_table(name, 0);
		if (!symbol)
			symbol = get_symbol_from_banks(name);
		if (symbol)
			obj_type = dml_obj_type(symbol);
		return obj_type;
	}

	return obj_type;
}

static int check_dml_obj_refer(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	int obj_type = 0;
	char* refer_name = (char*)(node->ident.str);
	if (strcmp(refer_name, "this") == 0) {
		obj_type = 1;
	}
	else {
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
#if 0
	if (is_obj == 0 && table->no_check == 0) {
		PWARNN("reference to unknown object '%s', table_num: %d, line: %d", node,
			ident->ident.str, table->table_num, __LINE__);
	}
#endif

	expr->node = node;
	if (table->no_check && is_obj == 0) {
		cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
		type->common.categ = NO_TYPE;
		expr->type = type;
	} else {
		expr = check_expression(node->quote.ident, table, expr);
	}

	return expr;
}

static cdecl_t* check_parameter_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL);
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->val = (value_t*)gdml_zmalloc(sizeof(value_t));
	parameter_attr_t* parameter = (parameter_attr_t*)(symbol->attr);
	paramspec_t* spec = parameter->param_spec;
	param_type_t param_type = 0;
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
		PERRORN("other parameter type", expr->node);
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
			PERRORN("other attribute type: %s", expr->node, attr_obj->type);
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
		case LOGGROUP_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			break;
		case METHOD_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = NO_TYPE;
			break;
		case ATTRIBUTE_TYPE:
			type = check_attribute_type(symbol, expr);
			break;
		case BANK_TYPE:
		case CONNECT_TYPE:
		case GROUP_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
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
		case STRUCT_TYPE:
			type = ((struct_attr_t*)(symbol->attr))->decl;
			break;
		case BITFIELDS_ELEM_TYPE:
			type = ((bit_element_t*)(symbol->attr))->decl;
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
		if (!symbol)
			symbol = get_symbol_from_banks(node->ident.str);
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
			type->common.categ = INT_T;
			expr->type = type;
		}
		else if (strcmp(str, "i") == 0) {
			object_t* obj = get_current_obj();
			if (obj && obj->is_array) {
				new_int_type(expr);
			} else {
				error("%s no undeclared\n", str);
			}
		}
		else {
			int*a = NULL;
			*a = 1000;
			error("%s no undeclared in template\n", str);
		}
	}

	expr->node = node;
	return expr;
}

#define object_is_array(attr, type) \
	do { \
		if (((type*)attr)->is_array) { \
			return 1; \
		} else { \
			error("subscripted value is neither array nor vector"); \
		} \
	} while (0)

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

static int data_is_array(symbol_t symbol) {
	assert(symbol != NULL);
	int ret_value = 0;
	cdecl_t* attr = symbol->attr;
	if (attr->common.categ == ARRAY_T ||
		attr->common.categ == POINTER_T) {
		ret_value = 1;
	} else {
		ret_value = 0;
	}
	return ret_value;
}

static int check_array_symbol(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	void* attr = symbol->attr;
	int *a = NULL;
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
		case DATA_TYPE:
			return data_is_array(symbol);
			break;
	        case OBJECT_TYPE:
                        return object_array(symbol);
		case PARAMETER_TYPE:
			attr = symbol->attr;
			if ((((param_value_t*)attr)->is_original == 0) &&
				(((param_value_t*)attr)->type == PARAM_TYPE_LIST)) {
				return 1;
			}
			fprintf(stderr, "symbol'%s' not array\n", symbol->name);
			return 0;
			break;
		case PARAM_TYPE:
			attr = symbol->attr;
			if (((((params_t*)attr)->decl->common.categ) == NO_TYPE) ||
				((((params_t*)attr)->decl->common.categ) == POINTER_T) ||
				((((params_t*)attr)->decl->common.categ) == ARRAY_T) ||
				((((params_t*)attr)->decl->common.categ) == PARAMETER_TYPE)) {
				return 1;
			}
			break;
		default:
			error("symbol'%s' not array", symbol->name);
			break;
	}

	return 0;
}

static symtab_t get_symbol_table_from_template(symtab_t table, const char* name) {
	assert(table != NULL); assert(name != NULL);
	check_undef_template(table);
	symtab_t ref_table = NULL;
	struct template_table* temp = table->template_table;
	symbol_t sym = NULL;
	while(temp) {
		sym = defined_symbol(temp->table, name, 0);
		if (sym) {
			ref_table = get_symbol_table(temp->table, sym);
			if (ref_table) {
				return ref_table;
			}
		}
		if (temp->table->template_table || temp->table->undef_temp) {
			ref_table = get_symbol_table_from_template(temp->table, name);
			if (ref_table) {
				return ref_table;
			}
		}
		temp = temp->next;
	}

	return ref_table;
}

symbol_t get_symbol_from_templates(symtab_t table, const char* name, int type) {
	assert(table != NULL); assert(name != NULL);
	symtab_t ref_table = NULL;
	symbol_t sym = NULL;
	struct template_table* temp = table->template_table;
	while(temp) {
		sym = defined_symbol(temp->table, name, type);
		if (sym) {
			break;
		}
		if (temp->table->template_table || temp->table->undef_temp) {
			sym = get_symbol_from_templates(temp->table, name, type);
			if (sym) {
				break;
			}
		}
		temp = temp->next;
	}
	return sym;
}

symbol_t get_symbol_from_template(symtab_t table, const char* parent_name, const char* name) {
	assert(table != NULL); assert(parent_name != NULL); assert(name != NULL);
	if (table->table_num == 0)
		table = table->sibling;
	check_undef_template(table);
	symtab_t ref_table = NULL;
	struct template_table* temp = table->template_table;
	symbol_t parent_sym = NULL;
	symbol_t child_sym = NULL;
	while (temp) {
		parent_sym = defined_symbol(temp->table, parent_name, 0);
		if (parent_sym) {
			ref_table = get_symbol_table(temp->table, parent_sym);
			if (ref_table && (ref_table != (void*)(0xFFFFFFFF))) {
				child_sym = symbol_find_notype(ref_table, name);
				if (child_sym) {
					return child_sym;
				}
			}
		}
		if (temp->table->template_table || temp->table->undef_temp) {
			ref_table = get_symbol_table_from_template(temp->table, parent_name);
			if (ref_table && (ref_table != (void*)(0xFFFFFFFF))) {
				child_sym = symbol_find_notype(ref_table, name);
				if (child_sym) {
					return child_sym;
				}
			}
		}
		temp = temp->next;
	}
	if (temp == NULL) {
		symbol_t symbol = symbol_find_notype(table, parent_name);
		table = get_symbol_table(table, symbol);
		table = table->parent;
		if (table)
			child_sym = get_symbol_from_template(table, parent_name, name);
	}

	return child_sym;
}

static int is_param_connect(symbol_t symbol) {
	assert(symbol != NULL);
	int ret = 0;
	symbol_t sym = get_symbol_from_root_table(symbol->name, CONNECT_TYPE);
	ret = (sym != NULL) ? 1 : 0;

	return ret;
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
	symbol = (symbol == NULL) ? get_symbol_from_banks(tmp->name) : symbol;
	while (tmp->next) {
		if ((symbol->type == INTERFACE_TYPE) ||
			(symbol->type == OBJECT_TYPE && !strcmp(((object_t*)(symbol->attr))->obj_type, "interface"))) {
			tmp->name = get_interface_name(symbol->name);
			//symbol = get_symbol_from_root_table(tmp->name, 0);
			symbol = symbol_find_notype(table, tmp->name);
			symbol = (symbol == NULL) ? get_symbol_from_root_table(tmp->name, 0) : symbol;
		}
		if (symbol != NULL) {
			ref_table = get_symbol_table(table, symbol);
			ref_table = (ref_table == NULL) ? get_symbol_table_from_template(table, tmp->name) : ref_table;
			if (tmp->is_array) {
				check_array_symbol(table, symbol);
			}
		}
		else {
			PERRORN("Undefined reference '%s'", expr->node, tmp->name);
			break;
		}

		if (ref_table == (void*)(0xFFFFFFFF)) {
			expr->type->common.categ = NO_TYPE;
			return expr;
		}
		else if (ref_table == NULL) {
			PERRORN("'%s' is not struct or dml object", expr->node, tmp->name);
			break;
		}


		const char* obj_type = NULL;
		if (symbol->type == OBJECT_TYPE) {
			obj_type = ((object_t*)(symbol->attr))->obj_type;
		}
		ref_symbol = symbol_find_notype(ref_table, tmp->next->name);
		ref_symbol = (ref_symbol == NULL) ? get_symbol_from_template(table, tmp->name, tmp->next->name) : ref_symbol;
		if ((symbol->type == CONNECT_TYPE && !strcmp(tmp->next->name, "obj")) ||
			((symbol->type == OBJECT_TYPE) && !(strcmp(obj_type, "connect")) && (!strcmp(tmp->next->name, "obj")))||
			(!strcmp(tmp->next->name, "obj") && is_param_connect(symbol))) {
			cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			type->common.size = sizeof(int) * 8;
			expr->type = type;
			tmp = tmp->next;
			if (tmp->next) {
				PERRORN("Undefined reference '%s' Line: %d", expr->node, tmp->next->name, __LINE__);
			}
			return expr;
		}
		else if (ref_symbol == NULL) {
			PERRORN("Undefined reference '%s' Line: %d", expr->node, tmp->next->name, __LINE__);
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
		PERRORN("the last elment about compent can not be interface", expr->node);
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

	cdecl_t* type = parse_typeoparg(node->sizeoftype.typeoparg, table);

	if (record_type(type) == 0) {
		PERRORN("sizeoftype not record type", node);
	}
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;

	return expr;
}

expr_t* check_new_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	PERRORN("Pay attention: not implement the new expression", node);

	expr->node = node;
	return expr;
}

void check_func_param(tree_t* node, symtab_t table, signature_t* sig, int param_start) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = NULL;
	vector_t* vect = sig->params;
	int argc = sig->params->len;
	param_t* param = NULL;

	int i = param_start;
	while (node) {
		param = (param_t*)(vect->data[i]);
		if ((sig->has_ellipse) && (i >= (argc - 1))) {
			return;
		}
		expr = check_expr(node, table);
		if (both_scalar_type(param->ty, expr->type) ||
				(both_array_type(param->ty, expr->type)) ||
				(is_same_type(param->ty, expr->type)) ||
				(is_parameter_type(expr->type) && is_scalar_type(param->ty)) ||
				(is_no_type(param->ty) || is_no_type(expr->type))) {
			node = node->common.sibling;
			i++;
		}
		else {
			error("incompatible type for argument \n");
			break;
		}
	}

	return;
}

extern object_t* get_current_obj();
static int first_param_is_obj(signature_t* sig) {
	if (sig == NULL)
		return 0;
	vector_t* vect = sig->params;
	param_t* param = vect->data[0];
	cdecl_t* type = param->ty;
	if (type->common.categ == POINTER_T) {
		cdecl_t* new_type = type->common.bty;
		object_t* obj = get_current_obj();
		if ((new_type->typedef_name) && !strcmp(new_type->typedef_name, "conf_object_t")) {
			if (!strcmp(obj->obj_type, "attribute")) {
				return 0;
			} else {
				return 1;
			}
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
	int arg_num = 0;
	if (func->type->common.categ == FUNCTION_T) {
		sig = func->type->function.sig;
		expr->type = func->type->common.bty;
		if (sig)
			argc = sig->params->len;
		arg_num = get_param_num(node->expr_brack.expr_in_brack);
	}
	else if (func->type->common.categ == POINTER_T) {
		sig = func->type->common.bty->function.sig;
		cdecl_t* bty = func->type->common.bty;
		expr->type = bty->common.bty;
		if (sig) {
			argc = sig->params->len;
			if (func->is_obj && first_param_is_obj(sig)) {
				arg_num = get_param_num(node->expr_brack.expr_in_brack);
				if (argc != arg_num) {
					argc -= 1;
					param_start = 1;
				}
			}
			else {
				arg_num = get_param_num(node->expr_brack.expr_in_brack);
			}
		}
	}
	else {
		cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
		type->common.categ = NO_TYPE;
		expr->type = type;
	}
	object_t* obj = get_current_obj();
	if (obj && !strcmp(obj->obj_type, "attribute")) {
		if ((func->type->common.categ == POINTER_T) &&
			(argc - arg_num == 1)) {
			argc -= 1;
			param_start = 1;
		}
	}
	DEBUG_EXPR("func name: %s, argc: %d, argnum: %d has_ellipse: %d\n", func->type->var_name, argc, arg_num, sig->has_ellipse);
	if ((arg_num != argc) && (sig->has_ellipse == 0)) {
		PERRORN("too %s arguments to function", node,
				(arg_num < argc) ? "few": "many");
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
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = NO_TYPE;
	type = array_of(type);
	expr->type = type;

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
	if ((e2 && !is_int_type(expr->type)) && !is_int_type(e1->type) &&
			!is_no_type(expr->type) && !is_no_type(e1->type) &&
			!is_parameter_type(expr->type) && !is_parameter_type(e1->type)) {
		PERRORN("the bit slicing must be int type", node);
	}
	if (e2 && (!is_int_type(e2->type) && !is_no_type(e2->type) &&
			is_no_type(e2->type) && !is_parameter_type(e2->type))) {
		PERRORN("the bit slicing must be int type", node);
	}

	tree_t* endian = node->bit_slic.endian;
	if (endian) {
		const char* str = endian->ident.str;
		if ((strcmp(str, "le") != 0) || (strcmp(str, "be") != 0)) {
			PERRORN("unknow bitorde: %s", node, str);
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
		check_expr(node, table);
		node = node->common.sibling;
	}
	node = expr_node;

	return ;
}
