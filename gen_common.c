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
	printf("not implemented\n");
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
		printf("quote type\n");
		ni = new_node_info(expr);
		add_node_info(fi, ni);
		return;
	}else if(type == COMPONENT_TYPE){
		node = expr->component.expr;
		printf("expr %p \n", node);
		collect_ref_info(node,fi);
		node = dml_keyword_node(expr->common.name);	
		ni = new_node_info(node);
		add_node_info(fi,ni);
		node = expr->component.ident;
		printf("ident %s, node %p\n", node->ident.str, node);
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
			printf("node name %s type %d\n",node->common.name,node->common.type);
		}
		if(i == 0){
			i++;
			sym = symbol_find(current_table,name,0);
			if(!sym){
				printf("no sym %s found in current symtab \n",name);
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

static symbol_t get_ref_sym(tree_t *t, int *is_obj, object_t **iface_obj,const char **op_name){
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
		}else if(node->common.type == QUOTE_TYPE){
			ref_obj = 1;
			node = node->quote.ident;
			name = node->ident.str;
		}else if(node->common.type == DML_KEYWORD_TYPE){
			ref_obj = 1;
			name = node->ident.str;
		} else {
			printf("error component foramt, type %d\n", node->common.type);
		}
	}
	sym = symbol_find_notype(symtab, name);
	if(!ref_obj && sym->type != OBJECT_TYPE){/*just printf it*/
		*is_obj = 0;
		return sym;	
	}
	*is_obj = 1;	
	/*maybe check expression*/
	p = &fi.list;
	struct list_head *head = p;
	printf("outof\n");
	if(p->next->next == p) {
		/*only one node*/
		return sym;
	}

	while(p->next != head)
	{
		ni = list_entry(p->next,node_info_t,entry);
		ni_next = list_entry(p->next->next,node_info_t,entry);
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
					printf("error format\n");
				}
				node3 = ((node_info_t *)ni_next->entry.next)->node;
				if(node3->common.type == IDENT_TYPE){
					name2 = node3->ident.str;
				} else {
					printf("only ident valid\n");
				}
				if(!name2 || !name){
					printf("error name null\n");
				}
				sym = symbol_find(symtab, name,OBJECT_TYPE); 
				printf("object found %s, sym %p\n", name, sym);
				if(!sym){
					printf("no object %s symbol found\n",name);
				}
				obj = (object_t *)(sym->attr);
				symtab = obj->symtab;
				printf("object name %s\n", obj->name);
				sym = symbol_find_notype(symtab,name2);
				if(!sym){
					/*may be interface function need */
					if(!strcmp(obj->obj_type,"interface") && !strcmp(node->common.name, "->")) {
						*iface_obj = obj;
						*op_name = name2;
						break;
					}
				}
			}
			p = ni_next->entry.next;
		}
	}
	ref_info_destroy(&fi);
	return sym;
}

void translate_quote(tree_t *t) {
	tree_t *node = t->quote.ident;
	const char *name;
	const char *ref;
	symbol_t sym;
	int is_obj;
	const char *dummy_name;
	object_t *dummy_obj;

	if(node->common.type == IDENT_TYPE) {
		name = node->ident.str;
		if(!strcmp(name, "this")) {
			ref = get_obj_ref(OBJ);
			D("%s", ref);
		}
		sym = get_ref_sym(t, &is_obj, &dummy_obj, &dummy_name);	
		if(sym) {
			param_value_t *val = (param_value_t *)sym->attr;	
			if(val->type == param_type_int) {
				D("0x%x", val->u.integer);
			} else {
				printf("other param val type %d\n", val->type);
			}
		} else {
			printf("symbol not found\n");
		}
	} else {
		printf("other type %d\n", node->common.type);
	}
}

static void translate_ref_expr(tree_t *t){
	ref_info_t fi;
	symbol_t sym;
	const char *name;
	object_t *dummy_obj;
	const char *dummy_name;
	int is_obj;
	object_t *obj;


	sym = get_ref_sym(t, &is_obj, &dummy_obj, &dummy_name);
	if(sym && !is_obj){
		translate_c_ref(t);
		return;
	}

	if(sym && sym->type == PARAMETER_TYPE){
		/**/
	}
	
	if(sym && sym->type == OBJECT_TYPE) {
		obj = (object_t *)sym->owner;
		name = get_obj_ref(obj);	
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
			D(prefix);
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
	const char *name;
	object_t *dummy_obj;
	const char *dummy_name;
	int is_obj;
	tree_t *t;

	if(node->common.type == EXPR_BRACK_TYPE) {
		t = node->expr_brack.expr;
	} else {
		t = node;
	}

	tmp = get_ref_sym(t, &is_obj, &dummy_obj, &dummy_name);
	return tmp;
}

static int block_empty(tree_t *t);
void translate_call(tree_t *t){
	tree_t *expr = t->call_inline.expr;
	tree_t *ret = t->call_inline.ret_args;
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
		printf("method not found in call \n");
	}

	sym = symbol_find(current_table,"exec",TMP_TYPE);
	name = get_symbol_alias(sym);
	//POS;
	D("%s = ",name);
	if(method_sym && (method_sym->type == METHOD_TYPE)) {
		obj = (object_t *)method_sym->owner;
		if(!obj) {
			printf("method object cannot empty\n");
		} else {
			printf("obj name %s, method name %s\n", obj->name, method_sym->name);
			add_object_method(obj, method_sym->name);
		}
	} else {
		printf("method not right\n");
	}
	/*
	if(expr->common.type == EXPR_BRACK_TYPE){
		in_method = 1;
		translate(expr);	
		in_method = 0;
	} else {
		translate(expr);
	}*/
	obj_name = get_obj_qname(obj);
	D("_DML_M_%s__%s", obj_name, method_sym->name);
	D("(_dev");
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
	POS;
	D("return 1;\n");
	exit_scope();
	new_line();
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
				printf("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym, 2, index);
			printf("sym name %s, alias name %s\n", sym->name, sym->alias_name);
		}
		/**/
		for (i = 0 ; i < mp->ret_argc; i ++){
			param = mp->ret_list[i];
			sym = symbol_find(table, param->var_name,PARAM_TYPE);
			if(!sym) {
				printf("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym, 2, index);
		}
	} else {
		for (i = 0 ; i < mp->in_argc; i ++){
			param = mp->in_list[i];
			sym = symbol_find(current_table,param->var_name,IDENT_TYPE);
			if(!sym) {
				printf("no sym %s found\n",param->var_name);
			}
			set_symbol_alias(sym,0,index);
		}
		for (i = 0 ; i < mp->ret_argc; i ++){
			param = mp->ret_list[i];
			sym = symbol_find(current_table,param->var_name,IDENT_TYPE);
			if(!sym) {
				printf("no sym %s found\n",param->var_name);
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
			POS;
			translate(node);
			if(node_is_expression(node)) {
				D(";");
			}
			new_line();
			node = node->common.sibling;
			if(node) {
				//new_line();
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
	printf("TODO\n");
}

void translate_if_else(tree_t *t) {
	tree_t *node;
	expression_t *expr = NULL;
	node = t->if_else.cond;
	
	//expr = parse_expression(&node, current_table);
	if( 1 || !expr->is_const) {
		D("if ");
		D("(");
		translate(node);
		D(")");
		node = t->if_else.if_block;
		translate(node);
		node = t->if_else.else_block;
		if(node) {
			D("else ");
			translate(node);
		}
	} else {
		printf("const if\n");
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

static void translate_while(tree_t *t) {
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
		printf("try to find %s\n", name);
		sym = symbol_find(current_table, name, PARAMETER_TYPE);
		return sym;
	} else {
		printf("TODO: other cases \n");
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
	
	node = t->foreach.ident;
	ident = node->ident.str;
	node = t->foreach.in_expr;
	list = get_expression_sym(node);
	if(list->type != ARRAY_TYPE){
		if(list->type == PARAMETER_TYPE ) {
			val = list->attr;
			if(val->type == param_type_list) {
				len = val->u.list.size;		
			} else {
				printf("error type in foreach\n");
			}
		} else {
			printf("error type in foreach\n");
		}
	}
	printf("before symbol insert\n");
	symbol_insert(current_table, ident, val->u.list.vector[0].type, NULL);
	printf("symbol %s insert\n", ident);
	tmp = symbol_find(current_table, ident, val->u.list.vector[0].type);
	printf("symbol %s found\n", ident);
	if(val->u.list.vector[0].type == param_type_ref) {
		symbol_set_type(tmp, OBJECT_TYPE);
	}
	enter_scope();
	for(i = 0; i < len; i++) {
		symbol_set_value(tmp, val->u.list.vector[i].u.ref);
		POS;
		enter_scope();
		node = t->foreach.block;
		if(node->common.type != BLOCK_TYPE){
			POS;
		}
		translate(node);
		exit_scope();
		new_line();
	}
	symbol_set_type(tmp, val->u.list.vector[0].type);
	exit_scope();
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

	sym = get_call_expr_info(expr->expr_brack.expr);
	if(sym) {
		printf("sym %s found\n", sym->name);
	} else {
		printf("sym not found\n");
	}
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
	const char *name = NULL;
	tree_t *ident;

	if(expr->common.type == EXPR_BRACK_TYPE) {
		expr = expr->expr_brack.expr_in_brack;	
		if(expr->common.type == QUOTE_TYPE) {
			ident = expr->quote.ident;
			if(ident->common.type == IDENT_TYPE) {
				name = ident->ident.str;
			} else {
				printf("error\n");
			}
		} else {
			printf("todo:other quote type\n");
		}
		if(!strcmp(name, "this")) {
			D("%s", OBJ->attr_type);
		} else {
			printf("todo: other typeof name %s\n", name);
		}
	} else {
		printf("other typeof \n");
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
	printf("here decl2\n");
	translate(t->cdecl.decl);
}

static void print_basetype(tree_t *node) {
	const char *name;
	

	if(node->common.type == IDENT_TYPE) {
		printf("name %s\n", node->ident.str);
		D("%s", node->ident.str);
	} else if(node->common.type == C_KEYWORD_TYPE) {
		D("%s",node->ident.str);
	} else if(node->common.type == TYPEOF_TYPE) {
		translate(node);
    }else {
		printf("other base type\n");
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
					sym = get_ref_sym(tmp->expr_brack.expr,&dummy, NULL, NULL);		
					if(!sym || !dummy) {
						printf("err,no inline function\n");
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

static void translate_float(tree_t *t) {
	D("float ");
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
	printf("log type %s\n", log_type);
	printf("log format %s\n", node->log.format);
	printf("log args %s\n", node->log.args);
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
		printf("no sym %s found in table\n",name);
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
			printf("TODO: vect type\n");
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
			printf("*\n");
			print_cdecl2(t->cdecl.decl);
		}else if (t->cdecl.is_vect) {
			/*vect type */
			printf("TODO: vect type\n");
		}
	} else {
		printf("cdecl3\n");
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
		printf("TODO: other cdecl3 type\n");
	}
}

static const char *get_cdecl3_name(tree_t *t) {
	if(t->common.type == IDENT_TYPE) {
		return t->ident.str;
	} else {
		printf("TODO: get_name:other cdecl3 type\n");
	}
	return NULL;
}

static void translate_cdecl3_array(tree_t *t) {
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
		printf("symbol %s not found in method params\n", name);
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
	tabcount_add(1);
	POS;
	D("{\n");
	tabcount_add(1);
	POS;
	do_block_logic(block);
	/*default return value*/
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
		D("\nstatic bool\
			\n_DML_M_%s__%s(%s_t *_dev",obj->qname, m->method.name, DEV->name);
	} else {
		D("\nstatic bool\
			\n_DML_M_%s(%s_t *_dev", m->method.name, DEV->name);
	}
	gen_method_params(obj, m);
}

void pre_gen_method(object_t *obj, tree_t *method) {	
	method_attr_t *attr;
	symtab_t table;

	attr = method->common.attr;
	table = attr->table;
	table->parent = obj->symtab;
	current_table = table;
	OBJ = obj;
}

void post_gen_method(object_t *obj, tree_t *method) {
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

