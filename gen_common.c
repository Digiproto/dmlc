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
#include "info_output.h"
#include "parameter_type.h"
#include <assert.h>
obj_ref_t *OBJ;
symtab_t current_table;
static flow_ctrl_t *flow;
static int no_alias;
static int no_star = 1;
static symbol_t SYM;
static int in_inline;
static int base_type = 0;
extern object_t *DEV;
FILE *out;

static void translate_parameter(symbol_t sym);

/**
 * @brief translate_quote : translate the quote expression from dml to c languange
 *
 * @param t : syntax tree node of expression
 */
void translate_quote(tree_t *t) {
	tree_t *node = t->quote.ident;
	const char *name;
	const char *ref;
	symbol_t sym;
	object_t *obj;
	ref_ret_t ref_ret;
	init_ref_ret(&ref_ret);
	
	
	if( 1 || node->common.type == IDENT_TYPE) {
		name = node->ident.str;
		if(!strcmp(name, "this")) {
			/*
			ref = get_obj_ref(OBJ);
			if(!OBJ->is_array) { 
				D("%s", ref);
			} else {
				//handle array index 
				D("%s[%s]", ref, "_idx0");
			}*/
			if(OBJ->ref) {
				printf_ref(&OBJ->ret);	
			} else {
				object_t *obj = OBJ->obj;
				ref = get_obj_ref(OBJ->obj);
				/*
				if(obj->is_array && strcmp(obj->obj_type, "data")) {
					int len = 0;
					char *tmp_str;
					tmp_str = strrchr(ref, '[');
					if(tmp_str) {
						len = tmp_str - ref;
					}
					tmp_str = gdml_zmalloc(len + 1);
					memcpy(tmp_str, ref, len);
					tmp_str[len] = '\0';
					ref = tmp_str;
				}*/		
				D("%s", ref);
			}
			return;
		}
		sym = get_ref_sym(t, &ref_ret, NULL);	
		if(sym && (sym->type == PARAMETER_TYPE)) {
			translate_parameter(sym);
		} else if(sym && (sym->type == OBJECT_TYPE)){
			obj = (object_t *)sym->attr;
			//name = get_obj_ref(obj);
			//fprintf(stderr, "object name %s\n", obj->name);
			printf_ref(&ref_ret);
			//D("%s", name);
		} else {
			my_DBG("TODO: sym %p\n", sym);
			fprintf(stderr, "fucking you %p\n", sym);
			if(sym)
				fprintf(stderr, "type %d, %d\n", sym->type, ATTRIBUTE_TYPE);
			exit(-1);
		}
	} else {
		my_DBG("other type %d\n", node->common.type);

		fprintf(stderr, "other type %d\n", node->common.type);

		exit(-1);
	}
}

/**
 * @brief translate_parameter : translate the paramter to c language
 *
 * @param sym : the symbol of parameter
 */
static void translate_parameter(symbol_t sym) {
	param_type_t type;
	param_value_t *val;
	paramspec_t *spec;
	parameter_attr_t *attr;
	object_t *obj;
	const char *name;
	tree_t *expr;
	
	val = (param_value_t *)sym->attr;		
	if(val->is_original) {
		attr = (parameter_attr_t *)sym->attr;
		spec = (paramspec_t *)attr->param_spec;
		val = (param_value_t *)spec->value;
		expr = spec->expr_node;
		if(expr && expr->common.type == QUOTE_TYPE) {
			object_t *obj = get_parameter_obj(current_table, sym->name);	
			if(obj) {
				name = get_obj_ref(obj);	
				D("%s", name);
				return;
			}
		}
	} else {
		val = (param_value_t *)sym->attr;
	}
	type = val->type;
	switch(type) {
		case PARAM_TYPE_INT:
			D("0x%x", val->u.integer);
			break;
		case PARAM_TYPE_FLOAT:
			D("%f", val->u.floating);
			break;
		case PARAM_TYPE_BOOL:
			D("%d", val->u.boolean);
			break;
		case PARAM_TYPE_STRING:
			D("%s", val->u.string);
			break;
		case PARAM_TYPE_REF:
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
			D("0x0");
			my_DBG("invalide parameter type %d\n", type);
			break;
	}
}

/**
 * @brief translate_ref_expr : translate the reference expression to language
 *
 * @param t : the syntax tree node of reference expression
 */
void translate_ref_expr(tree_t *t){
	ref_info_t fi;
	symbol_t sym;
	const char *name;
	object_t *obj;
	ref_ret_t ref_ret;
	init_ref_ret(&ref_ret);


	sym = get_ref_sym(t, &ref_ret, NULL);
	if(!sym) {
		fprintf(stderr, "no symbol found \n");
		exit(-1);
	}
	if(sym && !ref_ret.is_obj){
		//translate_c_ref(t);
		printf_ref(&ref_ret);
		return;
	}

	if(sym && sym->type == PARAMETER_TYPE){
		/**/
		translate_parameter(sym);
		return;
	}
		
	if(sym && sym->type == OBJECT_TYPE) {
		/*
		obj = (object_t *)sym->attr;
		name = get_obj_ref(obj);
		D("%s", name);
		*/
		printf_ref(&ref_ret);
		return;
	} 

	if(sym && sym->type == METHOD_TYPE) {
		obj = (object_t *)sym->owner;
		name = obj->name;
		if(!strcmp(obj->obj_type, "device")) {
			D("_DML_M_%s", sym->name);
		} else {
			if(obj->encoding == Obj_Type_Event) {
				name = obj->qname;
				D("_DML_EV_%s__%s", name, sym->name);
			} else {
				D("_DML_M_%s__%s", name, sym->name);
			}
		}
	}
}

extern object_t *interface_obj;
int in_method;

/**
 * @brief translate_brack_expr : translate the brack expression to c language
 *
 * @param t : syntax tree node of brack expression
 */
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

/**
 * @brief translate_expr_list : translate the expression list to c language
 *
 * @param expr : syntax tree node of expression list
 * @param prefix: the prefix of expression list
 */
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

/**
 * @brief get_call_expr_info : get the symbol of call
 *
 * @param node : syntax tree node of call expression
 * @param table : the table of block that contains call expression
 *
 * @return : symbol of call expression
 */
extern symtab_t root_table;
symbol_t  get_call_expr_info(tree_t *node, symtab_t table) {
	symbol_t tmp;
	tree_t *t;
	ref_ret_t ref_ret;
	init_ref_ret(&ref_ret);
	
	if(node->common.type == EXPR_BRACK_TYPE) {
		t = node->expr_brack.expr;
	} else {
		t = node;
	}
	tmp = get_ref_sym(t, &ref_ret, table);
	if(tmp->type != METHOD_TYPE) {
		tmp = symbol_find(table, tmp->name, METHOD_TYPE);
	}
	return tmp;
}

/**
 * @brief get_call_expr_ref  : get the reference of call expression
 *
 * @param node : tree node of call/inline expression
 * @param ref_ret : the list of reference
 *
 * @return : symbol of the last reference
 */
static symbol_t  get_call_expr_ref(tree_t *node, ref_ret_t *ref_ret) {
   symbol_t tmp;
   tree_t *t;
   init_ref_ret(ref_ret);

   if(node->common.type == EXPR_BRACK_TYPE) {
       t = node->expr_brack.expr;
   } else {
       t = node;
   }
   tmp = get_ref_sym(t, ref_ret, NULL);
   if(tmp->type != METHOD_TYPE) {
        tmp = symbol_find(current_table, tmp->name, METHOD_TYPE);
   }
   return tmp;
}

/**
 * @brief check_param_type : check the type of parameter is compatible
 *
 * @param node : syntax tree node of method parameters
 * @param list : two dimensional array for method defined parameters
 * @param args : the number of args
 * @param in_line : sign of inline expression
 *
 * @return : 1 - type is not compatible
 *			0 - type is compatible
 */
static int check_param_type(tree_t* node, params_t** list, int args, int in_line) {
	int i = 0;
	cdecl_t* type = 0;
	expr_t* expr = NULL;
	tree_t* tmp = node;

	while (tmp != NULL) {
		if ((list[i]->is_notype) && (in_line == 0)) {
			PERRORN("no type for input/output parameter\n", tmp);
			return 0;
		}
		type = list[i]->decl;
		expr = check_expr(tmp, current_table);
		if ((!both_scalar_type(type, expr->type)) && !(is_same_type(type, expr->type)) &&
			(!both_array_type(type, expr->type)) && !((is_no_type(type) || is_no_type(expr->type))) &&
			!(is_parameter_type(type) || is_parameter_type(expr->type))) {
			return 1;
		}
		tmp = tmp->common.sibling;
		i++;
	}

	return 0;
}

/**
 * @brief get_method_param_node : get syntax tree node of method parameters
 *
 * @param node: syntax tree node of method parameters with brack
 *
 * @return : tree node of parameters
 */
static tree_t* get_method_param_node(tree_t* node) {
	assert(node != NULL);
	/* base on the grammar, if an expression have brack
	 * the topest node about the expression is brack */
	if (node->common.type == EXPR_BRACK_TYPE) {
		fprintf(stderr, "heeeh %p\n", node->expr_brack.expr_in_brack);
		return node->expr_brack.expr_in_brack;
	}
	else
		return NULL;
}

/**
 * @brief check_method_in_param : check the type parameters is compatible with method defined
 * in parameters type
 *
 * @param sym : symbol of method
 * @param call_expr : the call/inline expression
 * @param in_line : sign of inline expression
 *
 * @return : 0 - type is compatible
 *			1 - type is not compatible
 */
static int check_method_in_param(symbol_t sym, tree_t* call_expr, int in_line) {
	assert(sym != NULL); assert(call_expr != NULL);
	method_attr_t* attr = sym->attr;
	method_params_t* params = attr->method_params;
	tree_t* expr = call_expr;
	int ret = 0;

	tree_t* param_node = get_method_param_node(call_expr);
	int arg_num = (param_node == NULL) ? 0 : get_param_num(param_node);
	fprintf(stderr, "file %s, line %d\n", expr->common.location.file->name, expr->common.location.first_line);
	if ((params == NULL) && (arg_num == 0)) {
		ret = 0;
	} else if ((params == NULL) || (params->in_argc != arg_num)) {
		//error("wrong number of input arguments\n");
		int *p = NULL;
		*p = 0;
		PERRORN("wrong number of input arguments or not mehtod '%s', expect args num %p, get %d, sym %p\n", param_node, sym->name, params,  arg_num, sym);
		//ret = 1;
	} else {
		/* check the type of parameters */
		ret = check_param_type(param_node, params->in_list, params->in_argc, in_line);
	}

	return ret;
}

/**
 * @brief check_method_out_param : check the type of return method parameters is compatible with defined
 *
 * @param sym : symbol of method
 * @param ret_expr : the syntax tree node of return arguments
 * @param in_line : sign of inline expression
 *
 * @return : 1 - type of arguments is not compatible
 *			0 - type of arguments is compatible
 */
static int check_method_out_param(symbol_t sym, tree_t* ret_expr, int in_line) {
	assert(sym != NULL);
	method_attr_t* attr = sym->attr;
	method_params_t* params = attr->method_params;
	tree_t* expr = ret_expr;
	int ret = 0;

	int arg_num = get_param_num(ret_expr);
	if ((params == NULL) && (arg_num == 0)) {
		ret = 0;
	}
	else if (params->ret_argc != arg_num) {
		//error("wrong number of output arguments\n");
		PERRORN("wrong number of output arguments\n", ret_expr);
		//ret = 1;
	} else {
		ret = check_param_type(expr, params->ret_list, params->ret_argc, in_line);
	}

	return ret;
}

/**
 * @brief check_method_param : check the parameters of method
 *
 * @param sym : the symbol of method
 * @param call_expr : the syntax tree node of call expression
 * @param ret_expr : the return expression of method
 * @param in_line : this method is in inline/call expression
 *
 * @return :  1 - the type parameters of call/inline is not compatible with method parameters type
 *			0 - the type parameters of call/inline is compatible with method parameters type
 */
int check_method_param(symbol_t sym, tree_t* call_expr, tree_t* ret_expr, int in_line) {
	assert(sym != NULL);
	method_attr_t* attr = sym->attr;
	tree_t* expr = call_expr;
	tree_t* ret = ret_expr;
	int ret_value = 0;

	ret_value = check_method_in_param(sym, expr, in_line);
	if (!ret)
		ret_value = check_method_out_param(sym, ret_expr, in_line);

	return ret_value;
}

/**
 * @brief translate_call_common : translate the common part about call/inline method
 *
 * @param expr : syntax tree node of call/inline
 * @param ret : the syntax tree node of return arguments
 */
static void translate_call_common(tree_t *expr, tree_t *ret){
	//tree_t *expr = t->call_inline.expr;
	//tree_t *ret = t->call_inline.ret_args;
	tree_t *expr_list;
	symbol_t sym;
	symbol_t method_sym;
	const char *name = NULL;
	const char *obj_name;
	object_t *obj = NULL;
	tree_t *block = NULL;
	tree_t *node;
	method_attr_t *method_attr;
	ref_ret_t ref_ret;
	

	method_sym = get_call_expr_ref(expr, &ref_ret);
	if(method_sym) {
		method_attr = method_sym->attr;
		node = method_attr->common.node;	
		block = node->method.block;
		if(block_empty(block)) {
			return;
		}
	} else {
		my_DBG("method not found in call \n");
	}

	/*
	if (check_method_param(method_sym, expr, ret, 0)) {
		return;
	}

	sym = symbol_find(current_table,"exec",TMP_TYPE);
	*/
	sym = flow->exec;
	if(sym) {
		name = get_symbol_alias(sym);
	}
	//POS;
	if(method_sym && (method_sym->type == METHOD_TYPE)) {
		obj = (object_t *)method_sym->owner;
		if(!obj) {
			my_DBG("method object cannot empty\n");
		} else {
			if (!block_empty(block))
				add_object_method(obj, method_sym->name);
		}
	} else {
		my_DBG("method not right %p, %d\n", method_sym, method_sym->type);
	}
	if(sym) {
		POS;
		D("%s = ",name);
	}
	obj_name = get_obj_qname(obj);
	if(!strcmp(obj->obj_type, "device")) {
		D("_DML_M_%s", method_sym->name);	
	} else {
		D("_DML_M_%s__%s", obj_name, method_sym->name);
	}
	D("(_dev");
	if(obj->depth) {
       if(!ref_ret.index) {
           D(", _idx0");
       } else {
           D(", ");
           translate(ref_ret.index);
       }
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
	gen_src_loc(&expr->common.location);
	POS;
	D("if(%s) ",name);
	enter_scope();
	if(flow->_throw) {
		symbol_t throw = flow->_throw;
		name = get_symbol_alias(throw);	
		gen_src_loc(flow->throw_loc);
		POS;
		D("goto %s;\n", name);
	} else {
		gen_src_loc(flow->exec_loc);	
		POS;
		D("return 1;\n");
	}
	exit_scope();
}

/**
 * @brief translate_call : translate the call expression
 *
 * @param t : syntax tree node of call expression
 */
void translate_call(tree_t *t) {
	tree_t *expr = t->call_inline.expr;
	tree_t *ret = t->call_inline.ret_args;

	translate_call_common(expr, ret);
}

/**
 * @brief translate_after_call : translate the after call expression
 *
 * @param t : syntax tree node of after_call expression
 */
void translate_after_call(tree_t *t) {
	tree_t *node = t;
	tree_t *call = node->after_call.call_expr;
	/*TODO just ignore timing info */
	translate_call_common(call, NULL);
}

static void bind_tmp_type(tree_t *node, tree_t *node2) {
}

/**
 * @brief process_method_parameters : process the method parameters for generating
 *
 * @param m : the attribute of method
 * @param alias: method parameters have aliases
 */
static void process_method_parameters(method_attr_t *m, int alias) {
	int index = 0;
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
			print_all_symbol(table);
			//sym = symbol_find(table, param->var_name, PARAM_TYPE);
			sym = symbol_find_notype(table, param->var_name);
			
			if(!sym) {
				my_DBG("no sym %s found\n",param->var_name);
				fprintf(stderr, "no sym %s found, table %p\n", param->var_name, table);
				exit(-1);
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
			set_symbol_alias(sym, 0, index);
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

/**
 * @brief get_expr_type : get the node type of expression
 *
 * @param t : syntax tree node of expression
 */
static void gen_expr_type(tree_t *t) {
        if(t->common.type == CAST_TYPE) {
                translate(t->cast.ctype);
        } else if(t->common.type == IDENT_TYPE || t->common.type == UNARY_TYPE) {
                D("typeof( ");
                translate(t);
                D(" )");
        } else if(t->common.type == CONST_STRING_TYPE) {
		D("const char *");
	} else {
		D("uint32");
	}
}

static int node_has_type(tree_t *node);

/**
 * @brief process_inline_start : process the context before generating inline expression
 *
 * @param m : the mehtod of inline
 * @param input : the input parameters of method
 * @param output : the output parameters of method
 * @param context : the context of current generating
 */
static void process_inline_start(method_attr_t *m, tree_t *input, tree_t *output, context_t *context, object_t *old_obj, ref_ret_t *ref_ret) {
	tree_t *mt = m->common.node;
	tree_t *params = mt->method.params;
	tree_t *node;
	tree_t *node2;
	const char *name;
	int i = 0;
	symbol_t sym;

	node = params->params.in_params;
	node2 = input;
	/*dirty start here*/
	if(node){
		while(node && node2) {
			/*FIXME: need more general assignment translate*/
			gen_src_loc(&node2->common.location);
			POS;
			if(node_has_type(node)) {
				  translate(node);
			} else {
				  /*just get type info*/
				  //print_cdecl1(node2, 0, 1);
				  cdecl_t *cdecl = node2->common.cdecl;
				  current_table = context->saved;
				  gen_expr_type(node2);
				  current_table = context->current;
				  bind_tmp_type(node, node2);
				  name = get_cdecl_name(node);
				  sym = symbol_find_notype(context->current, name);
				  if(cdecl) {
					cdecl_t *old_cdecl = sym->attr;
					sym->type = INT_T;
				  	sym->attr = cdecl;
					print_all_symbol(context->current);
				  }
				  //sym->attr = cdecl;
				  name = get_symbol_alias(sym);
				  D(" ");
				  D("%s", name);
			}

			D(" = ");
			/*translate node2*/
			//D("%s",node2->ident.str);
			current_table = context->saved;
			object_t *obj = OBJ->obj;
			OBJ->obj = old_obj;
			translate(node2);
			current_table = context->current;
			OBJ->obj = obj;
			D(";");
			new_line();
			node = node->common.sibling;
			node2 = node2->common.sibling;
		}

	}
	if(OBJ->obj->is_array) {
		current_table = context->saved;
		object_t *obj = OBJ->obj;
		OBJ->obj = old_obj;
		POS;
		if(ref_ret->index) {
			D("int _idx0 = ");
			translate(ref_ret->index);	
			D(";");
		}
		new_line();
		current_table = context->current;
		OBJ->obj = obj;
	}
	node = params->params.ret_params;
	
	node2 = output;
	while(node && node2) {
				gen_src_loc(&node2->common.location);
				POS;
				if(node_has_type(node)) {
						translate(node);
				} else {
						current_table = context->saved;
						gen_expr_type(node2);
						current_table = context->current;
						bind_tmp_type(node, node2);
						name = get_cdecl_name(node);
						sym = symbol_find_notype(current_table, name);
						name = get_symbol_alias(sym);
						D(" ");
						D("%s", name);
				}
				D(";");
				new_line();
				node = node->common.sibling;
				node2 = node2->common.sibling;
	}
}

/**
 * @brief translate_block : translate the content of block
 *
 * @param t : syntax tree node of block
 */
void translate_block(tree_t *t) {
	tree_t *node;
	YYLTYPE *l;
	int ret;

	enter_scope();
	if(t->block.statement) {
		node = t->block.statement;
		while(node) {
			/*
			l = &node->common.location;
			ret = need_pos(node);*/
			/*gen line/file info for gcc*/
			/*
			gen_src_loc(l);
			if(ret) {
				POS;	
			}*/
			//translate(node);
			/*expression ;*/
			/*
			if(need_semicolon(node)) {
				D(";");
			}*/
			handle_one_statement(node);
			new_line();
			node = node->common.sibling;
		}
	}
	exit_scope();
}
 
/**
 * @brief translate_local : translate the local declaration
 *
 * @param t : syntax tree node of local declaration
 */
void translate_local(tree_t *t) {
	tree_t *node;
	cdecl_t *type;
	const char *name;

	node = t->local_tree.local_keyword;
	if(node) {
		/*just ignore local keyword*/	
	}
	node = t->local_tree.cdecl;
	//symbol_t sym = symbol_find_notype(current_table, name);
	//type = (cdecl_t *)sym->attr;
	//undecl(buf, type, "test" );
	//printf("%s\n", buf);
	POS;
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

/**
 * @brief is_explicit : check the syntax tree node is explicit
 *
 * @param t : syntax tree node of checking
 * @param boolean : the value of explicit
 *
 * @return : 1 - is explicit
 *			0 - not explicit
 */
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
	} else if(node->common.type == BINARY_TYPE && (node->binary.type == EQ_OP_TYPE
		|| node->binary.type == NE_OP_TYPE 
		|| node->binary.type == OR_OP_TYPE 
		|| node->binary.type == AND_OP_TYPE)
		|| node->common.type == UNARY_TYPE) {
		expr_t *expr;
		expr = check_expr(node, current_table);
		if(expr->is_const) {
			*boolean = !!expr->val->int_v.value;
			ret = 1;
		}
	}
  	
	return ret;
}

/**
 * @brief translate_if_else : translate the if else expression
 *
 * @param t : syntax tree node of if else
 */
void translate_if_else(tree_t *t) {
	tree_t *node;
	expression_t *expr = NULL;
	node = t->if_else.cond;
	int explicit;
	int ret;
	int if_block;
	symtab_t table;
	symtab_t saved;
	tree_t *if_tree;
	int i = 0;
	//expr = parse_expression(&node, current_table);
	cal:	
	ret = is_explicit(node, &explicit);
	if(ret) {
		/*explicit*/
		if(explicit) {
			node = t->if_else.if_block;
			table = t->if_else.if_table;
		} else {
			tree_t *tmp = t->if_else.else_if;
			if(tmp) {
				node = tmp->if_else.cond;
				t = tmp;
				goto cal;
			} else {
				node = t->if_else.else_block;
				table = t->if_else.else_table;
			}
		}
		if(!node) {
			return;
		}
		saved = current_table;	
		current_table = table;
		if(!is_block(node)){
			new_line();
			handle_one_statement(node);
			new_line();
		} else {
			translate(node);
		}
		current_table = saved;
		return;
	}

	if(1) {
		//POS;
		D("if ");
		D("(");
		translate(node);
		D(")");
		node = t->if_else.if_block;
		int block = if_block = is_block(node);
		if(node) {
			table = t->if_else.if_table;
			saved = current_table;
			current_table = table;
			if(!block) {
				new_line();
				handle_one_statement(node);
				new_line();
			} else {
				translate(node);
			}
			current_table = saved;
		}
		node = t->if_else.else_if;
		if(node) {
			table = node->if_else.if_table;
			saved = current_table;
			current_table = table;
			D("else ");
			translate(node);
			current_table = saved;
		} else if(t->if_else.else_block) {
			node = t->if_else.else_block;
			if(!if_block){
				POS;
			}
			D("else ");
			table = t->if_else.else_table;
			saved = current_table;
			block = is_block(node);
			current_table = table;
			if(!block) {
				new_line();
				handle_one_statement(node);
				new_line();
			} else {
				translate(node);
			}
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

/**
 * @brief translate_while : translate the while expression and its block
 *
 * @param t : syntax tree node of while expression
 */
void translate_while(tree_t *t) {
	tree_t *node = t->do_while.cond;
	POS;
	D("while ");
	D("(");
	translate(node);
	D(") ");
	symtab_t saved;

	node = t->do_while.block;
	if(is_block(node)) {
		saved = current_table;	
		current_table = get_block_table(node);
	}
	translate(node);
	if(is_block(node)) {
		current_table = saved;
	}
}

/**
 * @brief translate_comma_expr_opt : translate the comma expression
 *
 * @param t : syntax tree node of comma expression
 */
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

/**
 * @brief translate_for : translate the for expression and its block
 *
 * @param t : syntax tree node of for expression
 */
void translate_for(tree_t *t) {
	tree_t *node;
	symtab_t saved = NULL;
	
	node = t->for_tree.init;
	POS;
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
	saved = current_table;
	current_table = t->for_tree.table;
	if(node) {
	translate(node);
	if(need_semicolon(node)) {
		D(";");
	}
	} else {
		D(";");
	}
	current_table = saved;
}

/**
 * @brief translate_try_catch : translate the try catch expression
 *
 * @param t : syntax tree node of try catch
 */
void translate_try_catch(tree_t *t) {
	tree_t *node;
	symbol_t sym;	
	symbol_t sym2;
	symbol_t sym3;
	symtab_t saved;
	symbol_t exec;
	YYLTYPE *l;
	symbol_t _throw;
	YYLTYPE *throw_l;

	node = t->try_catch.try_block;
	POS;
	enter_scope();
	symbol_insert(current_table,"exec",TMP_TYPE,NULL);
	symbol_insert(current_table,"throw",TMP_TYPE,NULL);
	sym = symbol_find(current_table,"exec",TMP_TYPE);
	sym2 = symbol_find(current_table,"throw",TMP_TYPE);
	set_throw_symbol_alias(sym2);
	saved = current_table;
	exec = flow->exec;
	l = flow->exec_loc;
	_throw = flow->_throw;
	throw_l = flow->throw_loc;
	flow->exec = sym;
	flow->exec_loc = &node->common.location;
	flow->_throw = sym2;
	flow->throw_loc = &node->common.location;
	POS;
	D("bool %s = 0;\n",get_symbol_alias(sym));
	POS;
	if(is_block(node)) {
		current_table = get_block_table(node);
	}
	/*try block*/
	translate(node);
	
	current_table = saved;
	flow->exec = exec;
	flow->exec_loc = l;
	flow->_throw = _throw;
	flow->throw_loc = throw_l;
	/*catch block*/
	node = t->try_catch.catch_block;
	new_line();
	POS_n(get_tab_count() - 1);
	D("%s:",get_symbol_alias(sym2));
	new_line();
	POS;
	D("if (%s) ",get_symbol_alias(sym));
	translate(node);
	new_line();
	exit_scope();
	new_line();
	/*
	sym3 = symbol_find(current_table,"exit",TMP_TYPE);
	if(sym3) {
		POS;
		D("goto %s;",get_symbol_alias(sym3));
	}*/
}

void translate_sizeof(tree_t *t) {
	tree_t *expr;
	
	expr = t->sizeof_tree.expr;
	D("sizeof");
	translate(expr);
	D("");
}

void translate_sizeoftype(tree_t *t) {
	tree_t *tmp;

	tmp = t->sizeoftype.typeoparg;
	if(tmp->typeoparg.ctypedecl) { 
		D("sizeof( ");
		translate(tmp->typeoparg.ctypedecl);
		D(")");
	}
	else if(tmp->typeoparg.ctypedecl_brack) {
		D("sizeof( ");	
		translate(tmp->typeoparg.ctypedecl_brack);
		D(")");
	}

}

void translate_new(tree_t *t) {
	D("malloc(sizeof (");
	translate(t->new_tree.type);
	D(")");
	if(t->new_tree.count) {
		D(" * ");
		translate(t->new_tree.count);
	}	
	D(")");
}

void translate_delete(tree_t *t) {
	POS;
	D("free((void *)");	
	translate(t->delete_tree.expr);
	D(");");
}
/**
 * @brief translate_throw : translate the throw expression
 *
 * @param t : syntax tree node of throw expression
 */
void translate_throw(tree_t *t) {
	tree_t *node = t;
	symbol_t sym;
	symbol_t sym2;

	if(!flow->_throw) {
		sym = flow->exec;
		gen_src_loc(flow->exec_loc);
		POS;
		D("return 1;");
	} else {
		sym = flow->exec;
		sym2 = flow->_throw;
		POS;
		enter_scope();
		POS;
		D("%s = 1;\n",get_symbol_alias(sym));
		POS;
		D("goto %s;\n",get_symbol_alias(sym2));
		exit_scope();
	}
}

void translate_break(tree_t *t) {
	POS;
	D("break;");
}

void translate_continue(tree_t *t) {
	POS;
	D("continue;");
}

void translate_goto(tree_t *t) {
	const char *label;
	tree_t *tmp;
	
	tmp = t->goto_tree.label;
	label = tmp->ident.str;
	POS;		
	D("goto %s;", label);
}

void translate_label(tree_t *t) {
	const char *label;
	tree_t *block;

	label = t->label.ident->ident.str;	
	block = t->label.block;
	POS;
	D("%s: ", label);
	new_line();
	POS;
	translate(block);
	if(need_semicolon(block)) {
		D(";");
	}
}

/**
 * @brief translate_switch : translate the switch expression
 *
 * @param t: syntax tree node of switch
 */
void translate_switch(tree_t *t) {
	POS;
	D("switch");
	D(" ( ");
	translate(t->switch_tree.cond);
	D(" )");
	translate(t->switch_tree.block);
}

void translate_case(tree_t *t) {
	tree_t *expr;

	expr = t->case_tree.expr;
	POS;
	D("case ");
	translate(expr);
	D(" :");
	new_line();
	translate(t->case_tree.block);
	if(need_semicolon(t->case_tree.block)) {
		D(";");
	}
}

void translate_default(tree_t *t) {
	POS;
	D("default :");
	translate(t->default_tree.block);
}

/**
 * @brief get_expression_sym : get symbol of expression
 *
 * @param node : syntax tree node of expression
 *
 * @return : symbol of expression
 */
symbol_t get_expression_sym(tree_t *node) {
	symbol_t sym;
	tree_t *tmp;
	const char *name;

	if(node->common.type == QUOTE_TYPE) {
		tmp = node->quote.ident;
		name = tmp->ident.str;
		my_DBG("try to find %s\n", name);
		sym = symbol_find(current_table, name, PARAMETER_TYPE);
		return sym;
	} else if (node->common.type == COMPONENT_TYPE){
		ref_ret_t ref_ret;
		init_ref_ret(&ref_ret);
		sym = get_ref_sym(node, &ref_ret, NULL);
		return sym;
	} else {
		my_DBG("TODO: other cases \n");
		fprintf(stderr, "get expression symbol other case %d\n", node->common.type);
		
		//exit(-1);
	}
	return NULL;
}

/**
 * @brief translate_foreach : translate the foreach expression
 *
 * @param t : syntax tree node of foreach
 */
void translate_foreach(tree_t *t) {
	tree_t *node;
	const char *ident;
	int i, j = 0, k = 0;
	int len = 0;
	symbol_t list;
	symbol_t tmp = NULL;
	param_value_t *val = NULL;
	object_t *obj;
	foreach_attr_t *attr;
	symtab_t table;
	symtab_t saved;
	object_t *index_obj;

	node = t->foreach.ident;
	ident = node->ident.str;
	node = t->foreach.in_expr;
	attr = t->common.attr;
	if (attr == NULL) {
		return;
	}
	table = attr->table;
	list = get_expression_sym(node);
	if(!list) {
		return;
	}
	if(list->type != ARRAY_TYPE){
		if(list->type == PARAMETER_TYPE ) {
			val = list->attr;
			if(val->type == PARAM_TYPE_LIST) {
				len = val->u.list.size;		
				if(!len) {
					return;
				}
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
	if(val->u.list.vector[0].type == PARAM_TYPE_REF) {
		symbol_set_type(tmp, OBJECT_TYPE);
	}
	POS;
	enter_scope();
	/*{*/
	for(i = 0; i < len; i++) {
		symbol_set_value(tmp, val->u.list.vector[i].u.ref);
		obj = (object_t *)val->u.list.vector[i].u.ref;
		my_DBG("--------object %s\n", obj->name);
		node = t->foreach.block;
		YYLTYPE *loc = &node->common.location;
		if(obj->is_array) {
			POS;
			enter_scope();
			k = obj->depth;
			j = 0;
			/*gen index var*/
			while(j < k) {
				gen_src_loc(loc);
				POS;
				D("int _idx%d;\n", j);
				POS;
				D("UNUSED(_idx%d);\n", j);
				j++;
			}
			j = 0;
			while(j < k) {
				gen_src_loc(loc);
				POS;
				index_obj  = find_index_obj(obj, j+1);
				D("for(_idx%d = 0; _idx%d < %d; _idx%d++)", j, j, index_obj->array_size, j);
				enter_scope();
				j++;
			}
		}
		/*
		gen_src_loc(loc);
		if(is_block(node)) {
			POS;
		}
		translate(node);
		*/
		handle_one_statement(node);
		new_line();
		if(obj->is_array) {
			j = 0;
			while(j < k) {
				exit_scope();
				new_line();
				j++;
			}
			exit_scope();
			new_line();
		}
		/*
		if (node->common.type == BLOCK_TYPE) {
			new_line();
			exit_scope();
			new_line();
			POS;
		} else {
			exit_scope();
			new_line();
		}*/
	}
	symbol_set_type(tmp, FOREACH_TYPE);
	exit_scope();
	current_table = saved; 
}


void translate_select(tree_t *t) {
	const char *var;
	symbol_t list;
	tree_t *list_expr;
	tree_t *cond;
	tree_t *ident;
	tree_t *if_statement;
	tree_t *else_statement;
	symbol_t tmp = NULL;
	param_value_t *val = NULL;
	int len = 0;
	symtab_t saved;
	int i;
	expr_t *expr_val;
	
	list_expr = t->select.in_expr;
	cond = t->select.cond;
	ident = t->select.ident;
	if_statement = t->select.where_block;
	else_statement = t->select.else_block;
	var = ident->ident.str;
	list = get_expression_sym(list_expr);
	if(!list) 
		return;
	if(list->type != ARRAY_TYPE){
		if(list->type == PARAMETER_TYPE ) {
			val = list->attr;
			if(val->type == PARAM_TYPE_LIST) {
				len = val->u.list.size;		
				if(!len) {
					return;
				}
			} else {
				my_DBG("error type in foreach\n");
			}
		} else {
			my_DBG("error type in foreach\n");
		}
	}
	saved = current_table;
	current_table = t->select.where_table; 
	tmp = symbol_find(current_table, var, SELECT_TYPE);
	if(val->u.list.vector[0].type == PARAM_TYPE_REF) {
		symbol_set_type(tmp, OBJECT_TYPE);
	}
	POS;
	enter_scope();
	for(i = 0; i < len; i++) {
		symbol_set_value(tmp, val->u.list.vector[i].u.ref);
		expr_val = check_expr(cond, current_table);
		if(expr_val->is_const) {
			if(expr_val->val->int_v.value) {
				POS;
				translate(if_statement);
				new_line();
			} else {
				POS;
				translate(else_statement);	
				new_line();
			}
		} else {
			fprintf(stderr, "expect const expression\n");
		}
	}
	current_table = saved; 	
	symbol_set_type(tmp, SELECT_TYPE);
	exit_scope();
}

/**
 * @brief process_inline_block : translate the block of inline method
 *
 * @param m : method of inline
 */
static void process_inline_block(method_attr_t *m) {
	tree_t *method = m->common.node;
	tree_t *block = method->method.block;
	POS;
	translate(block);
	return;
}

/**
 * @brief process_inline_end : process the context after generating inline expression
 *
 * @param m : the method of inline
 * @param output : the output parameters of method
 * @param context : the context of current generating
 */
static void process_inline_end(method_attr_t *m, tree_t *output, context_t *context) {
	tree_t *mt = m->common.node;
	tree_t *params = mt->method.params;
	tree_t *node;
	tree_t *node2;
	const char *name;
	symbol_t sym;


	node = params->params.ret_params;
	node2 = output;
	while(node && node2) {
		 current_table = context->saved;
		 gen_src_loc(&node2->common.location);
		 POS;
		 translate(node2);
		 current_table = context->current;
		 D(" = ");
		 name = get_cdecl_name(node);
		 sym = symbol_find_notype(context->current, name);
		 name = get_symbol_alias(sym);
		 D("%s;", name);
		 new_line();
		 node = node->common.sibling;
		 node2 = node2->common.sibling;
	}
	return;
}

/**
 * @brief translate_inline : translate the in_line expression
 *
 * @param t : syntax tree node of inline
 */
void translate_inline(tree_t *t) {
	tree_t *expr = t->call_inline.expr;
	tree_t *ret = t->call_inline.ret_args;
	symbol_t sym;
	symtab_t table;
	object_t *saved_obj;
	symtab_t saved_table;
	ref_ret_t dummy;
	context_t context;
	tree_t *call_expr;
	object_t *old_obj;
	method_attr_t *method_attr;
	tree_t *block;
	
	if(expr->common.type == EXPR_BRACK_TYPE) {
		call_expr = expr->expr_brack.expr;
	} else {
		call_expr = expr;
	}
	sym = get_call_expr_ref(expr, &dummy);
	if(sym) {
		method_attr = sym->attr;
		tree_t *node = method_attr->common.node;	
		block = node->method.block;
		if(block_empty(block)) {
			return;
		}
	} else {
		my_DBG("sym not found\n");
	}
	/*
	if(sym->owner != OBJ) {
		saved_obj = OBJ;				
	}
	saved_table = current_table;
	*/
	/*get sym info*/
	//translate(expr);
	tree_t *expr_list = NULL;
	if(expr->common.type == EXPR_BRACK_TYPE) {
		expr_list = expr->expr_brack.expr_in_brack; 
	}
	method_attr_t *method = sym->attr;
	//object_t *obj = SYM->attr;
	process_method_parameters(method, 1);
	/*
	table = method->table;
	table->parent = current_table;
	current_table = table;
	*/
	/* In inline, the method can be a marco, it will be to expand the
	 * method code of the calles method at the place of inline call,
	 * so we should change the method table's parent to the current_table,
	 * and then check the number and type about method parameters,
	 * at last, goto the method block to parsing elements and so on*/
	/*
	if (check_method_param(sym, expr, ret, 1))
		return;
	*/
	obj_ref_t obj_ref;
	obj_ref.obj = (object_t *)sym->owner;
	/*need to get this ref*/
	obj_ref.ref = NULL; 
	old_obj = OBJ->obj;
	pre_gen_method(&obj_ref, method->common.node, &context);
	POS;
	enter_scope();
	process_inline_start(method, expr_list, ret, &context, old_obj, &dummy);
	process_inline_block(method);
	D("\n");
	process_inline_end(method, ret, &context);
	post_gen_method(&obj_ref, method->common.node, &context);
	D("\n");
	exit_scope();
	return;
}

/**
 * @brief translate_typeof : translate the typeof expression
 *
 * @param t : syntax tree node of typeof expression
 */
void translate_typeof(tree_t *t) {
	tree_t *node = t;
	tree_t *expr = node->typeof_tree.expr;
	ref_ret_t ref_ret;
	symbol_t sym;
	const char *name = NULL;
	object_t *obj;
	
	init_ref_ret(&ref_ret);
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
		sym = get_ref_sym(expr, &ref_ret, NULL);
		if(sym) {
			name = sym->name;
		} else {
			exit(-1);
		}
		if(!strcmp(name, "this")) {
			if(OBJ->obj->is_abstract) {
				my_DBG("fatal error, cannot use abstract object %s\n", OBJ->obj->name);				
			}
			D("%s", OBJ->obj->attr_type);
		} else if(sym->type == OBJECT_TYPE){
			obj = (object_t *)sym->attr;
			D("%s", obj->attr_type);
		}
	} else {
		my_DBG("other typeof \n");
	}
}

/**
 * @brief translate_cdecl : translate the c declaration
 *
 * @param t : the syntax tree node of cdeclaration
 */
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

/**
 * @brief translate_ctypedecl : translate the c type declaration
 *
 * @param t : syntax tree node of c type declaration
 */
void translate_ctypedecl(tree_t *t) {
        if (t->ctypedecl.const_opt) {
                D("const ");
        }
        int old_no_star = no_star;
        int old_no_alias = no_alias;
        base_type = 1;
        translate(t->ctypedecl.basetype);
        D(" ");
        base_type = 0;
        no_star = old_no_star;
        no_alias = old_no_alias;
        my_DBG("here decl2\n");
        translate(t->ctypedecl.ctypedecl_ptr);
}

/**
 * @brief translate_ctypedecl_ptr : translate the pointer of c type declaration
 *
 * @param t : syntax tree node
 */
void translate_ctypedecl_ptr(tree_t *t) {
        if(t->ctypedecl_ptr.stars) {
                translate(t->ctypedecl_ptr.stars);
        }
        D(" ");
        if(t->ctypedecl_ptr.array) {
                translate(t->ctypedecl_ptr.array);
        }
}

/**
 * @brief translate_stars : translate the start
 *
 * @param t : syntax tree node of start
 */
void translate_stars(tree_t *t) {
        D("*");
        if(t->stars.is_const) {
                D(" const ");
        }
        if(t->stars.stars) {
                translate(t->stars.stars);
        }
}

/**
 * @brief print_basetype : generate the code of basetype
 *
 * @param node : tree node of basetype
 */
static void print_basetype(tree_t *node) {
	const char *name;

	if(node->common.type == IDENT_TYPE) {
		my_DBG("name %s\n", node->ident.str);
		D("%s", node->ident.str);
	} else if(node->common.type == C_KEYWORD_TYPE) {
		D("%s",node->ident.str);
	} else if(node->common.type == TYPEOF_TYPE) {
		translate(node);
        }else if(node->common.type == STRUCT_TYPE){
		D("struct { ");	
		tree_t *cdecls = node->struct_tree.block;
		tree_t *tmp = cdecls;
		while(tmp) {
			print_cdeclx(tmp);	
			D("; ");
			tmp = tmp->common.sibling;
		}
		D("}");
	} else {
		my_DBG("other base type\n");
	}
}

/**
 * @brief get_basetype_type : get the string of basetype
 *
 * @param node : syntax tree node of basetype
 *
 * @return : string of basetype
 */
static const char *get_basetype_type(tree_t *node) {
	const char *name = NULL;
	
	if(node->common.type == IDENT_TYPE) {
		name = node->ident.str;
	} else if(node->common.type == C_KEYWORD_TYPE) {
		name = node->ident.str;
	}else if(node->common.type == DML_KEYWORD_TYPE) {
		name = node->ident.str;
	}else {
		my_DBG("other base type \"%s\"\n", TYPENAME(node->common.type));
	}
	return name;
}

static void print_cdecl2(tree_t *t, int ret);

/**
 * @brief print_cdecl1 : generate the code of the first style of c declaration
 *
 * @param t : syntax tree node of declaration
 * @param ret : this c declaration is in method return parameter
 */
static void  print_cdecl1(tree_t *t, int ret) {
	if(t->cdecl.is_const) {
		D("const ");
	}
	if(t->cdecl.basetype && !t->cdecl.decl) {
		D("uint32 ");
		if(ret) {
			D("*");
		}
		const char *name = t->cdecl.basetype->ident.str;
		D("%s", name);
		return;
	}
	print_basetype(t->cdecl.basetype);
	D(" ");
	if(ret) {
		//D("*");
	}
	if (t->cdecl.decl)
		print_cdecl2(t->cdecl.decl, ret);
}

void print_cdeclx(tree_t *node) {
	print_cdecl1(node, 0);
}
/**
 * @brief node_has_type : check method parameters have type or not
 *
 * @param node : syntax tree node of method parameters
 *
 * @return : 1 - have type
 *			0 - not have type
 */
static int node_has_type(tree_t *node) {
	if(node->cdecl.basetype && node->cdecl.decl) {
			return 1;
	} else {
			return 0;
	}
}

static const char *get_cdecl2_name(tree_t *t);

/**
 * @brief get_cdecl_name : get the variable name of declaration
 *
 * @param node: syntax tree node of declaration
 *
 * @return : the variable name of declaration
 */
const char *get_cdecl_name(tree_t *node) {
	if(!node->cdecl.decl && node->cdecl.basetype) {
			return  get_basetype_type(node->cdecl.basetype);
	}
	return get_cdecl2_name(node->cdecl.decl);
}

/**
 * @brief get_type_info : get the type of declaration
 *
 * @param node : node of declaration
 *
 * @return  : string of type
 */
const char *get_type_info(tree_t *node) {
	if(node->cdecl.is_const) {
		my_DBG("TODO get type info:");
	}
	return get_basetype_type(node->cdecl.basetype);
}

/**
 * @brief block_empty : check the method have block or not
 *
 * @param t : syntax tree node of method block
 *
 * @return : 1 - empty block
 *			0 - not empty block
 */
int block_empty(tree_t *t) {
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

void translate_data(tree_t *t) {
	translate_ident(t);
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

void translate_string(tree_t *t) {
	D("%s", t->string.pointer);
}

void translate_c_keyword(tree_t *t) {
	D("%s", t->ident.str);
}

/**
 * @brief translate_log : translate the log
 *
 * @param t : syntax tree node of log
 */
void translate_log(tree_t *t) {
	tree_t *node = t;
	const char *log_type;

	log_type = node->log.log_type;
	//my_DBG("log args %s\n", node->log.args);
	if(!strcmp(log_type, "\"error\"")) {
		//D("SIM_log_err(&_dev->obj.qdev, 0, %s, %s);", node->log.format, node->log.args);		
	}
	D(";");
}

void translate_error(tree_t *t) {
	D(";");
}

/**
 * @brief translate_ident : translate the indentifier
 *
 * @param t : syntax tree node of identifier
 */
void translate_ident(tree_t *t) {
	const char *name = t->ident.str;
	const char *alias_name;
	cdecl_t *type;
	
	fprintf(stderr, "current table %d, parent %d, name %s\n", current_table->table_num, current_table->parent->table_num, name);
	//print_all_symbol(current_table);
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
	} else {
		type = (cdecl_t *)sym->attr;
		if(sym->type == STRUCT_TYPE || sym->type == TYPEDEF_T){
			D("%s\n", sym->name);
			set_symbol_alias_name_force(sym, sym->name);
			return;
		}
	}
	if(sym && sym->type == CONSTANT_TYPE) {
		constant_attr_t *attr = (constant_attr_t *)sym->attr;	
		expr_t *expr = attr->value;
		int tmp = expr->val->int_v.value;
		D("0x%x", tmp);
		return;
	} 
	if(no_alias) {
		if(no_star) {
			D("%s",name);
		} else {
			D("*%s",name);
		}
	} else {
		alias_name = get_symbol_alias(sym);
		D("%s",alias_name, sym->type);	
	}
}

/**
 * @brief translate_cdecl2 : translate the second style of c declaration, this name is
 * based on dml language
 *
 * @param t : syntax tree node of c declaration
 */
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

static void print_cdecl3(tree_t *node, int ret);

/**
 * @brief print_cdecl2 : generate the code of the second style of c declaration
 *
 * @param t : the syntax tree node of declaration
 * @param ret : the c declaration is in method returning declaration
 */
static void print_cdecl2(tree_t *t, int ret) {
	if(t->common.type == CDECL_TYPE) {
		if(t->cdecl.is_const) {
			D("const");
			print_cdecl2(t->cdecl.decl, ret);
		}else if(t->cdecl.is_point) {
			D("*");
			my_DBG("*\n");
			print_cdecl2(t->cdecl.decl, ret);
		}else if (t->cdecl.is_vect) {
			/*vect type */
			my_DBG("TODO: vect type\n");
		}
	} else {
		my_DBG("cdecl3\n");
		print_cdecl3(t, ret);
	}
}

static const char *get_cdecl3_name(tree_t *node);

/**
 * @brief get_cdecl2_name : get the variable name of the second style c declaration
 *
 * @param t : syntax tree node of declaration
 *
 * @return : variable name of declaration
 */
static const char *get_cdecl2_name(tree_t *t) {
	if(t->common.type == CDECL_TYPE) {
		return get_cdecl2_name(t->cdecl.decl);
	} else {
		return get_cdecl3_name(t);
	}
	return NULL;
}

static void print_cdecl_list(tree_t *t) {
	tree_t *it = t;
	if(!t) {
		return;
	}
	while(it){
		if(it->common.type == ELLIPSIS_TYPE) {
			D("...");
		} else if (it->common.type == CDECL_TYPE) {
			print_cdeclx(it);
		}
		if(it->common.sibling){
			D(", ");
		}
		it = it->common.sibling;
	}
}

/**
 * @brief print_cdecl3 : generate the code of the third style of c declaration
 *
 * @param t : the syntax tree node of declaration
 * @param ret : the c declaration is in method returning declaration
 */
static void print_cdecl3(tree_t *t, int ret) {
	if (t->common.type == IDENT_TYPE || t->common.type == DML_KEYWORD_TYPE) {
		if(ret) {
			D("*");
		}
		D("%s", t->ident.str);
	} else if(t->common.type == ARRAY_TYPE){
		print_cdecl3(t->array.decl, ret);
		D("[");
		translate(t->array.expr);	
		D("]");
	} else if(t->common.type == CDECL_BRACK_TYPE){
		if(t->cdecl_brack.is_list) {
			print_cdecl3(t->cdecl_brack.cdecl, ret);
			D("(");
			print_cdecl_list(t->cdecl_brack.decl_list);
			D(")");	
		} else {
			D("(");
			print_cdecl2(t->cdecl_brack.decl_list, ret);
			D(")");
		}
	} else {
		my_DBG("TODO: other cdecl3 type\n");
	}
}

/**
 * @brief get_cdecl3_name : get the variable name from the third style c declaration
 *
 * @param t : syntax tree node of declaration
 *
 * @return : variable name
 */
static const char *get_cdecl3_name(tree_t *t) {
	if (t->common.type == IDENT_TYPE || t->common.type == DML_KEYWORD_TYPE) {
		return t->ident.str;
	} else {
		my_DBG("TODO: get_name:other cdecl3 type\n");
	}
	return NULL;
}

/**
 * @brief translate_cdecl3_array : translate the array in the third c declaration
 *
 * @param t : syntax tree node of declaration
 */
void translate_cdecl3_array(tree_t *t) {
	translate(t->array.decl);
	D("[");
	translate(t->array.expr);
	D("]");
}

/**
 * @brief translate_decl_list : translate the list of declaration
 *
 * @param t : syntax tree node of list declaration
 */
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

/**
 * @brief translate_cdecl3_brack : translate the declaration in brack
 *
 * @param t : syntax tree node of declaration
 */
static void translate_cdecl3_brack(tree_t *t) {
	if(t->cdecl_brack.cdecl){
		translate(t->cdecl_brack.cdecl);
	}
	D("(");
	translate_decl_list(t->cdecl_brack.decl_list);
	D(")");
}

void translate_return(tree_t *t) {
	D("return 0");
}

/**
 * @brief do_method_param_alias : make the aliases for method parameter
 *
 * @param t : the syntax tree node of parameter
 * @param ret : sign of return parameter
 */
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
			set_symbol_alias_name_force(sym, dup);	
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
		fprintf(stderr, "symbol not found \n");
		exit(-1);
	}
}

/**
 * @brief translate_cdecl_or_ident_list : generate the code of parameters of method
 *
 * @param t : the syntax tree node of method parameters
 * @param ret : sign for return parameters
 */
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

/**
 * @brief cdecl_or_ident_list_params_alias : make parameters aliases based on parameter list
 *
 * @param t : syntax tree node of method parameter
 * @param ret : sign of return parameters
 */
void cdecl_or_ident_list_params_alias(tree_t *t, int ret) {
	tree_t *node = t;

	while(node) {
		do_method_param_alias(node, ret);
		node = node->common.sibling;
	}
}

/**
 * @brief gen_method_params : generate the parameters about method
 *
 * @param obj : the object owning method
 * @param m : syntax tree node of method
 */
void gen_method_params(object_t *obj, tree_t *m, int gen_ret){
	tree_t *params = m->method.params;
 	
	if(params) {
		if(params->params.in_params){
			D(", ");
			/*gen in_params*/
			translate_cdecl_or_ident_list(params->params.in_params, 0);
		}
		if(gen_ret && params->params.ret_params){
			D(", ");
			/*gen ret_params*/
			translate_cdecl_or_ident_list(params->params.ret_params, 1);
		} 
	}
	D(")");
}

/**
 * @brief do_method_params_alias : entry to make aliases for method parameters
 *
 * @param obj : the object owning method
 * @param m : syntax tree node of method
 */
void do_method_params_alias(object_t *obj, tree_t *m){
	tree_t *params = m->method.params;
	const char *name;
	if(obj->is_array) {
		name = obj->a_name;
	} else {
		name = obj->name;
	}
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

/**
 * @brief gen_method_entry : entry to generate method code
 *
 * @param obj : object owning method
 * @param m : syntax tree node of method
 */
static void gen_method_entry(object_t *obj, tree_t *m) {
	tree_t *block;
	int has_return;
	const char *name = NULL;

	block = m->method.block;
	has_return = method_has_return(m);
	if(has_return) {
		symbol_insert(current_table, "exit", TMP_TYPE, NULL); 
		symbol_t sym = symbol_find(current_table, "exit", TMP_TYPE);
		set_exit_symbol_alias(sym);
		name = get_symbol_alias(sym);
		if(flow) {
			symbol_t old_exit = flow->exit;
			YYLTYPE *old_loc = flow->exit_loc;
			flow->exit = sym;
		}
		POS;
		enter_scope();
	}
	POS;
	translate(block);
	new_line();
	if(has_return){
		int tab_count = get_tab_count();
		int pos = tab_count - 1;
		D_n(pos, "%s: ;\n", name);		
		exit_scope();
		new_line();
	}
}

/**
 * @brief do_block_logic : generate the logical code of method
 *
 * @param obj : object owning method
 * @param m : syntax tree node of method
 */
void do_block_logic(object_t *obj, tree_t *m) {
	tree_t *block;
	symbol_t sym = NULL;
	const char *name;
	int has_exec;
	
	block = m->method.block;
	has_exec = method_has_exec(m);
	if(has_exec) {
		symbol_insert(current_table, "exec", TMP_TYPE, NULL); 
		sym = symbol_find(current_table, "exec", TMP_TYPE);
		flow_ctrl_t l_flow;
		l_flow.exec = sym;
		POS;	
		enter_scope();
		l_flow.exec_loc = &block->common.location;
		flow = &l_flow;
		flow->_throw = NULL;
		flow->throw_loc = NULL;
		name = get_symbol_alias(sym);
		//tabcount_add(1);
		gen_src_loc(&block->common.location);
		POS;
		D("bool %s = 0;\n",name);
		POS;
		D("UNUSED(%s);\n", name);
	}
	/*
	POS;
	translate(block);
	new_line();
	*/
	gen_method_entry(obj, m);
	if(has_exec){
		exit_scope();
		new_line();
	}
}

/**
 * @brief gen_dummy_block : generate the code of method with empty method block
 *
 * @param block : syntax tree node of method block
 */
static void gen_dummy_block(tree_t *block) {
		enter_scope();
		gen_src_loc(&block->common.location);
		POS;
		D("return 0;\n");
		exit_scope();
		new_line();
}

/**
 * @brief gen_method_block : generate the code of method block
 *
 * @param obj : object of owning method
 * @param m : syntax tree node of method
 */
static void gen_method_block(object_t *obj, tree_t *m){
	tree_t *block = m->method.block;
	symbol_t sym;

	if(block_empty(block)) {
		gen_dummy_block(block);
		return;
	}
	new_line();
	POS;
	enter_scope();
	/*{\n*/
	D_HEAD;
	POS;
	enter_scope();
	/*	{\n*/
	POS;
	enter_scope();
	/*		{\n*/
	do_block_logic(obj, m);
	D_END;
	exit_scope();
	new_line();
	/*		}\n*/
	gen_src_loc(&block->common.location);
	POS;
	D("return 0;\n");
	exit_scope();
	new_line();
	/*	}\n*/
	exit_scope();
	new_line();
	/*}\n*/
}

/**
 * @brief gen_object_index : generate the index of object
 *
 * @param obj : the object to be generated
 */
static void gen_object_index(object_t *obj) {
	int i; 
	
	for(i = 0; i < obj->depth; i++) {
		D(", int _idx%d", i);
	}
}

/**
 * @brief gen_dml_method_header : generate the header of method
 *
 * @param obj : the object owning method
 * @param m : syntax tree node of method
 */
void gen_dml_method_header(object_t *obj, tree_t *m) {
	method_attr_t *attr;

	attr = m->common.attr;
	if(obj->encoding == Obj_Type_Device) {
		/*for global method no, array involved*/
		D("static bool\
			\n_DML_M_%s(%s_t *_dev", m->method.name, DEV->name);
	} else {
			D("static bool\
				\n_DML_M_%s__%s(%s_t *_dev", obj->qname, m->method.name, DEV->name);
			gen_object_index(obj);
	}
	gen_method_params(obj, m, 1);
}

void gen_extern_dml_method_header(object_t *obj, tree_t *m) {

}
/*need to implemention object ref */
static void change_object(object_t *obj) {
}

/*back to old object*/
static void restore_object(object_t *obj) {
}

/**
 * @brief context_switch_to : change the context with method
 *
 * @param context : the current context of generating code
 * @param obj : the object of method owned
 * @param to : the symbol table of method
 * @param method_parent : the parent table of method
 */
static inline void context_switch_to(context_t *context, obj_ref_t *obj, symtab_t to, symtab_t method_parent) {
	context->current = to;
	context->saved = current_table;
	context->method_parent = method_parent;
	context->obj = OBJ;
	current_table = to;
	OBJ = obj;
	change_object(obj->obj);
}

/**
 * @brief context_restore : restore the context of generating code
 *
 * @param context : the context of generating code
 * @param old : the old symbol table
 */
static inline void context_restore(context_t *context, symtab_t *old) {
	current_table = context->saved;
	OBJ = context->obj;
	*old = context->method_parent;
	/* temporary comment by eJim Lee */
	//restore_object(OBJ->obj);
}

/**
 * @brief pre_gen_method : get some information about context from method
 *
 * @param obj : the object that has method
 * @param method : syntax tree node of method
 * @param context : the context of generating code
 */
void pre_gen_method(obj_ref_t *obj, tree_t *method, context_t *context) {
	method_attr_t *attr;
	symtab_t table;
	symtab_t old_table;

	attr = method->common.attr;
	table = attr->table;
	old_table = table->parent;
	table->parent = obj->obj->symtab;
	context_switch_to(context, obj, table, old_table);
	my_DBG("method name %s, obj %s, table num %d\n", method->method.name, obj->obj->name, table->table_num);
	OBJ = obj;
}

/**
 * @brief post_gen_method : do some post work after method generated
 *
 * @param obj : the object of owning method
 * @param method : syntax tree node of method
 * @param context : the context of generating code
 */
void post_gen_method(obj_ref_t *obj, tree_t *method, context_t *context) {
	method_attr_t *attr;
	symtab_t table;

	attr = method->common.attr;
	table = attr->table;
	context_restore(context, &table->parent);
}

/**
 * @brief gen_object_info : generate the object information of method
 *
 * @param ref : the object of owning method
 * @param m : the struct of method
 */
static void gen_object_info(obj_ref_t *ref, struct method_name *m) {
	const char *name;
	char buf[256];

	if(!ref->ref && ref->obj) {
		if(ref->obj->encoding == Obj_Type_Device) {
			D("/* %s */\n", m->name);
			return;
		} else {
			name = get_obj_ref(ref->obj);
			sscanf(name, "_dev->%s", buf);
			D("/* %s.%s */\n", buf, m->name);
		}
	} else {
		/*gen ref info*/
	}
}

extern void print_method (tree_t* node, int pos);

/**
 * @brief gen_dml_method : generate the code of method
 *
 * @param obj : the object of method
 * @param m : the method struct
 */
void gen_dml_method(object_t *obj, struct method_name *m) {
	tree_t *method = m->method;
	context_t context;
	obj_ref_t obj_ref;

	/*set up entry condition*/
	obj_ref.obj = obj;
	obj_ref.ref = NULL;
	OBJ = NULL;
	current_table = obj->symtab;

	/* we should pase the elements and calcualate the
	 * expressions that in the method block, as we did
	 * not do them before */
	pre_gen_method(&obj_ref, method, &context);
	//parse_method_block(method);
	tabcount_set(0);
	D("\n");
	gen_src_loc(&method->common.location);
	gen_object_info(&obj_ref, m);
	gen_dml_method_header(obj, method);
	do_method_params_alias(obj, method);
	gen_method_block(obj, method);	
	post_gen_method(&obj_ref, method, &context);
}

