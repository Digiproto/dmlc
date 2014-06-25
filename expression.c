/*
 * expression.c:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
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
#include "expression.h"
#include "symbol.h"
#include "tree.h"
#include "types.h"
#include "ast.h"
#include "symbol-common.h"
#include "debug_color.h"
#include "gen_common.h"
#include "info_output.h"
#include "chk_common.h"

/**
 * @brief common_decl_copy : copy the type of identifier to tree node
 *
 * @param dest : the type of tree node
 * @param src : the type of identifier
 *
 * @return : type of tree node
 */
static cdecl_t* common_decl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL); assert(src != NULL);
	dest->common.categ = src->common.categ;
	dest->common.qual = src->common.qual;
	dest->common.is_extern = src->common.is_extern;
	dest->common.is_typedef = src->common.is_typedef;
	dest->common.is_static = src->common.is_static;
	dest->common.is_local = src->common.is_local;
	dest->common.no_decalare = src->common.no_decalare;
	dest->common.size = src->common.size;
	dest->common.bty = src->common.bty;
	dest->common.drv = src->common.drv;
	dest->typedef_name = src->typedef_name;
	dest->var_name = src->var_name;
	return dest;
}

/**
 * @brief struct_decl_copy : copy the struct type to another place
 *
 * @param dest : destination to store identifier type
 * @param src : type of identifier
 *
 * @return : destination to store struct identifier type
 */
static cdecl_t* struct_decl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL); assert(src != NULL);
	dest = common_decl_copy(dest, src);
	dest->struc.is_defined = src->struc.is_defined;
	dest->struc.id = src->struc.id;
	dest->struc.elem = src->struc.elem;
	dest->struc.table = src->struc.table;

	return dest;
}

/**
 * @brief layout_decl_copy : copy the layout type to another place
 *
 * @param dest : destination to store identifier type
 * @param src : type of identifier
 *
 * @return : destination to store layout identifier type
 */
static cdecl_t* layout_decl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL); assert(src != NULL);
	dest = common_decl_copy(dest, src);
	dest->layout.is_defined = src->layout.is_defined;
	dest->layout.id = src->layout.id;
	dest->layout.bitorder = src->layout.bitorder;
	dest->layout.elem = src->layout.elem;
	dest->layout.table = src->layout.table;

	return dest;
}

/**
 * @brief bitfields_decl_copy : copy the bitfields type to another place
 *
 * @param dest : destination to store bitfields identifier type
 * @param src : type of bitfields identifier
 *
 * @return : destination to store bitfields identifier type
 */
static cdecl_t* bitfields_decl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL); assert(src != NULL);
	dest = common_decl_copy(dest, src);
	dest->bitfields.is_defined = src->bitfields.is_defined;
	dest->bitfields.bit_size = src->bitfields.bit_size;
	dest->bitfields.id = src->bitfields.id;
	dest->bitfields.elem = src->bitfields.elem;
	dest->bitfields.table = src->bitfields.table;

	return dest;
}

/**
 * @brief function_decl_copy : copy the function type to another place
 *
 * @param dest : destination to store function identifier type
 * @param src : type of function identifier
 *
 * @return : destination to store function identifier type
 */
static cdecl_t* function_decl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL); assert(src != NULL);
	dest = common_decl_copy(dest, src);
	dest->function.sig = src->function.sig;

	return dest;
}

/**
 * @brief cdecl_copy : copy the identifier type to tree node
 *
 * @param dest : the destination to copy type
 * @param src : the source of copying type
 *
 * @return : the destination of copying type
 */
static cdecl_t* cdecl_copy(cdecl_t* dest, cdecl_t* src) {
	assert(dest != NULL && src != NULL);
	if (src->common.categ == STRUCT_T) {
		dest = struct_decl_copy(dest, src);
	} else if (src->common.categ == LAYOUT_T) {
		dest = layout_decl_copy(dest, src);
	} else if (src->common.categ == BITFIELDS_T) {
		dest = bitfields_decl_copy(dest, src);
	} else if (src->common.categ == FUNCTION_T){
		dest = function_decl_copy(dest, src);
	} else {
		dest = common_decl_copy(dest, src);
	}
	return dest;
}

/**
 * @brief expression_type_copy : copy the identifier type to epression
 *
 * @param dest : the destination to copy type
 * @param src : the source of copying type
 *
 * @return : the destination of copying type
 */
cdecl_t* expression_type_copy(void* dest, cdecl_t* src) {
	assert(src != NULL);
	dest = gdml_zmalloc(sizeof(cdecl_t));
	dest = cdecl_copy(dest, src);

	return dest;
}

/**
 * @brief set_current_obj : set the value about current object
 *
 * @param obj : the object to be assigned to current object
 */
void set_current_obj(object_t* obj) {
	if (OBJ == NULL) {
		OBJ = (obj_ref_t*)gdml_zmalloc(sizeof(obj_ref_t));
	}
	OBJ->obj = obj;
}

/**
 * @brief set_obj_array : set the object is an array
 */
void set_obj_array() {
	object_t* obj = get_current_obj();
	obj->is_array = 1;
	return;
}

/**
 * @brief get_symbol_from_root_table : get symbol from root table
 *
 * @param name : symbol name will be get
 * @param type : type of symbol
 *
 * @return : pointer of symbol
 */
symbol_t get_symbol_from_root_table(const char* name, type_t type) {
	assert(name != NULL);
	symtab_t root_table = get_root_table();
	symbol_t symbol = NULL;
	/*when the type is 0, will find symbol without type */
	if (type == 0) {
		symbol = symbol_find_notype(root_table, name);
	}
	else {
		symbol = symbol_find_curr(root_table, name, type);
	}

	return symbol;
}

extern symtab_t root_table;
/**
 * @brief get_device : get symbol from banks
 *
 * @return : pointer to symbol
 */
extern device_t* get_device();
symbol_t get_symbol_from_banks(const char* name) {
	assert(name != NULL);
	struct list_head *p;
	object_t *tmp;
	bank_attr_t* attr = NULL;
	symbol_t symbol = NULL;
	device_t* dev = get_device();
	fprintf(stderr, "dev %p\n", dev);
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		//symbol = defined_symbol(tmp->symtab->sibling, name, 0);
		if(!strcmp(name,  tmp->name)) {
			symbol = symbol_find(root_table, name, BANK_TYPE);		
		} else {
			symbol = defined_symbol(tmp->symtab->sibling, name, 0);
		}
		fprintf(stderr, "bank name %s, symbol %p\n", tmp->name, symbol);
		//symbol = defined_symbol(tmp->symtab, name, OBJECT_T);
		if (symbol)
			return symbol;
	}
	return symbol;
}

static int check_dml_obj_refer(tree_t* node, symtab_t table);

/**
 * @brief get_bit_slic_ref : get the reference of bit slicing
 *
 * @param node : tree node about bit slicing
 * @param ref : reference struct
 * @param expr : expressions information
 * @param table : table about bit slicing
 *
 * @return : reference about bit slicing
 */
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
				PERRORN("1reference to unknown object '%s', table_num: %d, line: %d", node,
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

/**
 * @brief get_reference : get reference about expreesion
 *
 * @param node : node about reference
 * @param ref : pointer to reference information struct
 * @param expr : expression information
 * @param table : table about reference expression
 *
 * @return : reference about expression
 */
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
				//PERRORN("reference to unknown object '%s'", node, ident_node->ident.str);
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
		case UNARY_TYPE:
			free(new);	
			new = get_reference(node->unary.expr, ref, expr, table);
			break;
		default:
			fprintf(stderr, "xxx\n");
			PERRORN("other node type: %s", node, node->common.name);
			/* FIXME: handle the error */
			break;
	}
	return new;
}

/**
 * @brief print_reference : print the reference name
 *
 * @param ref : pointer to reference
 */
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

/**
 * @brief get_tempalte_table : get table about template
 *
 * @param name : template name
 *
 * @return : pointer to template table
 */
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
		error("Can not find temmplate '%s'\n", name);
	}
	return NULL;
}

extern symtab_t get_obj_param_table(symtab_t table, symbol_t symbol);

/**
 * @brief get_default_symbol_tale : get table about something default symbol of simics
 *
 * @param table : table about using symbol
 * @param symbol : pointer of symbol
 *
 * @return : table about default symbol
 */
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

/**
 * @brief get_object_table : get table of object
 *
 * @param symbol : symbol about object
 *
 * @return : table of object
 */
symtab_t get_object_table(symbol_t symbol) {
	assert(symbol != NULL);
	object_t* obj = symbol->attr;
	tree_t* node = obj->node;
	return obj->symtab;
}

/**
 * @brief get_record_table : get table of record type
 *
 * @param type : record type
 *
 * @return : table of record
 */
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

/**
 * @brief get_data_table : get table about data
 *
 * @param symbol : symbol of data
 *
 * @return : table of data
 */
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

symtab_t get_data_table2(object_t *obj) {
	tree_t *node;

	node = obj->node;
	cdecl_t* type = node->common.cdecl;
	symtab_t table = NULL;
	
	/*
	if (type->common.categ == POINTER_T) {
		symbol_t new_sym = (symbol_t)gdml_zmalloc(sizeof(struct symbol));
		new_sym->name = symbol->name;
		new_sym->attr = type->common.bty;
		table = get_data_table(new_sym);
		new_sym->name = NULL; new_sym->attr = NULL;
		free(new_sym);
	}
	else*/ if (record_type(type)) {
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
			error("other data '%s' type: %d\n", obj->name, type->common.categ);
		}
	}
	else {
		error("other data '%s' type: %d\n", obj->name, type->common.categ);
	}

	return table ;
}
/**
 * @brief get_point_table : get table about pointer type
 *
 * @param symbol : symbol about pointer variable
 *
 * @return : tabel of pointer type
 */
static symtab_t get_point_table(symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	cdecl_t* attr = symbol->attr;
	cdecl_t* type = attr->common.bty;
	if (record_type(type)) {
		get_record_table(type);
	} else {
		table = (void*)0XFFFFFFFF;
	}

	return table;
}

/**
 * @brief get_array_table : get table about array type
 *
 * @param symbol : symbol of array
 *
 * @return : table about array type
 */
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
		table = (void*)0XFFFFFFFF;
	}
	return table;
}

/**
 * @brief get_typedef_table : get table of typedef type
 *
 * @param symbol : symbol about typedef
 *
 * @return : table of typedef symbol
 */
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

/**
 * @brief get_select_table : get table about select variable
 *
 * @param sym_tab : table of block that used the select symbol
 * @param symbol : symbol about select variable
 *
 * @return : table about select symbol
 */
static symtab_t get_select_table(symtab_t sym_tab, symbol_t symbol) {
	assert(symbol != NULL);
	symtab_t table = NULL;
	select_attr_t* attr = symbol->attr;
	if (!strcmp(symbol->name, "bank") && (attr->type == PARAMETER_TYPE)) {
		table = get_default_symbol_tale(sym_tab, symbol);
	} else {
		table = (void*)0XFFFFFFFF;
	}
	return table;
}

extern symtab_t get_symbol_table(symtab_t sym_tab, symbol_t symbol);

/**
 * @brief get_param_table : get table about method parameter
 *
 * @param sym_tab : table of method
 * @param symbol : symbol about method parameter
 *
 * @return : table of method parameter
 */
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

/**
 * @brief get_symbol_table : get table about symbol
 *
 * @param sym_tab : table of block that use symbol
 * @param symbol : symbol of being used
 *
 * @return : table about symbol
 */
symtab_t get_symbol_table(symtab_t sym_tab, symbol_t symbol) {
	assert(symbol != NULL);

	symtab_t table = NULL;
	void* attr = symbol->attr;

	fprintf(stderr, "symbol type %d, DATA TYPE %d, object type %d\n", symbol->type, DATA_TYPE, OBJECT_TYPE);
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
		case OBJECT_TYPE: {
			object_t *obj = symbol->attr;	
			if(!strcmp(obj->obj_type, "data")) {	
				table = get_data_table2(obj);	
			} else {
				table = get_object_table(symbol);
			}
			break;
		}
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

/**
 * @brief get_interface_name : get defined interface name about dml interface
 *
 * @param name : name of dml interface
 *
 * @return : name of defined interface name
 */
static char* get_interface_name(const char* name) {
	assert(name != NULL);
	int name_len = strlen(name) + strlen("_interface_t") + 1;
	char* new_name = (char*)gdml_zmalloc(name_len);
	strcpy(new_name, name);
	strcat(new_name, "_interface_t");

	return new_name;
}

/**
 * @brief parse_log_args : check log args expression about dml
 *
 * @param node : node about log args
 * @param table : table of block that uses log
 *
 * @return : expression information about log args
 */
expr_t* parse_log_args(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	assert(table != NULL);

	tree_t* arg_node = node;

	expr_t* expr = check_expr(node, table);
	if (expr->no_defined) {
		return expr;
	}

	while ((arg_node->common.sibling) != NULL) {
		arg_node = arg_node->common.sibling;
		expr = check_expr(arg_node, table);
		if (expr->no_defined) {
			return expr;
		}
	}

	return expr;
}

/**
 * @brief unqual : get the unqualified type of quality
 *
 * @param type : type of quality
 *
 * @return : unqualified type of quality
 */
cdecl_t* unqual(cdecl_t* type) {
	if (type->common.qual)
		type = type->common.bty;
	return type;
}

/**
 * @brief adjust : adjust expression type
 *
 * @param expr : expression to be checked
 * @param rvalue : the expression is right element of calculation
 *
 * @return : expression information
 */
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

/**
 * @brief can_modify : check one expression can be modify or not
 *
 * @param expr : expression information
 *
 * @return : 1 - can be modified
 *			0 - can not be modified
 */
static int can_modify(expr_t* expr) {
	assert(expr != NULL);

	if (expr->lvalue) {
		if ((expr->type->common.qual & CONST_QUAL) || expr->is_const) {
			return 0;
		}
	}

	return 1;
}

/**
 * @brief cal_muti_const_value : calculate the constant value about muti-operation
 *
 * @param expr : expression about muti-operation
 * @param op : muti-operator
 *
 * @return : the value and type about expression
 */
#define cal_muti_const_value(expr, op) \
		do { \
			if (both_int_type(expr->kids[0]->type, expr->kids[1]->type)) { \
				expr->type->common.categ = (expr->kids[0]->type->common.categ > expr->kids[1]->type->common.categ) ? expr->kids[0]->type->common.categ : expr->kids[1]->type->common.categ; \
				expr->val->int_v.value = ((expr->kids[0]->val->int_v.value) op (expr->kids[1]->val->int_v.value)); \
			} \
			else if (is_int_type(expr->kids[0]->type) && is_double_type(expr->kids[1]->type)) { \
				expr->type->common.categ = DOUBLE_T; \
				expr->val->d = ((expr->kids[0]->val->int_v.value) op (expr->kids[1]->val->d)); \
			} \
			else if (is_double_type(expr->kids[0]->type) && is_int_type(expr->kids[1]->type)) { \
				expr->type->common.categ = DOUBLE_T; \
				expr->val->d = ((expr->kids[0]->val->d) op (expr->kids[1]->val->int_v.value));  \
			} \
			else { \
				expr->type->common.categ = DOUBLE_T; \
				expr->val->d = ((expr->kids[0]->val->d) op (expr->kids[1]->val->d)); \
			} \
		} while (0)

/**
 * @brief cal_int_const : calculate the constant value about expression with int type
 *
 * @param expr : the expression to be calculated
 * @param op : the operator about expression
 *
 * @return : value and type about expression
 */
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

/**
 * @brief cal_unary : calculate the constant value about unary operation
 *
 * @param expr : expression to be calculated
 * @param op : operator about unary
 *
 * @return : value and type about expression
 */
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

/**
 * @brief cal_unary_int : calculate the constant value about int unary operation
 *
 * @param expr : expression to be calculated
 * @param op : unary operator
 *
 * @return : value and type about expression
 */
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

/**
 * @brief cal_const_value : calculate the constant value about expression
 *
 * @param expr : expression to be calculated
 *
 * @return : the type and value about expression
 */
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

/**
 * @brief check_binary_kids : check right and left kids about binary operation
 *
 * @param node : tree node about binary operation
 * @param table : table of block that contains binary operation
 * @param expr : pointer to expression information struct
 *
 * @return : pointer to expression information struct
 */
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

/**
 * @brief expr_to_int : modified the expression to be int
 *
 * @param expr : expression will be modified
 *
 * @return : expression information
 */
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

/**
 * @brief check_no_defiend_expr : check expression has undefined variable
 *
 * @param expr : expression to be checked
 * @param node : tree node about expression
 *
 * @return : expression information
 */
#define check_no_defiend_expr(expr, node) \
	do {	\
		if (expr->no_defined) { \
			expr->node = node; \
			expr->no_defined = 1; \
			return expr; \
		} \
	} while (0)

/**
 * @brief check_kids_no_defiend : check right and left kids of expression have  undefined variable
 *
 * @param expr : expression to be checked
 * @param node : tree node about expression
 *
 * @return : expression information
 */
#define check_kids_no_defiend(expr, node) \
		do { \
			if (expr->kids[0]->no_defined || expr->kids[1]->no_defined) { \
				expr->no_defined = 1; \
				expr->node = node; \
				return expr; \
			} \
		}while(0)

/**
 * @brief check_logical_expr : check logical operation expression
 *
 * @param node : tree node about logical expression
 * @param table : table about block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : expression struct
 */
expr_t* check_logical_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL);
	fprintf(stderr, "file %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_bitwise_expr : check bitwise expression
 *
 * @param node : tree node about bitwise expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_bitwise_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_equality_expr : check equality expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct that contains expression
 *
 * @return : pointer to expression struct
 */
expr_t* check_equality_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	cdecl_t* type1 = NULL;
	cdecl_t* type2 = NULL;

	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
	type1 = expr->kids[0]->type;
	type2 = expr->kids[1]->type;
	type1 = (type1->common.categ == ARRAY_T) ? type1->common.bty : type1;
	type2 = (type2->common.categ == ARRAY_T) ? type2->common.bty : type2;
	if (both_arith_type(type1, type2)) {
		expr = cal_const_value(expr);
		expr_to_int(expr);
		return expr;
	}
	if(is_string_type(type1) ^ is_string_type(type2)) {
		fprintf(stderr, "expect both string type, file %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
		fprintf(stderr, "type1 %d, type2 %d\n", type1->common.categ, type2->common.categ);
		fprintf(stderr, "string CMP %d\n", STRING_T);
	} else if(is_string_type(type1)){
		expr_t *expr1, *expr2;
		expr1 = expr->kids[0];
		expr2 = expr->kids[1];

		if(!(expr1->is_const && expr2->is_const)) {
			fprintf(stderr, "%s:%d: ", node->common.location.file->name, node->common.location.first_line);
			fprintf(stderr, "error: comparison with string literal results in unspecified behavior\n");
			exit(-1);
		}
		const char *name1 = expr1->val->p;
		const char *name2 = expr2->val->p;
		cdecl_t *type = gdml_zmalloc(sizeof *type);
		value_t *val = gdml_zmalloc(sizeof *val);
		expr->is_const = 1;
		type->common.categ = BOOL_T;
		expr->val = val;
		expr->type = type;
		if(!strcmp(name1, name2)) {
			val->int_v.value = 1;
		} else {
			val->int_v.value = 0;
		}
		if(node->binary.type == NE_OP_TYPE) {
			val->int_v.value = !val->int_v.value;
		}
		return expr;
	}
		
	tree_t *left = node->binary.left;
	if(left->common.type == COMPONENT_TYPE && !strcmp(left->component.ident->ident.str, "name")) {
		tree_t *right = node->binary.right;
		if(right->common.type == CONST_STRING_TYPE) {
			cdecl_t *type = gdml_zmalloc(sizeof *type);
			value_t *val = gdml_zmalloc(sizeof *val);
			expr->is_const = 1;
			type->common.categ = BOOL_T;
			expr->val = val;
			val->int_v.value = 0;
			return expr;
		}
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

/**
 * @brief check_relation_expr : check relation expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_relation_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_shift_expr: check shift expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_shift_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_add_expr: check add expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_add_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check add expr\n");
	EXPR_TRACE("check binary kids\n");
	expr = check_binary_kids(node, table, expr);
	EXPR_TRACE("check binary kids end\n");
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_sub_expr: check sub expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_sub_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief check_multiplicative_expr: check mutiplicative expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_multiplicative_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr = check_binary_kids(node, table, expr);
	check_kids_no_defiend(expr, node);
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

/**
 * @brief two_type_compatible :check two type can be compatible
 *
 * @param type1 : the first type to be checked
 * @param type2 : the second type to be checked
 *
 * @return : 1 - two type can be compatible
 *			0 - two type can not be compatible
 */
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
		return 1;
	}
	if (is_no_type(type1) || is_no_type(type2))
		return 1;
	if (is_array_type(type1) || is_array_type(type2))
		return array_type_compatible(type1, type2);

	return 0;
}

/**
 * @brief array_type_compatible : check array type can be compatible with other type
 *
 * @param type1 : the first type to be checked
 * @param type2 : the second type to be checked
 *
 * @return : 1 - can be compatible
 *			0 - can not be compatible
 */
static int array_type_compatible(cdecl_t* type1, cdecl_t* type2) {
	assert(type1 != NULL); assert(type2 != NULL);
	cdecl_t* tmp_type1 = NULL;
	cdecl_t* tmp_type2 = NULL;
	tmp_type1 = (type1->common.categ == ARRAY_T) ? type1->common.bty : type1;
	tmp_type2 = (type2->common.categ == ARRAY_T) ? type2->common.bty : type2;
	return two_type_compatible(tmp_type1, tmp_type2);
}

/**
 * @brief can_assign : check assignment expression can be assigned
 *
 * @param l_expr : left expression of assignment
 * @param r_expr : right expression of assignment
 *
 * @return : 1 - can be assigned
 *			0 - can not be assigned
 */
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

/**
 * @brief check_assign_expr : check assignment expression
 *
 * @param node : tree node about assign expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_assign_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	expr->op = node->expr_assign.type;
	if (node->expr_assign.type != EXPR_ASSIGN_TYPE) {
		EXPR_TRACE("in op= expr\n");
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
		EXPR_TRACE("end in op = expr\n");
	}
	else {
		EXPR_TRACE("check direct assignment\n");
		expr->kids[0] = check_expr(node->expr_assign.left, table);
		expr->kids[1] = check_expr(node->expr_assign.right, table);
		check_kids_no_defiend(expr, node);
	}
	check_no_defiend_expr(expr, node);

	EXPR_TRACE("check modifiable\n");
	if (!can_modify(expr->kids[0])) {
		error("lvalue required as left operand of assignment\n");
	}
	EXPR_TRACE("check type compatiable\n");
	if (!can_assign(expr->kids[0], expr->kids[1])) {
		error("wrong assignment, left type %d, right %d\n", expr->kids[0]->type->common.categ, expr->kids[1]->type->common.categ);
	}
	expr->type = expr->kids[0]->type;
	node->expr_assign.ty = expression_type_copy(node->expr_assign.ty, expr->type);
	expr->node = node;

	return expr;
}

/**
 * @brief check_binary_expr : check binary expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_binary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL);
	expr->op = node->binary.type;
	EXPR_TRACE("check binary op %s\n", TYPENAME(node->binary.type));
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
	check_no_defiend_expr(expr, node);
	expr->node = node;
	EXPR_TRACE("end check binary op %s\n", TYPENAME(node->binary.type));
	return expr;
}

/**
 * @brief check_ternary_expr: check ternary expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_ternary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	EXPR_TRACE("check ternary expr\n");
	expr_t* cond_expr = check_expr(node->ternary.cond, table);
	if (cond_expr->no_defined) {
		expr->node = node;
		expr->no_defined = 1;
		return expr;
	}
	if (!is_scalar_type(cond_expr->type) &&
		!is_no_type(cond_expr->type) &&
		!is_parameter_type(cond_expr->type)) {
		error("used struct type value where scalar is required\n");
	}

	expr_t* true_expr = check_expr(node->ternary.expr_true, table);
	expr_t* false_expr = check_expr(node->ternary.expr_false, table);
	if (true_expr->no_defined || false_expr->no_defined) {
		expr->node = node;
		expr->no_defined = 1;
		return expr;
	}
	cdecl_t* type1 = true_expr->type;
	cdecl_t* type2 = false_expr->type;

	expr->type = type1->common.categ > type2->common.categ ? type1 : type2;
	expr->node = node;
	if(cond_expr->is_const) {
		if(cond_expr->val->int_v.value) {
			return true_expr;	
		} else {
			return false_expr;
		}
	}
	return expr;
}

static expr_t* check_expression(tree_t* node, symtab_t table, expr_t* expr);
/**
 * @brief check_cast_expr: check cast expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_cast_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check cast expr\n");
	expr = check_expression(node->cast.expr, table, expr);

	cdecl_t* type = parse_ctype_decl(node->cast.ctype, table);
	if (expr->no_defined || type->common.no_decalare) {
		expr->no_defined = 1;
	}
	expr->type = type;
	expr->node = node;
	node->cast.ty = expression_type_copy(node->cast.ty, expr->type);

	return expr;
}

/**
 * @brief check_sizeof_expr: check sizeof expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_sizeof_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check sizeof\n");
	expr = check_expression(node->sizeof_tree.expr, table, expr);
	check_no_defiend_expr(expr, node);
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;
	node->sizeof_tree.ty = expression_type_copy(node->sizeof_tree.ty, expr->type);

	return expr;
}

/**
 * @brief check_unary_expr: check unary expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_unary_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	expr->op = node->unary.type;
	EXPR_TRACE("check unary expr op %s \n", TYPENAME(node->unary.type));
	switch(node->unary.type) {
		case NEGATIVE_TYPE:
		case CONVERT_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
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
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
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
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
			if (expr->kids[0]->is_const) {
				error("lvalue required as unary ‘&’ operand\n");
			}
			else {
				expr->type = pointer_to(expr->kids[0]->type);
				node->common.cdecl = expr->type;
			}
			break;
		case POINTER_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
			if (expr->kids[0]->type->common.categ != POINTER_T) {
				error("invalid type argument of ‘unary *’\n");
			}
			else {
				expr->type = expr->kids[0]->type->common.bty;
			}
			break;
		case DEFINED_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
			expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			expr->val = (value_t *)gdml_zmalloc(sizeof (value_t));
			expr->val->int_v.value = 1;
			expr->is_const = 1;
			expr->type->common.categ = BOOL_T;
			expr->type->common.size = sizeof(int) * 8;
			break;
		case EXPR_TO_STR_TYPE:
			expr->kids[0] = check_expr(node->unary.expr, table);
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
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
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
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
			expr->no_defined = expr->kids[0]->no_defined;
			check_no_defiend_expr(expr, node);
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

	EXPR_TRACE("end check unary expr op %s \n", TYPENAME(node->unary.type));
	expr->node = node;
	return expr;
}

/**
 * @brief check_unary_expr: check expression with constant value
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_const_expr(tree_t* node, expr_t* expr) {
	assert(node != NULL); assert(expr != NULL);
	value_t* value = (value_t*)gdml_zmalloc(sizeof(value_t));
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->is_const = 1;
	expr->type = type;
	EXPR_TRACE("check const expr op %s\n", TYPENAME(node->common.type));
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
			node->int_cst.ty = expression_type_copy(node->int_cst.ty, expr->type);
			break;
		case FLOAT_TYPE:
			expr->type->common.categ = DOUBLE_T;
			expr->type->common.size = sizeof(double) * 8;
			expr->val = value;
			expr->is_const = 1;
			value->d = node->float_cst.value;
			node->float_cst.ty = expression_type_copy(node->float_cst.ty, expr->type);
			break;
		case CONST_STRING_TYPE:
			expr->type->common.categ = STRING_T;
			expr->type->common.size = sizeof(char*) * 8;
			expr->val = value;
			expr->is_const = 1;
			value->p = strndup(node->string.pointer + 1, strlen(node->string.pointer) - 2 );
			node->string.ty = expression_type_copy(node->string.ty, expr->type);
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

	EXPR_TRACE("end check const expr op %s\n", TYPENAME(node->common.type));
	expr->node = node;
	return expr;
}

/**
 * @brief dml_obj_type : check symbol is object or not
 *
 * @param symbol : symbol to be checked
 *
 * @return : object type or 0 for not object type
 */
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

/**
 * @brief find_dml_obj : find object from table
 *
 * @param table : table to find symbol
 * @param name : object name
 *
 * @return : object type
 */
extern object_t *default_bank;
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
				/*
				symbol = get_symbol_from_banks(name);
				if(symbol)
					obj_type = dml_obj_type(symbol);
				*/
			}
			return obj_type;
		}
	}
	else {
		symbol = get_symbol_from_root_table(name, 0);
		if (!symbol) {
			//symbol = get_symbol_from_banks(name);
	                if(default_bank) {
                        symtab_t table = default_bank->symtab;
                        symbol_t sym = defined_symbol(table, name, 0);
                        if(sym)
                        	symbol = sym;;
                	}
		}
		if (symbol)
			obj_type = dml_obj_type(symbol);
		return obj_type;
	}

	return obj_type;
}

/**
 * @brief check_dml_obj_refer : check reference is object or not
 *
 * @param node : tree node about reference
 * @param table : table of block that use object reference
 *
 * @return : 1 - reference is object
 *			0 - reference is not object
 */
static int check_dml_obj_refer(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	int obj_type = 0;
	char* refer_name = (char*)(node->ident.str);
	if (strcmp(refer_name, "this") == 0) {
		//fprintf(stderr, "compare this true\n");
		obj_type = 1;
	}
	else {
		obj_type = find_dml_obj(table, refer_name);
	}

	return obj_type;

}

/**
 * @brief check_quote_expr: check quote expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct about expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_quote_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	tree_t* ident = node->quote.ident;

	EXPR_TRACE("check quote expr \n");
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

	check_no_defiend_expr(expr, node);
	node->quote.ty = expression_type_copy(node->quote.ty, expr->type);
	EXPR_TRACE("end check quote expr \n");
	return expr;
}

/**
 * @brief check_parameter_type : check the type of parameter
 *
 * @param symbol : symbol of parameter
 * @param expr : expression struct to store parameter value and type
 *
 * @return : pointer to expression
 */
cdecl_t* check_parameter_type(symbol_t symbol, expr_t* expr) {
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
		fprintf(stderr, "is orignal\n");
	} else {
		param_type = ((param_value_t*)parameter)->type;
	}

	if (param_type == PARAM_TYPE_NONE || param_type == PARAM_TYPE_LIST ||
		param_type == PARAM_TYPE_REF) {
		fprintf(stderr, "paramxxx check %d, list type %d\n", param_type, PARAM_TYPE_LIST);
		type->common.categ = PARAMETER_TYPE;
	} else if(param_type == PARAM_TYPE_LIST2) {
		type->common.categ = LIST_T;
		type->node = spec->expr_node;
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
				expr->is_const = 1;
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
	//fprintf(stderr, "++++++++++++type %d\n ", type->common.categ);
	return type;
}

/**
 * @brief check_constant_type : check the type of constant
 *
 * @param symbol : symbol of constant
 * @param expr : expression struct to store parameter value and type
 *
 * @return : pointer to expression struct
 */
cdecl_t* check_constant_type(symbol_t symbol, expr_t* expr) {
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

/**
 * @brief check_method_param_type : check type of method parameters
 *
 * @param table : table of method
 * @param symbol : symbol of method parameter
 *
 * @return : pointer to declarantion struct about method parameter
 */
static cdecl_t* check_method_param_type(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	params_t* param = (params_t*)(symbol->attr);
	return param->decl;
}

/**
 * @brief check_foreach_type : check the type of foreach
 *
 * @param symbol : symbol of foreach
 *
 * @return : pointer to foreach symbol type
 */

cdecl_t* check_foreach_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	foreach_attr_t* attr = (foreach_attr_t*)(symbol->attr);
	return attr->expr->type;
}

/**
 * @brief check_register_type : check the type of register
 *
 * @param symbol : symbol of register
 *
 * @return : pointer to register type
 */
static cdecl_t* check_register_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	register_attr_t* attr = symbol->attr;
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = INT_T;
	type->common.size = sizeof(int) * 8;

	return type;
}

/**
 * @brief check_attribute_type : check the type about attribute
 *
 * @param symbol : symbol of attribute
 * @param expr : expression struct that contains attribute expression information
 *
 * @return : pointer to type of attribute
 */
static cdecl_t* check_attribute_type(symbol_t symbol, expr_t* expr) {
	assert(symbol != NULL); assert(expr != NULL);
	attribute_attr_t* attr = (attribute_attr_t*)(symbol->attr);
	attribute_t* attr_obj = (attribute_t*)(attr->attr_obj);
	if(!attr_obj) {
		new_int_type(expr);
		return expr->type;
	}
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

/**
 * @brief get_obj_type : get the type of obejct
 *
 * @param symbol : symbol of object
 * @param table : table of object
 * @param expr : expression struct to store expression calculating information
 *
 * @return : pointer to type of object
 */
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
	if(!strcmp(obj->obj_type, "data")) {
		type = node->common.cdecl;				
	} else {
	    object_attr_t* attr = node->common.attr;
	    symbol_t new_symbol = (symbol_t)gdml_zmalloc(sizeof(struct symbol));
	    new_symbol->name = symbol->name;
	    new_symbol->type = attr->common.obj_type;
	    new_symbol->attr = attr;
	    type = get_common_type(table, new_symbol, expr);
	    free(new_symbol);
	}

	return type;
}

/**
 * @brief get_common_type : get type of symbol
 *
 * @param table : table of block that uses symbol
 * @param symbol: symbol to be gotten type
 * @param expr : expression struct to store expression calculating information
 *
 * @return : type of symbol
 */
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

/**
 * @brief check_ident_expr : check the expression of identifier
 *
 * @param node : tree node about identifier expression
 * @param table : table of block that uses identifier expression
 * @param expr : expression struct to store identifier expression information
 *
 * @return : pointer ot expression struct
 */
expr_t* check_ident_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check ident expr\n");
	const char* str = node->ident.str;

	if (strcmp(str, "this") == 0) {
			cdecl_t *type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			expr->type = type;
			expr->node = node;
			node->ident.ty = expression_type_copy(node->ident.ty, expr->type);
			//fprintf(stderr, "this ~~~~~~~~~~\n");
			return expr;
	}
	symbol_t symbol = symbol_find_notype(table, node->ident.str);
	if (symbol == NULL) {
		symtab_t root_table = get_root_table();
		symbol = symbol_find_notype(root_table, node->ident.str);
		EXPR_TRACE("in root table symbol %p=n", symbol);
		if (!symbol)
			symbol = get_symbol_from_banks(node->ident.str);
	}
	fprintf(stderr, "symbol %p\n", symbol);
	if (symbol != NULL) {
		//fprintf(stderr, "symbol name: %s, type: %d, table %d\n", symbol->name, symbol->type, table->table_num);
		if (is_common_type(symbol->type)) {
			EXPR_TRACE("common symbol type\n");
			expr->type = symbol->attr;
		}
		else {

			EXPR_TRACE("complex symbol type\n");
			expr->type = get_common_type(table, symbol, expr);
		}
	}
	else {
		//fprintf(stderr, "special case\n");
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
		/*
		else if (strcmp(str, "this") == 0) {
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = INT_T;
			expr->type = type;
			fprintf(stderr, "this ~~~~~~~~~~\n");
		} */
		else if (strcmp(str, "i") == 0) {
			object_t* obj = get_current_obj();
			//if (obj && obj->is_array) {
				new_int_type(expr);
			/*} else {
				int *p = NULL;
				*p = 0;
				error("%s no undeclared\n", str);
			}*/
		}
		else {
			if (table->pass_num == 0) {
				expr->no_defined = 1;
			} else {
				int *p = NULL;
				*p = 0;
				error("%s not undeclared in template\n", str);
			}
		}
	}

	expr->node = node;
	//node->ident.ty = expression_type_copy(node->ident.ty, expr->type);
	EXPR_TRACE("end check ident expr\n");
	return expr;
}

/**
 * @brief object_is_array : marc for checking object is array or not
 *
 * @param attr : the attribute of object
 * @param type : type of object
 *
 * @return : 1 - object is array
 *			0 - object is not array
 */
#define object_is_array(attr, type) \
	do { \
		if (((type*)attr)->is_array) { \
			return 1; \
		} else { \
			int *p = NULL; \
			*p = 0; \
			error("subscripted value is neither array nor vector"); \
		} \
	} while (0)

/**
 * @brief object_array : check object is array or not
 *
 * @param symbol : symbol of object
 *
 * @return : 1 - object is array
 *			0 - object is not array
 */
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

/**
 * @brief data_is_array : check data is array or not
 *
 * @param symbol : symbol of data
 *
 * @return : 1 - data is array
 *			0 - data is not array
 */
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

/**
 * @brief check_array_symbol : check symbol is array or not
 *
 * @param table : table of block that uses symbol
 * @param symbol : symbol of checking
 *
 * @return 1 - symbol is array
 *			0 - symbol is not array
 */
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
			//error("symbol'%s' not array", symbol->name);
			break;
	}

	return 0;
}

/**
 * @brief get_symbol_table_from_template : get symbol table from templates of table
 *
 * @param table : table with templates
 * @param name : name of symbol
 *
 * @return : table of symbol
 */
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

/**
 * @brief get_symbol_from_templates : get symbol from templates of table
 *
 * @param table : table with templates
 * @param name : name of symbol
 * @param type : type of symbol
 *
 * @return : pointer to symbol
 */
symbol_t get_symbol_from_templates(symtab_t table, const char* name, int type) {
	assert(table != NULL); assert(name != NULL);
	symtab_t ref_table = NULL;
	symbol_t sym = NULL;
	struct template_table* temp = table->template_table;
	int i = 0;
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
		i++;
		if(i > 20) {
			fprintf(stderr, "table %p\n", table);
			break;
		}
		temp = temp->next;
	}
	return sym;
}

/**
 * @brief get_symbol_from_template : get symbol of reference from templates of table
 *
 * @param table : table with templates
 * @param parent_name : the topper level symbol of reference symbol
 * @param name : symbol of reference
 *
 * @return  : symbol of reference
 */
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

/**
 * @brief is_param_connect : check a symbol is connect type
 *
 * @param symbol : symbol to be checked
 *
 * @return : 1 - connect type
 *			0 - not connect type
 */
static int is_param_connect(symbol_t symbol) {
	assert(symbol != NULL);
	int ret = 0;
	symbol_t sym = get_symbol_from_root_table(symbol->name, CONNECT_TYPE);
	ret = (sym != NULL) ? 1 : 0;

	return ret;
}

/**
 * @brief check_refer : check reference of expression
 *
 * @param table : table of block that contains reference
 * @param ref : struct contains reference
 * @param expr : expression struct that store the expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_refer(symtab_t table, reference_t* ref, expr_t* expr) {
	assert(table != NULL); assert(ref != NULL); assert(expr != NULL);

	symbol_t symbol = NULL;
	symbol_t ref_symbol = NULL;
	symtab_t ref_table = NULL;
	reference_t* tmp = ref;

	if (table->no_check == 1) {
		fprintf(stderr, "no check \n");
		expr->type->common.categ = INT_T;
		return expr;
	}
	if(!strcmp(tmp->name, "arl_table")) {
		fprintf(stderr, "+++++++++++++try to find arl table \n");
	}
	symbol = symbol_find_notype(table, tmp->name);
	if(!strcmp(tmp->name, "arl_table")) {
		fprintf(stderr, "+++++++++++++try to find arl table %p, name %s, type %d\n", symbol, symbol->name, symbol->type);
	}
	fprintf(stderr, "symbol name %s\n", tmp->name);
	symbol = (symbol == NULL) ? get_symbol_from_banks(tmp->name) : symbol;
	while (tmp->next) {
		if ((symbol->type == INTERFACE_TYPE) ||
			(symbol->type == OBJECT_TYPE && !strcmp(((object_t*)(symbol->attr))->obj_type, "interface"))) {
			tmp->name = get_interface_name(symbol->name);
			//symbol = get_symbol_from_root_table(tmp->name, 0);
			symbol = symbol_find_notype(table, tmp->name);
			symbol = (symbol == NULL) ? get_symbol_from_root_table(tmp->name, 0) : symbol;
		}
		fprintf(stderr, "symbol %p\n", symbol);	
		if (symbol != NULL) {
			ref_table = get_symbol_table(table, symbol);
			ref_table = (ref_table == NULL) ? get_symbol_table_from_template(table, tmp->name) : ref_table;
			if (tmp->is_array) {
				check_array_symbol(table, symbol);
			}
		}
		else {
			if (table->pass_num == 0) {
				expr->no_defined = 1;
			}
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
		fprintf(stderr, "ref name %s\n", tmp->next->name);
		const char *wired = tmp->next->name;
		if(!strcmp(wired, "c") || !strcmp(wired, "rma")) {
			fprintf(stderr, "%s, ref_table %p, table %p\n", wired, ref_table, table);
			print_all_symbol(ref_table);
			print_all_symbol(table);
		}
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
			if (table->pass_num == 0) {
				expr->no_defined = 1;
			}
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

/**
 * @brief check_component_expr : check expression with component(-> or .)
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct contains expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_component_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check component expr \n");
	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	reference_t* reference = (reference_t*)gdml_zmalloc(sizeof(reference_t));
	if (node->component.type == COMPONENT_POINTER_TYPE) {
		reference->is_pointer = 1;
	}
	tree_t* ident = node->component.ident;
	reference->name = ident->ident.str;
	reference = get_reference(node->component.expr, reference, expr, table);
	print_reference(reference);
	fprintf(stderr, "before ref check, file %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
	expr = check_refer(table, reference, expr);
	fprintf(stderr, "after ref check\n");
	check_no_defiend_expr(expr, node);
	expr->node = node;
	node->component.ty = expression_type_copy(node->component.ty, expr->type);
	EXPR_TRACE("end check component expr \n");
	return expr;
}


/**
 * @brief check_sizeoftype_expr : check sizeoftype expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct contains expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_sizeoftype_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	EXPR_TRACE("check sizeof expr\n");
	cdecl_t* type = parse_typeoparg(node->sizeoftype.typeoparg, table);
	expr->no_defined = type->common.no_decalare;
	check_no_defiend_expr(expr, node);

	expr->type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	expr->type->common.categ = INT_T;
	expr->type->common.size = sizeof(int) * 8;
	expr->node = node;
	node->sizeoftype.ty = expression_type_copy(node->sizeoftype.ty, expr->type);

	EXPR_TRACE("end check sizeof expr\n");
	return expr;
}

/**
 * @brief check_new_expr : check new expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct contains expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_new_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check new expr\n");
	cdecl_t* type  = parse_ctype_decl(node->new_tree.type, table);
	//PERRORN("Pay attention: not check the new expression with testcase", node);

	expr->type = type;
	expr->node = node;
	node->new_tree.ty = expression_type_copy(node->new_tree.ty, expr->type);
	EXPR_TRACE("end check new expr\n");
	return expr;
}

/**
 * @brief check_func_param : check function parameters
 *
 * @param node : tree node of function parameters
 * @param table : table of block that uses function
 * @param sig : function struct that store function information
 * @param param_start : the start of function parameters
 *
 * @return : pointer to expression struct about function
 */
expr_t* check_func_param(tree_t* node, symtab_t table, signature_t* sig, int param_start) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = NULL;
	vector_t* vect = sig->params;
	int argc = sig->params->len;
	param_t* param = NULL;

	int i = param_start;
	while (node) {
		param = (param_t*)(vect->data[i]);
		if ((sig->has_ellipse) && (i >= (argc - 1))) {
			return NULL;
		}
		expr = check_expr(node, table);
		check_no_defiend_expr(expr, node);
		if (both_scalar_type(param->ty, expr->type) ||
				(both_array_type(param->ty, expr->type)) ||
				(is_same_type(param->ty, expr->type)) ||
				(is_parameter_type(expr->type) && is_scalar_type(param->ty)) ||
				(is_no_type(param->ty) || is_no_type(expr->type))) {
			node = node->common.sibling;
			i++;
		}
		else {
			fprintf(stderr, "file %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
			//error("incompatible type for argument \n");
			break;
		}
	}

	return expr;
}

extern object_t* get_current_obj();

/**
 * @brief first_param_is_obj : check the first parameter of function is object
 *
 * @param sig : function struct that store function information
 *
 * @return : 1 - first parameter is object
 *			0 - firset parameter is not object
 */
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

/**
 * @brief check_function_expr : check the function expression
 *
 * @param node : tree node about function
 * @param table : table of block that uses function
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
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
	if ((func->type) && func->type->common.categ == FUNCTION_T) {
		sig = func->type->function.sig;
		expr->type = func->type->common.bty;
		if (sig)
			argc = sig->params->len;
		arg_num = get_param_num(node->expr_brack.expr_in_brack);
	}
	else if ((func->type) && func->type->common.categ == POINTER_T) {
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

	expr->node = node;
	return expr;
}

/**
 * @brief check_brack_expr : check expression with brack
 *
 * @param node : tree node about expression with brack
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_brack_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	EXPR_TRACE("check function apply\n");
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
	EXPR_TRACE("end check function apply\n");
	return expr;
}

/**
 * @brief check_array_expr : check array expression
 *
 * @param node : tree node about array expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_array_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);
	EXPR_TRACE("check array expr\n");
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type = array_of(type, 0);
	type->common.categ = ARRAY_TYPE;
	type->node = node;
	expr->type = type;

	EXPR_TRACE("end check array expr\n");
	expr->node = node;
	fprintf(stderr, "check array typexx %s, line %d\n", node->common.location.file->name, node->common.location.first_line);
	return expr;
}

/**
 * @brief check_bit_slic_expr : check bit slicing expression
 *
 * @param node : tree node about bit slicing
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_bit_slic_expr(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	EXPR_TRACE("check bit slic\n");
	/* Bit Slicing Expressions : expr[e1:e2, bitorder]
	 *							 expr[e1, bitorder]
	 * expr is of integer type
	 * Both e1 and e2 must be integers
	 */
	expr = check_expression(node->bit_slic.expr, table, expr);
	expr_t* e1 = check_expr(node->bit_slic.bit, table);
	expr_t* e2 = node->bit_slic.bit_end ? check_expr(node->bit_slic.bit_end, table) : NULL;
	if (expr->no_defined || e1->no_defined || (e2 && e2->no_defined)) {
		expr->no_defined = 1;
		expr->node = node;
		return expr;
	}
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
	node->bit_slic.ty = expression_type_copy(node->bit_slic.ty, expr->type);

	EXPR_TRACE("end check bit slic\n");
	return expr;
}

/**
 * @brief check_expression : check all category expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 * @param expr : expression struct to store expression information
 *
 * @return : pointer to expression struct
 */
expr_t* check_expression(tree_t* node, symtab_t table, expr_t* expr) {
	assert(node != NULL); assert(table != NULL); assert(expr != NULL);

	EXPR_TRACE("in expression dispatch: node type %s, file %s, line %d\n", TYPENAME(node->common.type),
										node->common.location.file->name,
										node->common.location.first_line);
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
			break;
	}
	EXPR_TRACE("end expression dispatch: node type %s\n", TYPENAME(node->common.type));
	return expr;
}


/**
 * @brief check_expr : the entry to check expression
 *
 * @param node : tree node about expression
 * @param table : table of block that contains expression
 *
 * @return : pointer to expression struct
 */
expr_t* check_expr(tree_t* node, symtab_t table) {
	if(!node) 
		return NULL;
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = (expr_t*)gdml_zmalloc(sizeof(expr_t));
	check_expression(node, table, expr);

	return expr;
}



/**
 * @brief get_typeof_type : get the type of typeof expression
 *
 * @param node : tree node about typeof expression
 * @param table : table of block that contains typeof expression
 *
 * @return : the type of typeof expression
 */
cdecl_t* get_typeof_type(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	expr_t* expr = check_expr(node->typeof_tree.expr, table);
	expr->type->common.no_decalare = expr->no_defined;
	tree_t *t = node->typeof_tree.expr;
	return expr->type;
}

/**
 * @brief check_comma_expr : check the comma expression
 *
 * @param node : tree node about comma expression
 * @param table : table of block that contains expression
 *
 * @return : pointer to expression struct
 */
expr_t* check_comma_expr(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	assert(table != NULL);

	tree_t* expr_node = node;
	expr_t* expr = NULL;
	while (node != NULL) {
		fprintf(stderr, "check node type %d, assign %d\n", node->common.type, EXPR_ASSIGN_TYPE);
		expr = check_expr(node, table);
		if (expr->no_defined) {
			break;
		}
		node = node->common.sibling;
	}
	node = expr_node;

	return expr;
}
