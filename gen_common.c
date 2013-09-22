/*
 * gen_common.c: 
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

#include "gen_common.h"

symtab_t current_table;
	
static int no_alias;
static int no_star = 1;
static symbol_t SYM;
static int in_inline;
static int base_type = 0;

extern object_t *DEV;
extern object_t *OBJ;

FILE *out;

static tree_t *create_tmp_node(void) {
/*
	int index;
	tree_t *node;
	char  *name;
	int ret;

	index = get_tmp_index();
	ret = asprintf(&name, "%s%d","tmp",index);
	if(ret == -1) {
		printf("mem not enough\n");
	}
	node = create_node(name, TMP_TYPE, sizeof(struct tree_tmp));
	node->tmp_node.name = name;
	*/
	my_DBG("not implemented\n");
	return NULL;
}

static void translate_tmp_node(tree_t *t) {
	/*
	D("%s",t->tmp_node.name);
	*/
}

/*dirty interp*/ 
/*default reduce: do nothing, just derect use the node*/
static tree_t *default_reduce(tree_t *t) {
	return t;
}

/*
static tree_t *reduce(tree_t *t) {
	symbol_t sym;
	
	tree_t *node = create_tmp_node();
	symbol_insert(current_table, node->tmp_node.name, TMP_TYPE);
	sym = symbol_find(current_table, node->tmp_node.name, TMP_TYPE);
	sym_set_aias_name(sym, node->tmp_node.name);

	translate(node);
	D(" = ");
	translate(t);
	D(";");
	new_line();
	return node;
}
*/

static void collect_ref_info(tree_t *expr, ref_info_t *fi){
	tree_t *node;
	node_info_t *ni;
	type_t type = expr->common.type;
	if(type == IDENT_TYPE){
		ni = new_node_info(expr);
		add_node_info(fi, ni);
		return;
	}else if(type == QUOTE_TYPE){
		my_DBG("quote type\n");
		ni = new_node_info(expr);
		add_node_info(fi, ni);
		return;
	}else if(type == COMPONENT_TYPE){
		node = expr->component.expr;
		my_DBG("expr %p \n", node);
		collect_ref_info(node,fi);
		node = dml_keyword_node(expr->common.name, &expr->common.location);
		ni = new_node_info(node);
		add_node_info(fi,ni);
		node = expr->component.ident;
		my_DBG("ident %s, node %p\n", node->ident.str, node);
		ni = new_node_info(node);
		add_node_info(fi,ni);
	} else if (type == DML_KEYWORD_TYPE) {
		/*bank field */
		ni = new_node_info(expr);
		add_node_info(fi, ni);
	}
}

static void printf_ref(ref_info_t *fi){
	tree_t *node;
	struct list_head *p;
	node_info_t *ni;
	const char *name;
	symbol_t sym;
	int i = 0;
	list_for_each(p,&fi->list){
		ni = list_entry(p,node_info_t,entry);
		node = ni->node;
		if(node->common.type == IDENT_TYPE){
			name = node->ident.str;	
		} else if (node->common.type == DML_KEYWORD_TYPE){
			name = node->common.name;
		}else {
			my_DBG("node name %s type %d\n",node->common.name,node->common.type);
		}
		if(i == 0){
			i++;
			sym = symbol_find(current_table,name,0);
			if(!sym){
				my_DBG("no sym %s found in current symtab \n",name);
			}
			name = get_symbol_alias(sym);
			D("%s",name);
		}else {
			D("%s",name);
		}	
	}
}

static void translate_c_ref(tree_t *t) {
	ref_info_t fi;
	
	ref_info_init(&fi);
	collect_ref_info(t, &fi);
	printf_ref(&fi);
	ref_info_destroy(&fi);
}

symbol_t get_ref_sym(tree_t *t, ref_ret_t *ret){
	ref_info_t fi;
	tree_t *node;
	tree_t *tmp;
	tree_t *node_next;
	tree_t *node3;
	node_info_t *ni_next;
	node_info_t *ni;
	struct list_head *p;
	symbol_t sym = NULL;
	const char *name = NULL;
	const char *name2 = NULL;
	struct symtab *symtab = current_table;
	int ref_obj = 0;
	object_t *obj = NULL;
	object_t *obj2 = NULL;

	/*collect ref info*/	
	ref_info_init(&fi);
	collect_ref_info(t, &fi);
	ref_info_print(&fi);
	p = fi.list.next;
	if(p){
		ni = list_entry(p, node_info_t, entry);
		node = ni->node;
		if(node->common.type == IDENT_TYPE){
			ref_obj = 0;
			name = node->ident.str;
			if(is_simics_api(name)) {
				ret->is_obj = 0;
				goto end;
			}
		}else if(node->common.type == QUOTE_TYPE){
			ref_obj = 1;
			node = node->quote.ident;
			name = node->ident.str;
		}else if(node->common.type == DML_KEYWORD_TYPE){
			ref_obj = 1;
			name = node->ident.str;
		} else {
			my_DBG("error component foramt, type %d\n", node->common.type);
		}
	}
	sym = symbol_find_notype(symtab, name);
	if(!ref_obj && sym->type != OBJECT_TYPE){/*just printf it*/
		ret->is_obj = 0;
		goto end;	
	}
	ret->is_obj = 1;	
	/*maybe check expression*/
	p = &fi.list;
	struct list_head *head = p;
	my_DBG("outof\n");
	if(p->next->next == p) {
		/*only one node*/
		goto end;
	}

	while(p->next != head)
	{
		ni = list_entry(p->next, node_info_t, entry);
		ni_next = list_entry(p->next->next, node_info_t, entry);
		node = ni->node;
		node_next = ni_next->node;
		if(node_next) {
			if(node_next->common.type == DML_KEYWORD_TYPE) {
				if(node->common.type == QUOTE_TYPE){
					tmp = node->quote.ident;
					name = tmp->ident.str;
				} else if(node->common.type == IDENT_TYPE || node->common.type == DML_KEYWORD_TYPE){
					name = node->ident.str;
				} else {
					my_DBG("error format\n");
				}
				node3 = ((node_info_t *)ni_next->entry.next)->node;
				if(node3->common.type == IDENT_TYPE){
					name2 = node3->ident.str;
				} else {
					my_DBG("only ident valid\n");
				}
				if(!name2 || !name){
					my_DBG("error name null\n");
				}
				sym = symbol_find(symtab, name,OBJECT_TYPE); 
				my_DBG("object found %s, sym %p\n", name, sym);
				if(!sym){
					my_DBG("no object %s symbol found\n",name);
				}
				obj = (object_t *)(sym->attr);
				symtab = obj->symtab;
				my_DBG("object name %s, name %s\n", obj->name, name2);
				sym = symbol_find_notype(symtab, name2);
				if(sym){
					my_DBG("sym type %d, interface type %d", sym->type, INTERFACE_TYPE);
					/*may be interface function need */
					if(sym->type == OBJECT_TYPE) {
						obj2 = (object_t *)sym->attr;
						if(!strcmp(obj2->obj_type, "interface")) {
							my_DBG("interface object\n");
							ret->con = obj;
							ret->iface = obj2;
							node = t->component.ident;
							name = node->ident.str;
							ret->method = name;	
							break;
						}
					}
				}
			}
			p = ni_next->entry.next;
		}
	}
end:
	my_DBG("out of 2\n");
	ref_info_destroy(&fi);
	return sym;
}

static void translate_parameter(symbol_t sym);
void translate_quote(tree_t *t) {
	tree_t *node = t->quote.ident;
	const char *name;
	const char *ref;
	symbol_t sym;
	object_t *obj;
	ref_ret_t ref_ret;

	if(node->common.type == IDENT_TYPE) {
		name = node->ident.str;
		if(!strcmp(name, "this")) {
			ref = get_obj_ref(OBJ);
			if(!OBJ->is_array) { 
				D("%s", ref);
			} else {
				/*handle array index */
				D("%s[%s]", ref, "_idx0");
			}
			return;
		}
		sym = get_ref_sym(t, &ref_ret);	
		if(sym && (sym->type == PARAMETER_TYPE)) {
			translate_parameter(sym);
		} else if(sym && (sym->type == OBJECT_TYPE)){
			obj = (object_t *)sym->attr;
			name = get_obj_ref(obj);
			D("%s", name);
		} else {
			my_DBG("TODO: sym %p\n", sym);
		}
	} else {
		my_DBG("other type %d\n", node->common.type);
	}
}

static void translate_parameter(symbol_t sym) {
	param_type_t type;
	param_value_t *val;
	object_t *obj;
	const char *name;

	val = (param_value_t *)sym->attr;
	type = val->type;
	switch(type) {
		case param_type_int:
			D("0x%x", val->u.integer);
			break;
		case param_type_float:
			D("%f", val->u.floating);
			break;
		case param_type_bool:
			D("%d", val->u.boolean);
			break;
		case param_type_string:
			D("%s", val->u.string);
			break;
		case param_type_ref:
			obj = val->u.ref;
			if(!strcmp(obj->obj_type, "device")) {
#if backend == 1
				D("&_dev->obj.qdev");
#else 
				D("&_dev->obj");
#endif
				return;
			} else {
				name = get_obj_ref(obj);
				D("%s", name);
			}
			break;
		default:
			my_DBG("invalide parameter type %d\n", type);
			break;
	}
}

void translate_ref_expr(tree_t *t){
	ref_info_t fi;
	symbol_t sym;
	const char *name;
	object_t *obj;
	ref_ret_t ref_ret;


	sym = get_ref_sym(t, &ref_ret);
	if(!strcmp(sym->name, "DMA_interrupt_posted")){
		exit(-1);
	}
	if(sym && !ref_ret.is_obj){
		translate_c_ref(t);
		return;
	}

	if(sym && sym->type == PARAMETER_TYPE){
		/**/
		translate_parameter(sym);
		return;
	}
		
	if(sym && sym->type == OBJECT_TYPE) {
		obj = (object_t *)sym->attr;
		name = get_obj_ref(obj);
		D("%s", name);
		return;
	} 

	if(sym && sym->type == METHOD_TYPE) {
		obj = (object_t *)sym->owner;
		name = get_obj_ref(obj);
	}

}

extern object_t *interface_obj;
int in_method;
static void translate_brack_expr(tree_t *t){
	tree_t *expr_list,*expr;
	const char *name;

	expr_list = t->expr_brack.expr_in_brack;
	expr  = t->expr_brack.expr;
	translate_ref_expr(expr);
	if(in_inline) {
		return;
	}

	if( 1 /*in_method*/){
		D("(_dev");
		if(expr_list){
			D(", ");
		}
	}else if(interface_obj){
		/*connect_obj*/
		name = get_obj_ref(interface_obj->parent);
		interface_obj = NULL;
		D("(%s.obj",name);
		if(expr_list){
			D(", ");
		}
	}else {
		D("(");
	}
	translate_expr_list(expr_list,NULL);	
	if(!in_method){
		D(")");
	}
}

void translate_expr_list(tree_t *expr,const char *prefix){
	tree_t *it = expr;

	while(it){
		if(prefix){
			D("%s", prefix);
		}
		translate(it);
		if(it->common.sibling){
			D(", ");
		}
		it = it->common.sibling;
	}
}

static symbol_t  get_call_expr_info(tree_t *node) {
	symbol_t tmp;
	tree_t *t;
	ref_ret_t ref_ret;
	
	if(node->common.type == EXPR_BRACK_TYPE) {
		t = node->expr_brack.expr;
	} else {
		t = node;
	}
	tmp = get_ref_sym(t, &ref_ret);
	return tmp;
}

static int block_empty(tree_t *t);

static void translate_call_common(tree_t *expr, tree_t *ret){
	//tree_t *expr = t->call_inline.expr;
	//tree_t *ret = t->call_inline.ret_args;
	tree_t *expr_list;
	symbol_t sym;
	symbol_t method_sym;
	const char *name;
	const char *obj_name;
	object_t *obj = NULL;
	tree_t *block;
	tree_t *node;
	method_attr_t *method_attr;

	method_sym = get_call_expr_info(expr);
	if(method_sym) {
		method_attr = method_sym->attr;
		node = method_attr->common.node;	
		block = node->method.block;
		if(block_empty(block)) {
			new_line();
			return;
		}
	} else {
		my_DBG("method not found in call \n");
	}

	sym = symbol_find(current_table,"exec",TMP_TYPE);
	name = get_symbol_alias(sym);
	//POS;
	if(method_sym && (method_sym->type == METHOD_TYPE)) {
		obj = (object_t *)method_sym->owner;
		if(!obj) {
			my_DBG("method object cannot empty\n");
		} else {
			my_DBG("obj name %s, method name %s\n", obj->name, method_sym->name);
			add_object_method(obj, method_sym->name);
		}
	} else {
		my_DBG("method not right %p, %d\n", method_sym, method_sym->type);
	}
	D("%s = ",name);
	obj_name = get_obj_qname(obj);
	if(!strcmp(obj->obj_type, "device")) {
		D("_DML_M_%s", method_sym->name);	
	} else {
		D("_DML_M_%s__%s", obj_name, method_sym->name);
	}
	D("(_dev");
	if(obj->is_array) {
		D(", _idx0");
	}
    if(expr->common.type == EXPR_BRACK_TYPE) {
		expr_list = expr->expr_brack.expr_in_brack;
		if(expr_list) {
			D(", ");
		}
		translate_expr_list(expr_list, NULL);
	}
	if(ret){
		D(", ");
		translate_expr_list(ret,"&");
	}
	D(");\n");
	/*handle exception,hardcode*/
	POS;
	D("if(%s) ",name);
	enter_scope();
	D("return 1;\n");
	exit_scope();
	new_line();
}

void translate_call(tree_t *t) {
	tree_t *expr = t->call_inline.expr;
	tree_t *ret = t->call_inline.ret_args;

	translate_call_common(expr, ret);
}

void translate_after_call(tree_t *t) {
	tree_t *node = t;
	tree_t *call = node->after_call.call_expr;
	/*TODO just ignore timing info */
	translate_call_common(call, NULL);
}

static void process_method_parameters(method_attr_t *m, int alias) {
	int index;
	symbol_t sym;
	symtab_t table;
	int i;
	method_params_t *mp = m->method_params;
	params_t *param;

	table = m->table;
	if(alias) {
		/**/
		index = get_local_index();
		for (i = 0 ; i < mp->in_argc; i ++){
			param = mp->in_list[i];
			sym = symbol_find(table, param->var_name, PARAM_TYPE);
			if(!sym) {
				my_DBG("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym, 2, index);
			my_DBG("sym name %s, alias name %s\n", sym->name, sym->alias_name);
		}
		/**/
		for (i = 0 ; i < mp->ret_argc; i ++){
			param = mp->ret_list[i];
			sym = symbol_find(table, param->var_name,PARAM_TYPE);
			if(!sym) {
				my_DBG("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym, 2, index);
		}
	} else {
		for (i = 0 ; i < mp->in_argc; i ++){
			param = mp->in_list[i];
			sym = symbol_find(current_table,param->var_name,IDENT_TYPE);
			if(!sym) {
				my_DBG("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym,0,index);
		}
		for (i = 0 ; i < mp->ret_argc; i ++){
			param = mp->ret_list[i];
			sym = symbol_find(current_table,param->var_name,IDENT_TYPE);
			if(!sym) {
				my_DBG("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym,1,index);
		}	
	}
}

static void process_inline_start(method_attr_t *m, tree_t *input) {
	tree_t *mt = m->common.node;
	tree_t *params = mt->method.params;
	tree_t *node;
	tree_t *node2;
	const char *name;
	int i = 0;

	node = params->params.in_params;
	node2 = input;
	/*dirty start here*/
	if(node){
		while(node && node2) {
			POS;
			translate(node);
			D(" = ");
			D("%s",node2->ident.str);
			D(";");	
			new_line();
			node = node->common.sibling;
			node2 = node2->common.sibling;
		}
	}

	node = params->params.ret_params;
	
	if(node) {
		while(node) {
			POS;
			translate(node);
			D(";");
			new_line();
			node = node->common.sibling;
		}
	}
}

static int node_is_expression(tree_t *node) {
	int ret = 0;

	switch(node->common.type) {
		case EXPR_ASSIGN_TYPE:
		case BINARY_TYPE:
		case CAST_TYPE:
		case SIZEOF_TYPE:
		case UNARY_TYPE:
		case EXPR_BRACK_TYPE:
		case INTEGER_TYPE:
		case FLOAT_TYPE:
		case CONST_STRING_TYPE:
		case UNDEFINED_TYPE:
		case QUOTE_TYPE:
		case COMPONENT_TYPE:
		case SIZEOFTYPE_TYPE:
		case NEW_TYPE:
		case ARRAY_TYPE:
		case BIT_SLIC_EXPR_TYPE:
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}

void translate_block(tree_t *t) {
	tree_t *node;
	
	enter_scope();
	if(t->block.statement) {
		node = t->block.statement;
		while(node) {
			translate(node);
			if(node_is_expression(node)) {
				D(";");
			}
			new_line();
			node = node->common.sibling;
			if(node) {
				//new_line();
				POS;
			}
		}
	}
	exit_scope();
}
 
void translate_local(tree_t *t) {
	tree_t *node;
	
	node = t->local_tree.local_keyword;
	if(node) {
		/*just ignore local keyword*/	
	}
	if(t->local_tree.is_static) {
		/*static varable should be in dev struct */
		D("static ");
	}
	node = t->local_tree.cdecl;
	translate(node);
	
	node = t->local_tree.expr;
	if(node) {
		D(" = ");
		translate(node);
	}
	D(";");
}

static void translate_expr(tree_t *t) {
	tree_t *node;
	/*
	node = t->expr.expr;
	if(node) {
		translate(node);
	}
	new_line();
	*/
	my_DBG("TODO\n");
}

static int is_explicit(tree_t *t, int *boolean) {
	tree_t *node = t;
	int ret = 0;
	symbol_t sym = NULL;
	object_t *obj = NULL;
	param_value_t *val = NULL;
	param_value_t tmp;
	symtab_t table;

	if(node->common.type == COMPONENT_TYPE && node->component.type == COMPONENT_DOT_TYPE) {
		node = node->component.ident;
		if(node->common.type == IDENT_TYPE) {
			if(!strcmp(node->ident.str, "explicit")) {
					sym = symbol_find(current_table, "fields", PARAMETER_TYPE);
					val = (param_value_t *)sym->attr;
					my_DBG("vector[0] type %d\n", val->type);
					tmp = val->u.list.vector[0];
					obj = (object_t *)tmp.u.ref;
					table = obj->symtab;
					sym = symbol_find(table, "explicit", PARAMETER_TYPE);
					if(sym) {
						val = (param_value_t *)sym->attr;
						*boolean = val->u.integer;
						ret = 1;
					}
				}
			}
		}
	return ret;
}

void translate_if_else(tree_t *t) {
	tree_t *node;
	expression_t *expr = NULL;
	node = t->if_else.cond;
	int explicit;
	int ret;
	symtab_t table;
	symtab_t saved;
	//expr = parse_expression(&node, current_table);
	
	ret = is_explicit(node, &explicit);
	if(ret) {
		/*explicit*/
		if(explicit) {
			node = t->if_else.if_block;
			table = t->if_else.if_table;
		} else {
			node = t->if_else.else_block;
			table = t->if_else.else_table;
		}
		saved = current_table;	
		current_table = table;
		translate(node);
		current_table = saved;
		return;
	}

	if(1) {
		D("if ");
		D("(");
		translate(node);
		D(")");
		node = t->if_else.if_block;
		if(node) {
			table = t->if_else.if_table;
			saved = current_table;
			current_table = table;
			translate(node);
			current_table = saved;
		}
		node = t->if_else.else_if;
		if(node) {
			D("else ");
			table = node->if_else.if_table;
			saved = current_table;
			current_table = table;
			translate(node);
			current_table = saved;
		} else if(t->if_else.else_block) {
			node = t->if_else.else_block;
			D("else ");
			table = t->if_else.else_table;
			saved = current_table;
			current_table = table;
			translate(node);
			current_table = saved;
		}
	} else {
		my_DBG("const if\n");
		if(expr->const_expr->int_value) {
			/*true*/
			node = t->if_else.if_block;
			translate(node);
		} else {
			node = t->if_else.else_block;
			translate(node);
		}
	}	
}

void translate_while(tree_t *t) {
	tree_t *node = t->do_while.cond;
	D("while ");
	D("(");
	translate(node);
	D(") ");

	node = t->do_while.block;
	translate(node);
}

static void translate_comma_expr_opt(tree_t *t) {
	tree_t *node = t;
	while(node) {
		translate(node);
		node = node->common.sibling;
		if(node){
			D(", ");
		}
	}
}

static void translate_for(tree_t *t) {
	tree_t *node;
	
	node = t->for_tree.init;
	D("for ");
	D("(");
	if(node) {
		translate_comma_expr_opt(node);
	}

	D("; ");
	node = t->for_tree.cond;
	if(node) {
		translate(node);
	}
	D("; ");
	node = t->for_tree.update;

	if(node) {
		translate_comma_expr_opt(node);
	}

	D(")");
	node = t->for_tree.block;
	translate(node);
}

static void translate_try_catch(tree_t *t) {
	tree_t *node;
	symbol_t sym;	
	symbol_t sym2;
	symbol_t sym3;
	int i;

	node = t->try_catch.try_block;
	enter_scope();
	symbol_insert(current_table,"exec",TMP_TYPE,NULL);
	symbol_insert(current_table,"throw",TMP_TYPE,NULL);
	sym = symbol_find(current_table,"exec",TMP_TYPE);
	sym2 = symbol_find(current_table,"throw",TMP_TYPE);
	set_throw_symbol_alias(sym2);

	D("bool %s = 0;",get_symbol_alias(sym));
	new_line();
	enter_scope();
	/*try block*/
	translate(node);
	exit_scope();
	
	/*catch block*/
	node = t->try_catch.catch_block;
	D("%s: ;",get_symbol_alias(sym2));
	new_line();
	D("if (%s) ",get_symbol_alias(sym));
	enter_scope();
	translate(node);
	sym3 = symbol_find(current_table,"exit",TMP_TYPE);
	if(sym3) {
		D("goto %s;",get_symbol_alias(sym3));
	}
	exit_scope();
}

static void translate_throw(tree_t *t) {
	tree_t *node = t;
	symbol_t sym;
	symbol_t sym2;

	sym = symbol_find(current_table,"exec",TMP_TYPE);
	sym2 = symbol_find(current_table,"throw",TMP_TYPE);

	enter_scope();
	D("%s = 1;",get_symbol_alias(sym));
	new_line();
	D("goto %s",get_symbol_alias(sym2));
	exit_scope();
}

static symbol_t  get_expression_sym(tree_t *node) {
	symbol_t sym;
	tree_t *tmp;
	const char *name;

	if(node->common.type == QUOTE_TYPE) {
		tmp = node->quote.ident;
		name = tmp->ident.str;
		my_DBG("try to find %s\n", name);
		sym = symbol_find(current_table, name, PARAMETER_TYPE);
		return sym;
	} else {
		my_DBG("TODO: other cases \n");
	}
	return NULL;
}

void translate_foreach(tree_t *t) {
	tree_t *node;
	const char *ident;
	int i;
	int len = 0;
	symbol_t list;
	symbol_t tmp = NULL;
	param_value_t *val = NULL;
	object_t *obj;
	foreach_attr_t *attr;
	symtab_t table;
	symtab_t saved;

	node = t->foreach.ident;
	ident = node->ident.str;
	node = t->foreach.in_expr;
	attr = t->common.attr;
	table = attr->table;
	list = get_expression_sym(node);
	if(list->type != ARRAY_TYPE){
		if(list->type == PARAMETER_TYPE ) {
			val = list->attr;
			if(val->type == param_type_list) {
				len = val->u.list.size;		
			} else {
				my_DBG("error type in foreach\n");
			}
		} else {
			my_DBG("error type in foreach\n");
		}
	}
	saved = current_table;
	if(table) {
		current_table = table;
	}
	tmp = symbol_find(current_table, ident, FOREACH_TYPE);
	if(val->u.list.vector[0].type == param_type_ref) {
		symbol_set_type(tmp, OBJECT_TYPE);
	}
	enter_scope();
	for(i = 0; i < len; i++) {
		symbol_set_value(tmp, val->u.list.vector[i].u.ref);
		obj = (object_t *)val->u.list.vector[i].u.ref;
		my_DBG("--------object %s\n", obj->name);
		enter_scope();
		node = t->foreach.block;
		/*
		if(obj->is_array) {
			D("int _idx0;");
			new_line();
			POS;
			D("for(_idx0 = 0; _idx0 < %d; _idx++)", obj->array_size);
			enter_scope();
		}*/
		translate(node);
		/*if(obj->is_array) {
			exit_scope();
		}*/
		if (node->common.type == BLOCK_TYPE) {
			new_line();
			exit_scope();
			new_line();
			POS;
		} else {
			exit_scope();
			new_line();
		}
	}
	symbol_set_type(tmp, FOREACH_TYPE);
	exit_scope();
	current_table = saved; 
	new_line();
}

static void process_inline_block(method_attr_t *m) {
	tree_t *method = m->common.node;
	tree_t *block = method->method.block;
	POS;
	translate(block);
	return;
}

static void process_inline_end(method_attr_t *m, tree_t *output) {
	tree_t *mt = m->common.node;
	tree_t *params = mt->method.params;
	tree_t *node;
	tree_t *node2;

	node = params->params.ret_params;
	node2 = output;
	if(node) {
		while(node && node2) {
			POS;
			translate(node2);
			D(" = ");
			translate(node);
			new_line();
			node = node->common.sibling;
			node2 = node2->common.sibling;
		}
	}
	return;
}

void translate_inline(tree_t *t) {
	tree_t *expr = t->call_inline.expr;
	tree_t *ret = t->call_inline.ret_args;
	symbol_t sym;
	symtab_t table;
	object_t *saved_obj;
	symtab_t saved_table;

	sym = get_call_expr_info(expr->expr_brack.expr);
	if(sym) {
		my_DBG("sym %s found\n", sym->name);
	} else {
		my_DBG("sym not found\n");
	}
	if(sym->owner != OBJ) {
		saved_obj = OBJ;				
	}
	saved_table = current_table;
	/*get sym info*/
	//translate(expr);
	tree_t *expr_list = expr->expr_brack.expr_in_brack;
	method_attr_t *method = sym->attr;
	//object_t *obj = SYM->attr;
	//process_method_parameters(method, 1);	
	table = method->table;
	table->parent = current_table;
	current_table = table;
	enter_scope();
	process_inline_start(method, expr_list);
	process_inline_block(method);
	D("\n");
	process_inline_end(method,ret);
	D("\n");
	exit_scope();
	return;
}

void translate_typeof(tree_t *t) {
	tree_t *node = t;
	tree_t *expr = node->typeof_tree.expr;
	ref_ret_t ref_ret;
	symbol_t sym;
	const char *name = NULL;
	object_t *obj;

	if(expr->common.type == EXPR_BRACK_TYPE) {
		expr = expr->expr_brack.expr_in_brack;	
		/*
		if(expr->common.type == QUOTE_TYPE) {
			ident = expr->quote.ident;
			if(ident->common.type == IDENT_TYPE) {
				name = ident->ident.str;
			} else {
				printf("error\n");
			}
		} else {
			printf("todo:other quote type\n");
		}*/
		sym = get_ref_sym(expr, &ref_ret);
		if(sym) {
			name = sym->name;
		} else {
			exit(-1);
		}
		if(!strcmp(name, "this")) {
			D("%s", OBJ->attr_type);
		} else if(sym->type == OBJECT_TYPE){
			obj = (object_t *)sym->attr;
			D("%s", obj->attr_type);
		}
	} else {
		my_DBG("other typeof \n");
	}
}

void translate_cdecl(tree_t *t) {
	if (t->cdecl.is_const) {
		D("const ");
	}
	int old_no_star = no_star;
	int old_no_alias = no_alias;
	base_type = 1;
	translate(t->cdecl.basetype);
	D(" ");
	base_type = 0;
	no_star = old_no_star;
	no_alias = old_no_alias;
	my_DBG("here decl2\n");
	translate(t->cdecl.decl);
}

static void print_basetype(tree_t *node) {
	const char *name;
	

	if(node->common.type == IDENT_TYPE) {
		my_DBG("name %s\n", node->ident.str);
		D("%s", node->ident.str);
	} else if(node->common.type == C_KEYWORD_TYPE) {
		D("%s",node->ident.str);
	} else if(node->common.type == TYPEOF_TYPE) {
		translate(node);
    }else {
		my_DBG("other base type\n");
	}
}
static void print_cdecl2(tree_t *t);
static void  print_cdecl1(tree_t *t, int ret) {
	if(t->cdecl.is_const) {
		D("const");
	}
	print_basetype(t->cdecl.basetype);
	D(" ");
	if(ret) {
		D("*");
	}
	print_cdecl2(t->cdecl.decl);
}

static const char *get_cdecl2_name(tree_t *t);
static const char *get_cdecl_name(tree_t *node) {
	return get_cdecl2_name(node->cdecl.decl);
}

static int block_has_call(tree_t *t) {
	tree_t *node;
	tree_t *it;
	tree_t *tmp;
	symbol_t sym;
	int dummy;
	int ret = 0;
	ref_ret_t ref_ret;
	
	node = t;
	if(!node) {
		return 0;
	}
	if(node->common.type == CALL_TYPE) {
		return 1;
	}
	if(node->common.type == BLOCK_TYPE) {
		if(node->block.statement) {
			it = node->block.statement;
			while(it) {
				if(it->common.type == CALL_TYPE) {
					ret = 1;	
					break;
				} else if (it->common.type == BLOCK_TYPE) {
						ret |= block_has_call(it);	
						if(ret) {
							break;
						}
				} else if(it->common.type == IF_ELSE_TYPE) {
						ret = block_has_call(it);
						if(ret) {
							break;
						}
						ret = block_has_call(it);
						if(ret) {
							break;
						}
				} else if (it->common.type == DO_WHILE_TYPE) {
					ret = block_has_call(it->do_while.block);
					if(ret) {
						break;
					}
				} else if (it->common.type == INLINE_TYPE) {
					/*a little trick for inline*/
					tmp = it->call_inline.expr;
					sym = get_ref_sym(tmp->expr_brack.expr,&ref_ret);		
					if(!sym) {
						my_DBG("err,no inline function\n");
					}
					method_attr_t *m = sym->attr;
					tmp = m->common.node;
					tmp = tmp->method.block;
					ret = block_has_call(tmp);
					if(ret) {
						break;
					}
				} else if(it->common.type == FOREACH_TYPE) {
					tmp = it->foreach.block;
					ret = block_has_call(tmp);
					if(ret) {
						break;
					}
				}
				it = it->common.sibling;
			}
		}
	}
	return ret;
}

static int block_has_return(tree_t *t) {
	tree_t *node;
	tree_t *tmp;
	tree_t *it;
	int ret = 0;
	
	node = t;
	if(node->common.type == BLOCK_TYPE) {
		if(node->block.statement) {
			it = node->block.statement;
			while(it) {
				if(it->common.type == RETURN_TYPE) {
					ret = 1;	
					break;
				} else if (it->common.type == BLOCK_TYPE) {
						ret |= block_has_return(it);	
						if(ret) {
							break;
						}
				} else if(it->common.type == IF_ELSE_TYPE) {
						ret = block_has_return(it);
						if(ret) {
							break;
						}
						ret = block_has_return(it);
						if(ret) {
							break;
						}
				} else if (it->common.type == DO_WHILE_TYPE) {
					ret = block_has_return(it->do_while.block);
					if(ret) {
						break;
					}
				} else if(it->common.type == FOREACH_TYPE) {
					tmp = it->foreach.block;
					ret = block_has_call(tmp);
					if(ret) {
						break;
					}
				}
				it = it->common.sibling;
			}
		}
	}
	return ret;
}

static int block_empty(tree_t *t) {
	tree_t *node;
	tree_t *it;
	int ret = 0;
	
	node = t;

	if(!node) {
		return 1;
	}

	if(node->common.type == BLOCK_TYPE) {
		if(!node->block.statement) {
			ret = 1;
		}	
	}
	return ret;
}
static void translate_char(tree_t *t) {
	D("char ");
}

static void translate_double(tree_t *t) {
	D("double ");
}

void translate_float(tree_t *t) {
	D("%s", t->float_cst.float_str);
}

static void translate_int(tree_t *t){
	D("int ");
}

static void translate_long(tree_t *t) {
	D("long ");
}

static void translate_short(tree_t *t) {
	D("short ");
}

static void translate_unsigned(tree_t *t) {
	D("unsigned ");
}

static void translate_signed(tree_t *t) {
	D("signed ");
}

static void translate_void(tree_t *t) {
	D("void ");
}

void translate_dml_keyword(tree_t *t) {
	tree_t *node = t;
	
	D("%s", node->ident.str);
}

void translate_integer_literal(tree_t *t) {
	D("%s", t->int_cst.int_str);
}

void translate_c_keyword(tree_t *t) {
	D("%s", t->ident.str);
}

void translate_log(tree_t *t) {
	tree_t *node = t;
	const char *log_type;

	log_type = node->log.log_type;
	my_DBG("log type %s\n", log_type);
	my_DBG("log format %s\n", node->log.format);
	my_DBG("log args %s\n", node->log.args);
	if(!strcmp(log_type, "\"error\"")) {
		//D("SIM_log_err(&_dev->obj.qdev, 0, %s, %s);", node->log.format, node->log.args);		
	}
}

void translate_ident(tree_t *t) {
	const char *name = t->ident.str;
	const char *alias_name;
	
	if(is_simics_api(name)) {
		D("%s", name);
		return;
	}
	if(base_type) {
		D("%s", name);
		return;
	}

	if(!strcmp(name, "false")) {
		D("0");
		return;
	} else if(!strcmp(name, "true")) {
		D("1");
		return;
	}
	symbol_t sym = symbol_find_notype(current_table, name);
	if(!sym) {
		my_DBG("no sym %s found in table\n",name);
	}
	if(no_alias) {
		if(no_star) {
			D("%s",name);
		} else {
			D("*%s",name);
		}
	} else {
		alias_name = get_symbol_alias(sym);
		D("%s",alias_name);	
	}
}

void translate_cdecl2(tree_t *t) {
	if(t->common.type == CDECL_TYPE) {
		if(t->cdecl.is_const) {
			D("const");
		}else if(t->cdecl.is_point) {
			D("*");
		}else if (t->cdecl.is_vect) {
			/*vect type */
			my_DBG("TODO: vect type\n");
		}
		translate(t->cdecl.decl);
	} else {
		translate(t);
	}
}

static void print_cdecl3(tree_t *node);
static void print_cdecl2(tree_t *t) {
	if(t->common.type == CDECL_TYPE) {
		if(t->cdecl.is_const) {
			D("const");
			print_cdecl2(t->cdecl.decl);
		}else if(t->cdecl.is_point) {
			D("*");
			my_DBG("*\n");
			print_cdecl2(t->cdecl.decl);
		}else if (t->cdecl.is_vect) {
			/*vect type */
			my_DBG("TODO: vect type\n");
		}
	} else {
		my_DBG("cdecl3\n");
		print_cdecl3(t);
	}
}

static const char *get_cdecl3_name(tree_t *node);

static const char *get_cdecl2_name(tree_t *t) {
	if(t->common.type == CDECL_TYPE) {
		return get_cdecl2_name(t->cdecl.decl);
	} else {
		return get_cdecl3_name(t);
	}
	return NULL;
}

static void print_cdecl3(tree_t *t) {
	if(t->common.type == IDENT_TYPE) {
		D("%s", t->ident.str);
	} else {
		my_DBG("TODO: other cdecl3 type\n");
	}
}

static const char *get_cdecl3_name(tree_t *t) {
	if(t->common.type == IDENT_TYPE) {
		return t->ident.str;
	} else {
		my_DBG("TODO: get_name:other cdecl3 type\n");
	}
	return NULL;
}

void translate_cdecl3_array(tree_t *t) {
	translate(t->array.decl);
	D("[");
	translate(t->array.expr);
	D("]");
}

static void translate_decl_list(tree_t *t) {
	tree_t *it = t;
	if(!t) {
		return;
	}
	no_alias = 1;
	while(it){
		if(it->common.type == ELLIPSIS_TYPE) {
			D("...");
		} else if (it->common.type == CDECL_TYPE) {
			translate(it);
		}
		if(it->common.sibling){
			D(", ");
		}
	}
	no_alias = 0;
}
static void translate_cdecl3_brack(tree_t *t) {
	if(t->cdecl_brack.cdecl){
		translate(t->cdecl_brack.cdecl);
	}
	D("(");
	translate_decl_list(t->cdecl_brack.decl_list);
	D(")");
}

static void do_method_param_alias(tree_t *t, int ret) {
	tree_t *node = t;
	const char *name;
	char *dup;
	symbol_t sym;
	int len;

	name = get_cdecl_name(node);
	sym = symbol_find(current_table, name, PARAM_TYPE);
	if(sym) {
		if(!ret) {
			dup = strdup(name);
			set_symbol_alias_name(sym, dup);	
		} else if(ret == 1){
			len = strlen(name) + 2;
			dup = (char *)gdml_zmalloc(len);	
			dup[0] = '*';
			dup[1] = '\0';
			strcat(dup, name);	
			set_symbol_alias_name(sym, dup);
		} else if(ret == 2) {
			/*only for set/get_attribute*/
			const char *status = "_status";
			dup = strdup(status);
			set_symbol_alias_name(sym, dup);
		}
	} else {
		my_DBG("symbol %s not found in method params\n", name);
	}
}

static void translate_cdecl_or_ident_list(tree_t *t, int ret) {
	tree_t *node = t;

	while(node) {
		print_cdecl1(node, ret);
		node = node->common.sibling;
		if(node) {
			D(", ");
		}
	}
}

void cdecl_or_ident_list_params_alias(tree_t *t, int ret) {
	tree_t *node = t;

	while(node) {
		do_method_param_alias(node, ret);
		node = node->common.sibling;
	}
}

static void gen_method_params(object_t *obj, tree_t *m){
	tree_t *params = m->method.params;
 	
	if(params) {
		if(params->params.in_params){
			D(", ");
			/*gen in_params*/
			translate_cdecl_or_ident_list(params->params.in_params, 0);
		}
		if(params->params.ret_params){
			D(", ");
			/*gen ret_params*/
			translate_cdecl_or_ident_list(params->params.ret_params, 1);
		} 
	}
	D(")");
}

static void do_method_params_alias(object_t *obj, tree_t *m){
	tree_t *params = m->method.params;
 	
	if(params) {
		if(params->params.in_params){
			/*gen in_params*/
			cdecl_or_ident_list_params_alias(params->params.in_params, 0);
		}
		if(params->params.ret_params){
			/*gen ret_params*/
			cdecl_or_ident_list_params_alias(params->params.ret_params, 1);
		} 
	}
}

void do_block_logic(tree_t *block) {
	symbol_t sym;
	const char *name;

	D("{\n");
	symbol_insert(current_table, "exec", TMP_TYPE, NULL); 
	sym = symbol_find(current_table, "exec", TMP_TYPE);
	name = get_symbol_alias(sym);
	tabcount_add(1);
	POS;
	D("bool %s = 0;\n",name);
	POS;
	D("UNUSED(%s);\n", name);
	POS;
	translate(block);
	new_line();
	tabcount_sub(1);
	POS;
	D("}\n");
	POS;
}

static void gen_dummy_block(tree_t *block) {
		D("{\n");
		tabcount_add(1);
		POS;
		D("/*dummy function, need optimized*/\n");
		POS;
		D("return 0;\n");
		tabcount_sub(1);
		D("}\n");
}

static void gen_method_block(object_t *obj, tree_t *m){
	tree_t *block = m->method.block;
	symbol_t sym;

	if(block_empty(block)) {
		gen_dummy_block(block);
		return;
	}

	D("{\n");
	D_HEAD;
	tabcount_add(1);
	POS;
	D("{\n");
	tabcount_add(1);
	POS;
	do_block_logic(block);
	/*default return value*/
	D_END;
	D("return 0;\n");
	tabcount_sub(1);
	POS;
	D("}\n");
	tabcount_sub(1);
	POS;
	D("}\n");
}

void gen_dml_method_header(object_t *obj, tree_t *m) {
	/*generate function head*/
	if(strcmp(obj->obj_type, "device" )) {
		if(!obj->is_array) {
			D("\nstatic bool\
				\n_DML_M_%s__%s(%s_t *_dev",obj->qname, m->method.name, DEV->name);
		} else {
			D("\nstatic bool\
				\n_DML_M_%s__%s(%s_t *_dev, int _idx0",obj->qname, m->method.name, DEV->name);
		}
	} else {
		if(!obj->is_array) {
			D("\nstatic bool\
				\n_DML_M_%s(%s_t *_dev", m->method.name, DEV->name);
		} else {
			D("\nstatic bool\
				\n_DML_M_%s(%s_t *_dev, int _idx0", m->method.name, DEV->name);
		}
	}
	gen_method_params(obj, m);
}

static symtab_t saved_table;
void pre_gen_method(object_t *obj, tree_t *method) {	
	method_attr_t *attr;
	symtab_t table;

	attr = method->common.attr;
	table = attr->table;
	saved_table = table->parent;
	table->parent = obj->symtab;
	current_table = table;
	my_DBG("method name %s, obj %s, table num %d\n", method->method.name, obj->name, table->table_num);
	OBJ = obj;
}

void post_gen_method(object_t *obj, tree_t *method) {
	method_attr_t *attr;
	symtab_t table;

	attr = method->common.attr;
	table = attr->table;
	table->parent = saved_table;
	OBJ = NULL;
}

void gen_dml_method(object_t *obj, struct method_name *m) {
	tree_t *method = m->method;

	pre_gen_method(obj, method);
	gen_dml_method_header(obj, method);
	do_method_params_alias(obj, method);
	D("\n");
	tabcount_set(0);
	gen_method_block(obj, method);	
	post_gen_method(obj, method);
}

