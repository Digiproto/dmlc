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
extern cdecl_t *new_int();
extern cdecl_t *parse_cdecl(tree_t *node, symtab_t table);
static void check_parameter_list(symbol_t sym, symtab_t table) {
		
}

static cdecl_t* get_common_type2(symtab_t table, symbol_t symbol) {
	assert(table != NULL); assert(symbol != NULL);
	expr_t expr;

	cdecl_t* type = NULL;
	fprintf(stderr, "sym type %d, object type %d, no type %d\n", symbol->type, OBJECT_TYPE, NO_TYPE);
	switch(symbol->type) {
		case PARAM_TYPE:
			type = ((params_t*)(symbol->attr))->decl;
			if(type->common.categ == NO_TYPE) {
				tree_t *node = type->node;				
				fprintf(stderr, "parse cdeclxx\n");
				type = parse_cdecl(node, table);		
				if(type->common.categ == NO_TYPE) {
				type = new_int(); 
				}
			}
			break;
		case PARAMETER_TYPE:
			fprintf(stderr, "to check parameterxxxx! type\n");
			type = check_parameter_type(symbol, &expr);
			parameter_attr_t *attr = symbol->attr;
			paramspec_t *spec = attr->param_spec; 
			tree_t *expr_node = NULL;
			if(spec)
				expr_node = spec->expr_node;
			if(type->common.categ == LIST_T) {
				check_parameter_list(symbol, table);
			} else if(!strcmp(symbol->name, "this")) {
				object_t *obj = get_current_obj();
				type->common.categ = OBJECT_T;
				if(obj->is_array) {
					object_t *elem = gdml_zmalloc(sizeof *elem);
					*elem = *obj;	
					elem->is_array = 0;
					type->object.obj = elem;
				} else 
					type->object.obj = obj;
			} else if(expr_node && expr_node->common.type == QUOTE_TYPE) {
					type_info_t *info;
					check_expr_type(expr_node, table, &info);		
					type = expr_node->common.cdecl;
			}
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
			/*
			if(!strcmp(((object_t *)(symbol->attr))->obj_type, "data")) {
				object_t *obj = symbol->attr;
				*type = *(obj->node->common.cdecl);
			} else */ 
			{
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
	} else if (categ == BITFIELDS_T) {
		return type->bitfields.table;
	} else if(categ == POINTER_T) {
		cdecl_t *ptype;
		ptype = type->common.bty;
		categ = ptype->common.categ;
		if((categ  == STRUCT_T ) || (categ == LAYOUT_T) || (categ == BITFIELDS_T));
			return get_type_aggregate_table(ptype);
	}
	return NULL;
}

static symtab_t get_obj_table(cdecl_t *type) {
	object_t *obj;

	obj = type->object.obj;
	fprintf(stderr, "obj %p, obj name %s, is array %d, type %s\n",  obj, obj->name, obj->is_array, obj->obj_type);
	if(!strcmp(obj->obj_type, "data")) {
		type->common.categ = obj->node->common.cdecl->common.categ; 
		fprintf(stderr, "get data table\n");
		return get_data_table2(obj);
	} else {
		return obj->symtab;
	}
	return NULL;
}
void check_quote_node(tree_t *node, symtab_t table, type_info_t **info) {
	cdecl_t *cdecl = gdml_zmalloc(sizeof *cdecl);

	NODE_TRACE("check quote, %p, table %p\n", node->quote.ident, table);
	object_t *obj = get_current_obj();
	symtab_t obj_tab = NULL;
	symtab_t table2;
	if(obj) {
		obj_tab = obj->symtab;	
	}
	if(obj_tab) {
		table2 = obj_tab;	
	} else {
		table2 = table;
	}
	check_expr_type(node->quote.ident, table2, info);
	/* just make a copy */
	*cdecl = *node->quote.ident->common.cdecl;
	node->common.cdecl = cdecl;
}

extern object_t *default_bank;
symbol_t get_default_bank_obj(const char *name) {
	symbol_t sym = NULL;
	if(default_bank) {
		symtab_t table = default_bank->symtab;
		sym = defined_symbol(table, name, OBJECT_TYPE);	
	}
	return sym;
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
			//symbol = get_symbol_from_banks(node->ident.str);
			symbol = get_default_bank_obj(node->ident.str);
			
	}
	if (symbol != NULL) {
		NODE_TRACE("symbolxxx name: %s, type : %d\n", symbol->name, symbol->type);
		if(symbol->type == PARAM_TYPE) {
			symbol_t tmp = _symbol_find_not(table->table, node->ident.str, PARAM_TYPE);
			if(tmp)
				symbol = tmp;
		}
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
			object_t *obj = get_current_obj();			
			type->object.obj = obj; 
			fprintf(stderr, "object ++++++++++ obj %p\n", obj);
		}
		else if (strcmp(str, "i") == 0) {
			object_t* obj = get_current_obj();
			if (obj && obj->is_array) {
				//new_int_type(expr);
				type->common.categ = INT_T;
				type->common.size = sizeof(int) * 8;

				fprintf(stderr, "kkkkkkk++++\n");
			} else {
				error("%s no undeclared\n", str);
			}
		}
		else {
			if (table->pass_num == 0) {
				fprintf(stderr, "kkkkkkk___\n");
			} else {
				error("%s no undeclared in template\n", str);
			}
		}
	}
	node->common.cdecl = type;
	NODE_TRACE("ident name %s ,type %d, ident type %d\n", node->ident.str, type->common.categ, IDENT_TYPE);
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
				if(!strcmp(obj->obj_type, "data")) {
					cdecl_t *type = obj->node->common.cdecl;
					fprintf(stderr, "arrayxxx name %s\n", obj->name);
					if(type->common.categ == ARRAY_T) {
						cdecl_t *type2 = type->common.bty;
						if(type2 && type2->common.categ == ARRAY_T) {
							elem->is_array = 1;
						} else {
							elem->is_array = 0;
						}
					}
				} else {
					elem->is_array = 0;
				}
				typex->common.categ = OBJECT_T;
				typex->object.obj = elem;
				type = typex;
				fprintf(stderr, "object arrayxxxx, obj type %s\n", elem->obj_type);
			} else {
				if(!obj->is_abstract) {
					fprintf(stderr, "objxxxx name %s\n", obj->name);
					cdecl_t *typex = gdml_zmalloc(sizeof *type);
					typex->common.categ = INT_T;
					type->common.size = sizeof(int);
					collect_singlebit_info(node, info);
				}
			}
		} else if(type->common.categ == LIST_T){ 
			tree_t *node_list;
			tree_t *elem;
			tree_t *list;
			
			node_list = type->node;								
			list = node_list->array.expr;
			elem = list;
			type_info_t *dummy;
			type->node = elem;
		}
	} else {
		type = gdml_zmalloc(sizeof *type);
		type->common.categ = BITSLIC_T;
		/* it,s  a bitslic the same as bitfield */
		*info = gdml_zmalloc(sizeof **info);
		(*info)->kind = TYPE_BITFIELD;
		(*info)->bt.expr = node->bit_slic.expr;
		(*info)->bt.start = node->bit_slic.bit;
		(*info)->bt.end = node->bit_slic.bit_end;
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
	(*info)->layout.expr = node->component.expr;
	type = node->component.expr->common.cdecl;
	table = get_type_aggregate_table(type);
	int offset = 0;
	const char *name = node->component.ident->ident.str;
	(*info)->layout.endian = type->layout.bitorder;
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
	(*info)->layout.offset = offset/8;
	NODE_TRACE("offsetxxx %d\n", offset);
}

static void collect_bitfield_info(tree_t *node, type_info_t **info) {
	symtab_t table;
	cdecl_t *type;

	if(!*info) {
		*info = gdml_zmalloc(sizeof **info);
	}
	(*info)->kind |= TYPE_BITFIELD;
	(*info)->bt.expr = node->component.expr;
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
		(*info)->bt.start = e->start;
		(*info)->bt.end = e->end;
	}
}

static void collect_singlebit_info(tree_t *node, type_info_t **info) {

	*info = gdml_zmalloc(sizeof **info);
	(*info)->kind = TYPE_BITFIELD;
	(*info)->bt.start = node->bit_slic.bit;
	(*info)->bt.end = NULL;
	(*info)->bt.expr = node->bit_slic.expr;
}

void check_component_node(tree_t *node, symtab_t table, type_info_t **info) {
	cdecl_t *type;
	symtab_t tab;

	NODE_TRACE("before check component %p, %p, file %s, line %d\n", node->component.expr, table, node->common.location.file->name, node->common.location.first_line);
	fprintf(stderr, "table num %d\n", table->table_num);
	check_expr_type(node->component.expr, table, info);
	type = node->component.expr->common.cdecl;
	if(type->common.categ == LIST_T){ 
		tree_t *elem;

		elem = type->node;								
		type_info_t *dummy;
		check_expr_type(elem, table, &dummy);	
		node->component.expr = elem;
		type = elem->common.cdecl;
		fprintf(stderr, "list eval, node %p\n", elem);
	}
	if(node->component.type == COMPONENT_DOT_TYPE) {
		if(!is_type_aggregate(type) && !is_type_object(type) ){
			//PERRORN("trying to get a member of non-aggregate or object\n", node);
		}
	} else if(node->component.type == COMPONENT_POINTER_TYPE) {
		if(type->common.categ != POINTER_T || !(type->common.categ == OBJECT_TYPE)) {
		} else {
			type = type->common.bty;
			if(!is_type_aggregate(type) && !is_type_object(type)) {
				PERRORN("trying to get a member of non-aggregate or object\n", node);
			}
		}
	}
	if(is_type_aggregate(type) || type->common.categ == POINTER_T) {
		tab = get_type_aggregate_table(type);
	} else {
		fprintf(stderr, "type %d\n", type->common.categ);
		tab = get_obj_table(type);
	}
	const char *field = node->component.ident->ident.str;
	symbol_t sym = NULL;
	if(is_type_object(type)) {
		object_t *obj = type->object.obj;
		if(obj && (!strcmp(obj->obj_type, "interface") )) {
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
	if(is_type_aggregate(type)|| type->common.categ == POINTER_T) {
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
		object_t *obj = type->object.obj;
		if(!strcmp(obj->obj_type, "data")) {
			type = sym->attr;
		} else {
			type = gdml_zmalloc(sizeof *type);
			type->common.categ = OBJECT_T;
			type->object.obj = sym->attr;
		}
	}
	node->common.cdecl = type;
	NODE_TRACE("component type %p, type %d, table %p\n", type, type->common.categ, table);
}

static void check_unary_type(tree_t *node, symtab_t table, type_info_t **info) {
	tree_t *expr;
	cdecl_t *type;
	cdecl_t *tmp;	
	
	type = gdml_zmalloc(sizeof *type);
	expr = node->unary.expr;
	check_expr_type(expr, table, info);
	tmp = expr->common.cdecl;	
	if(node->unary.type == ADDR_TYPE) {
		type->common.categ = POINTER_T;		
		type->common.bty = tmp;
	} else if(node->unary.type == POINTER_TYPE) {
		*type = *tmp->common.bty;	
	}
	node->common.cdecl = type;
}

static void check_binary_type(tree_t *node, symtab_t table, type_info_t **info) {
	tree_t *left, *right;
	
	left = node->binary.left;
	right = node->binary.right;
	check_expr_type(left, table, info);	
	check_expr_type(right, table, info);
	node->common.cdecl = left->common.cdecl;	
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
		case EXPR_BRACK_TYPE:
			check_node_type(node->expr_brack.expr_in_brack, table, info);
			node->common.cdecl = node->expr_brack.expr_in_brack->common.cdecl;
			break;
		case UNARY_TYPE:
			check_unary_type(node, table, info);	
			break;
		case BINARY_TYPE:
			check_binary_type(node, table, info);
			break;
		default:
			break;
	}
}

void check_expr_type(tree_t *node, symtab_t table, type_info_t **info) {
	check_node_type(node, table, info);
}
