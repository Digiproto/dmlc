/*
 * node_type.c: 
 *
 * Copyright (C) 2014 alloc <alloc.young@gmail.com>
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
#include "node_type.h"
#include "expression.h"

extern symtab_t root_table;
static cdecl_t* get_common_type2(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	expr_t expr;

	cdecl_t* type = NULL;
	switch(symbol->type) {
		case PARAM_TYPE:
			type = ((params_t*)(symbol->attr))->decl;
			break;
		case PARAMETER_TYPE:
			type = check_parameter_type(symbol, &expr);
			break;
		case CONSTANT_TYPE:
			type = check_constant_type(symbol, &expr);
			break;
		case FOREACH_TYPE:
			type = check_foreach_type(symbol, &expr);
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
		case OBJECT_TYPE:
			//object_t *obj = symbol->attr;
			type = (cdecl_t *)gdml_zmalloc(sizeof(cdecl_t));
			if(!strcmp(((object_t *)(symbol->attr))->obj_type, "data")) {
				object_t *obj = symbol->attr;
				*type = *(obj->node->common.cdecl);
			} else {
				type->common.categ = OBJECT_T;
				type->object.obj = symbol->attr;
			}
			break;
		case NO_TYPE:
			type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
			type->common.categ = NO_TYPE;
			break;
		case STRUCT_TYPE:
			type = ((struct_attr_t*)(symbol->attr))->decl;
			break;
		case BITFIELDS_ELEM_TYPE:
			type = ((bit_element_t*)(symbol->attr))->decl;
			break;
		default:
			printf("In %s, line = %d, other dml %s(%s)\n",
					__FUNCTION__, __LINE__, symbol->name, TYPENAME(symbol->type));
			/* FIXME: only for debugging */
//			exit(-1);
			break;
	}
	return type;
}

static symtab_t get_type_aggregate_table(cdecl_t *type) {
	int categ;

	categ = type->common.categ;
	if(categ == STRUCT_T) {
		return type->struc.table;
	} else if(categ == LAYOUT_T) {
		return type->layout.table;
	} else {
		return type->bitfields.table;
	}
	return NULL;
}

static symtab_t get_obj_table(cdecl_t *type) {
	object_t *obj;

	obj = type->object.obj;
	return obj->symtab;
}
void check_quote_node(tree_t *node, symtab_t table, type_info_t **info) {
	cdecl_t *cdecl = gdml_zmalloc(sizeof *cdecl);

	NODE_TRACE("check quote, %p, table %p\n", node->quote.ident, table);
	check_expr_type(node->quote.ident, table, info);
	/* just make a copy */
	*cdecl = *node->quote.ident->common.cdecl;
	node->common.cdecl = cdecl;
}

void check_ident_node(tree_t* node, symtab_t table, type_info_t **info) {
	assert(node != NULL); assert(table != NULL);
	cdecl_t *type;
	type = gdml_zmalloc(sizeof *type);
	symbol_t symbol = symbol_find_notype(table, node->ident.str);
	if (symbol == NULL) {
		NODE_TRACE("no symbol found\n");
		symtab_t root_table = get_root_table();
		symbol = symbol_find_notype(root_table, node->ident.str);
		if (!symbol)
			symbol = get_symbol_from_banks(node->ident.str);
	}
	if (symbol != NULL) {
		NODE_TRACE("symbol name: %s, type name: %s\n", symbol->name, TYPENAME(symbol->type));
		if (is_common_type(symbol->type)) {
			type = symbol->attr;
		}
		else {
			NODE_TRACE("get type from common2 %s\n", symbol->name);
			type = get_common_type2(table, symbol);
		}
	}
	else {
		const char* str = node->ident.str;
		if ((strcmp(str, "false") == 0) || (strcmp(str, "true") == 0)) {
			type->common.categ = BOOL_T;
			type->common.size = sizeof(int) * 8;
		}
		else if (strcmp(str, "NULL") == 0) {
			type->common.categ = POINTER_T;
			type->common.size = sizeof(void*) * 8;
		}
		else if (strcmp(str, "this") == 0) {
			type->common.categ = OBJECT_T;
		}
		else if (strcmp(str, "i") == 0) {
			object_t* obj = get_current_obj();
			if (obj && obj->is_array) {
				//new_int_type(expr);
				type->common.categ = INT_T;
				type->common.size = sizeof(int) * 8;
			} else {
				error("%s no undeclared\n", str);
			}
		}
		else {
			if (table->pass_num == 0) {
			} else {
				error("%s no undeclared in template\n", str);
			}
		}
	}
	node->common.cdecl = type;
	NODE_TRACE("ident name %s ,type %p, type name %s\n", node->ident.str, type, TYPENAME(type->common.categ));
}

static void collect_singlebit_info(tree_t *node, type_info_t **info);

void check_bit_slic_node(tree_t *node, symtab_t table, type_info_t **info) {
	cdecl_t *type = NULL;

	NODE_TRACE("start in bit slic, table %p, table num %d\n", table, table->table_num);
	check_expr_type(node->bit_slic.expr, table, info);
	NODE_TRACE("in bit slic\n");
	type = node->bit_slic.expr->common.cdecl;
	if(!type) {
		PERRORN("no type \n", node);
	}
	if(!node->bit_slic.bit_end) {
		if(is_int_type(type)){
			type = gdml_zmalloc(sizeof *type);
			type->common.categ = BITSLIC_T;
			type->common.size = -1;
			collect_singlebit_info(node, info);
		} else if(is_array_type(type)){
			type = type->common.bty;
		} else if(is_type_object(type)) {
			object_t *obj;
			obj = type->object.obj;
			if(obj->is_array) {
				cdecl_t *typex = gdml_zmalloc(sizeof *type);
				object_t *elem = gdml_zmalloc(sizeof *elem);
				*elem = *obj;
				elem->is_array = 0;
				typex->common.categ = OBJECT_T;
				typex->object.obj = elem;
				type = typex;
			} else {
				if(!obj->is_abstract) {
					cdecl_t *typex = gdml_zmalloc(sizeof *type);
					typex->common.categ = INT_T;
					type->common.size = sizeof(int);
					collect_singlebit_info(node, info);
				} else {
					PERRORN("index an abstract non array object \n", node);
				}
			}
		} else {
			NODE_TRACE("got type %d\n", type->common.categ);
			PERRORN("subscripted value is neither array nor pointer nor integer(bitslice)\n", node);
		}
	} else {
		if(!is_int_type(type)) {
			int *p = NULL;
			*p = 0;
			PERRORN("illegal bitslice operation, Expect integer type, got type %s\n", node, TYPENAME(type->common.categ));
		}
		type = gdml_zmalloc(sizeof *type);
		type->common.categ = BITSLIC_T;
		/* it,s  a bitslic the same as bitfield */
		*info = gdml_zmalloc(sizeof **info);
		(*info)->kind = TYPE_BITFIELD;
		(*info)->u.bt.expr = node->bit_slic.expr;
		(*info)->u.bt.start = node->bit_slic.bit;
		(*info)->u.bt.end = node->bit_slic.bit_end;
	}
	NODE_TRACE("leave bit slic\n");
	node->common.cdecl = type;
}

static void collect_layout_info(tree_t *node, type_info_t **info) {
	symtab_t table;
	cdecl_t *type;
	cdecl_t *tmp;

	*info = gdml_zmalloc(sizeof **info);
	(*info)->kind = TYPE_LAYOUT;
	(*info)->u.layout.expr = node->component.expr;
	type = node->component.expr->common.cdecl;
	table = get_type_aggregate_table(type);
	int offset = 0;
	const char *name = node->component.ident->ident.str;
	(*info)->u.layout.endian = type->layout.bitorder;
	symbol_t sym = table->list;
	while(sym) {
		if(!strcmp(sym->name, name)) {
			break;
		} else {
			tmp = sym->attr;
			offset += tmp->common.size;
		}
		sym = sym->lnext;
	}
	(*info)->u.layout.offset = offset/8;
	NODE_TRACE("offsetxxx %d\n", offset);
}

static void collect_bitfield_info(tree_t *node, type_info_t **info) {
	symtab_t table;
	cdecl_t *type;

	if(!*info) {
		*info = gdml_zmalloc(sizeof **info);
	}
	(*info)->kind |= TYPE_BITFIELD;
	(*info)->u.bt.expr = node->component.expr;
	const char *name = node->component.ident->ident.str;
	type = node->component.expr->common.cdecl;
	bit_element_t *e;
	table = get_type_aggregate_table(type);
	symbol_t sym = table->list;
	while(sym) {
		if(!strcmp(sym->name, name)) {
			break;
		}
		sym = sym->lnext;
	}
	e = sym->attr;
	if(e) {
		(*info)->u.bt.start = e->start;
		(*info)->u.bt.end = e->end;
	}
}

static void collect_singlebit_info(tree_t *node, type_info_t **info) {

	*info = gdml_zmalloc(sizeof **info);
	(*info)->kind = TYPE_BITFIELD;
	(*info)->u.bt.start = node->bit_slic.bit;
	(*info)->u.bt.end = NULL;
	(*info)->u.bt.expr = node->bit_slic.expr;
}

void check_component_node(tree_t *node, symtab_t table, type_info_t **info) {
	cdecl_t *type;
	symtab_t tab;

	NODE_TRACE("before check component %p, %p, file %s, line %d\n", node->component.expr, table, node->common.location.file->name, node->common.location.first_line);
	check_expr_type(node->component.expr, table, info);
	type = node->component.expr->common.cdecl;
	if(node->component.type == COMPONENT_DOT_TYPE) {
		if(!is_type_aggregate(type) && !is_type_object(type) ){
			PERRORN("trying to get a member of non-aggregate or object\n", node);
		}
	} else if(node->component.type == COMPONENT_POINTER_TYPE) {
		if(type->common.categ != POINTER_T) {
			PERRORN("not pointer \n", node);
		} else {
			type = type->common.bty;
			if(!is_type_aggregate(type) && !is_type_object(type)) {
				PERRORN("trying to get a member of non-aggregate or object\n", node);
			}
		}
	}
	if(is_type_aggregate(type)) {
		tab = get_type_aggregate_table(type);
	} else {
		tab = get_obj_table(type);
	}
	const char *field = node->component.ident->ident.str;
	symbol_t sym = NULL;
	if(is_type_object(type)) {
		object_t *obj = type->object.obj;
		if(obj && !strcmp(obj->obj_type, "interface")) {
			return;
		}
		sym = symbol_find_notype(tab, field);
	} else {
		sym = _symbol_find_notype(tab->table, field);
	}
	if(!sym) {
		if(is_type_aggregate(type)) {
			PERRORN("unkown aggregate type member %s\n", node, field);
		} else {
			int *p = NULL;
			//*p = 0;
			print_all_symbol(tab);

			PERRORN("object has no child named %s, tab %p, table num %d, root_table %p\n", node, field, tab, tab->table_num, root_table);
		}
	}
	if(is_type_aggregate(type)) {
		//type = sym->attr;
		NODE_TRACE("type name %d layout %d, bitfield %d\n", type->common.categ, LAYOUT_T, BITFIELDS_T);
		switch(type->common.categ) {
			case LAYOUT_T:
				collect_layout_info(node, info);
				break;
			case BITFIELDS_T:
				collect_bitfield_info(node, info);
				break;
			default:
				break;
		}
		type = sym->attr;
	} else {
		type = gdml_zmalloc(sizeof *type);
		type->common.categ = OBJECT_T;
		type->object.obj = sym->attr;
	}
	node->common.cdecl = type;
	NODE_TRACE("component type %p, name %s, table %p\n", type, TYPENAME(type->common.categ), table);
}

void check_node_type(tree_t *node, symtab_t table, type_info_t **info) {

	switch(node->common.type) {
		case QUOTE_TYPE:
			check_quote_node(node, table, info);
			break;
		case IDENT_TYPE:
		case DML_KEYWORD_TYPE:
			check_ident_node(node, table, info);
			break;
		case COMPONENT_TYPE:
			check_component_node(node, table, info);
			break;
		case BIT_SLIC_EXPR_TYPE:
			check_bit_slic_node(node, table, info);
			break;
		default:
			break;
	}
}

void check_expr_type(tree_t *node, symtab_t table, type_info_t **info) {
	check_node_type(node, table, info);
}
