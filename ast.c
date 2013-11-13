
/* Copyright (C) 
 * 2012 - Michael.Kang blackfin.kang@gmail.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/**
 * @file ast.c
 * @brief
 * @author Michael.Kang blackfin.kang@gmail.com
 * @version 7849
 * @date 2012-12-07
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "ast.h"
#include "debug_color.h"
#include "types.h"
#include "symbol.h"
#include "decl_func.h"
#include "info_output.h"
#include "object.h"
#include "gen_common.h"
#include "chk_common.h"

extern symtab_t current_table;

#ifdef AST_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

/**
 * @brief gdml_zmalloc : allocates size bytes and
 * initialize it with 0, at last returns a pointer
 * to the allocated memory
 *
 * @param size: allocates size
 *
 * @return : return a pointer to the allocated memory
 */
void* gdml_zmalloc(int size) {
	void* ret = malloc(size);
	if (ret == NULL) {
		fprintf(stderr, "malloc failed : %s!\n", strerror(errno));
		exit(-1);
	}
	memset(ret, 0, size);

	return ret;
}

/**
 * @brief get_digits: get the digits from string
 *
 * @param str: the string of digits
 * @param base: the carinal number of  digits: 16,10,8
 *
 * @return: the integer of digits
 */
long long get_digits (char *str, int base)
{
	char *endptr = NULL;
	long long value = 0;

	/* To distinguish success/failure after call */
	errno = 0;

	value = strtoll (str, &endptr, base);

	/* Check for various possible errors */
	if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
		|| (errno != 0 && value == 0)) {
		perror ("strtoll");
		exit (EXIT_FAILURE);
	}

	if (endptr == str) {
//		fprintf (stderr, "No digits were found\n");
//		exit (EXIT_FAILURE);
		error("No digits were found");
	}

	if (*endptr != '\0') {
//		fprintf (stderr, "Further characters after number(%s : %s)\n", str,
//				 endptr);
//		exit (EXIT_FAILURE);
		error("Further characters after number (%s, %s)", str, endptr);
	}

	return value;
}

/**
 * @brief strtoi: change the data string to data
 *
 * @param str: the string of data
 *
 * @return: the data of decimal
 */
long long strtoi (char *str)
{
	char *stop = NULL;
	char *first_pos = NULL;
	long long value = 0;
	int length = 0;

	if (str == NULL) {
//		fprintf (stderr, "The string changed to int is NULL\n");
//		return -1;
		error("the string changed to integer isn't exist");
	}

	if ((strncmp(str, "0x", 2) == 0) ||
			(strncmp(str, "0X", 2) == 0)) {
		value = get_digits (str, 16);
	}//hex
	else if ((strncmp(str, "0b", 2) == 0) ||
			(strncmp(str, "0B", 2) == 0)) {
		value = get_digits (&(str[2]), 2);
	}//binary
	else if (str[0] == '0') {
		int i = 1;
		for(i = 0; i < strlen(str); i++) {
			if ((str[i] < '0') || (str[i] > '7')) {
//				fprintf(stderr, "Wrong Octal : %s\n", str);
				error("Wrong Octal \"%s\"", str);
				/* TODO: handle the error */
//				exit(-1);
			}
		}
		value = get_digits(str, 8);
	}//octal
	else {
		value = get_digits (str, 10);
	}//decimal

	return value;
}

/**
 * @brief find_tail : find the last node from list
 *
 * @param head : the head node about list
 *
 * @return : return the pointer to the last node
 */
static tree_t* find_tail (tree_t* head)
{
	tree_t* it = head;
	while (it->common.sibling != NULL) {
		it = it->common.sibling;
		DEBUG_FIND_TAIL ("In %s, head=0x%x, it=0x%x\n", __FUNCTION__, head, it);
	}
	return it;
}

/**
 * @brief find_node: find the node with node type
 *
 * @param node: the entrance node of finding
 * @param type: the type of node that be found
 *
 * @return: return the found node
 */
tree_t* find_node (tree_t* node, int type)
{
	if (node == NULL) {
		return NULL;
	}
	if ((node->common.type) == type) {
		DBG ("node find, name: %s\n", node->name);
		return node;
	}
	else if (((node->common.child) != NULL) && ((node->common.type) != type)) {
		return find_node (node->common.child, type);
	}
	else if (((node->common.sibling) != NULL) && ((node->common.type) != type)) {
		return find_node (node->common.sibling, type);
	}
	else {
//		printf ("Not find the node.\n");
		PWARN("Not find the node (target: %s)", node->common.location, TYPENAME(type));
		return NULL;
	}
}

/**
 * @brief add_child : add a child node to parent
 *
 * @param parent : parent node
 * @param child : child node
 */
void add_child (tree_t* parent, tree_t* child)
{
	assert (parent != NULL);
	DEBUG_ADD_CHILD ("In %s, child->name=%s\n", __FUNCTION__, child->common.name);
	parent->common.child = child;
}

void dummy_translate(tree_t *node) {
#ifndef RELEASE
	printf("not correct, should call you own translate function, node name %s\n", node->common.name);
#endif
}
static int node_num = 0;

/**
 * @brief create_node : create a tree node
 *
 * @param name : node name
 * @param type : node type
 * @param size : node size
 * @param location : node location
 *
 * @return : return a pointer to created node
 */
tree_t* create_node (const char *name, int type, int size, YYLTYPE* location)
{
	tree_t* node = (tree_t*)gdml_zmalloc(size) ;
	node->common.name = strdup (name);
	node->common.sibling = NULL;
	node->common.child = NULL;
	node->common.translate = dummy_translate;
	DEBUG_CREATE_NODE ("In %s, node: 0x%x, name=%s, type = %d\n", __FUNCTION__, node, name, type);
	node->common.type = type;
	if(location) {
		node->common.location = *location;
	}
	node_num++;

	return node;
}

static int check_object_place(symtab_t table, int obj_type) {
	assert(table != NULL);
	int type = table->type;
	int ret = 0;

   /* In simics, there are some rules about object:
	* 1. attribute, bank, implement, connect may only appear in device
	* 2. register only appear in bank group and template
	* 3. field may only appear in register and template
	* 4. interface may only appear in connect and template */
	switch(obj_type) {
		case BANK_TYPE:
		case ATTRIBUTE_TYPE:
		case CONNECT_TYPE:
		case IMPLEMENT_TYPE:
			ret = ((type == DEVICE_TYPE) || (type == TEMPLATE_TYPE) || (type == PORT_TYPE)) ? 0 : 1;
			break;
		case REGISTER_TYPE:
			ret = (type == BANK_TYPE || type == GROUP_TYPE || type == TEMPLATE_TYPE) ? 0 : 1;
			break;
		case FIELD_TYPE:
			ret = (type == REGISTER_TYPE || type == TEMPLATE_TYPE) ? 0 : 1;
			break;
		case INTERFACE_TYPE:
			ret = (type == CONNECT_TYPE || type == TEMPLATE_TYPE) ? 0 : 1;
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}

object_attr_t* create_object(symtab_t table, const char* node_name, const char* symbol_name, int type, int node_size, int attr_size, YYLTYPE* location) {
   assert(table != NULL);
   assert(node_name != NULL);
   assert(symbol_name != NULL);

   object_attr_t* attr = NULL;
   tree_t* node = NULL;

   /* in simics, some object can appear in speial place */
   if (check_object_place(table, type)) {
	   error("this object '%s' is not allowed here\n", symbol_name);
	   return NULL;
   }

   /* In simics, an object can defined many times, so we should check one
    * object is defined before, if not, create new, if defined, get it's
    * node and attribute, and add other default parameter, and also get it's
    * symbol table, insert other parameters and variables into table, but it's
    * default parameters can defined only once, such as: size, desc,etc*/
   if (symbol_defined(table, symbol_name)) {
       symbol_t symbol = symbol_find(table, symbol_name, type);
       if (symbol == NULL) {
           fprintf(stderr, "error: name collision on '%s'\n", symbol_name);
           /* TODO: handle the error */
           exit(-1);
           return NULL;
       }// usr defined another variable with same name
       else {
           attr = symbol->attr;
           attr->common.is_defined = 1;
       } // object defined
   }
   else {
           node = (tree_t*)create_node(node_name, type, node_size, location);
           attr = (object_attr_t*)gdml_zmalloc(attr_size);
           node->common.attr = attr;
           attr->common.obj_type = type;
           attr->common.node = node;
   }// object not defined, create new one

   return attr;
}
void parse_undef(symtab_t table, undef_var_t* undef_head) {
	assert(table);
	assert(undef_head);

	expression_t* expr = NULL;
	tree_t* node = NULL;
	undef_var_t* undef = undef_head;

	while (undef != NULL) {
		node = (tree_t*)(undef->node);
		expr = parse_expression(&node, table);
		if (expr->is_undeclare) {
//			fprintf(stderr, "%s no declare(firs use)\n", expr->undecl_name);
			error("%s isn't declared (first use)", expr->undecl_name);
			/* TODO: handle the error */
//			exit(-1);
		}
		undef = undef->next;
	}


	return;
}

void parse_undef_node(symtab_t table) {
	assert(table);

	if (table->undef_list) {
		parse_undef(table, table->undef_list);
	}

	if ((table->sibling == NULL) && (table->child == NULL)) {
		return;
	}

	if (table->sibling) {

		parse_undef_node(table->sibling);
	}
	if (table->child) {
		parse_undef_node(table->child);
	}

	return;
}

/**
 * @brief dml_keyword_node : make a node for dml keyword
 *
 * @param name: dml keyword
 *
 * @param location: dml location
 *
 * @return : return a pointer to the node
 */
tree_t* dml_keyword_node(const char* name, YYLTYPE* location) {
    tree_t* node = create_node("dml_keyword", DML_KEYWORD_TYPE, sizeof(struct tree_ident), location);
    node->ident.str = strdup(name);
    node->ident.len = strlen(name);
	node->common.print_node = print_ident;

    return node;
}

/**
 * @brief c_keyword_node : make node for c language keyword
 *
 * @param name : c language keyword
 *
 * @param location : c language location
 *
 * @return : return a pointer to node
 */
tree_t* c_keyword_node (const char* name, YYLTYPE* location) {
    tree_t* node = create_node("c_keyword", C_KEYWORD_TYPE, sizeof(struct tree_ident), location);
    node->ident.str = strdup(name);
    node->ident.len = strlen(name);
	node->common.print_node = print_ident;

    return node;
}

static tree_t* node_in_list(tree_t* head, tree_t* new_node) {
   assert(head != NULL);
   assert(new_node != NULL);

   tree_t* tmp = head;
   while (tmp != NULL) {
       if (tmp == new_node) {
           return tmp;
       }
       tmp = tmp->common.sibling;
   }

   return NULL;
}
/**
 * @brief concentrate the two node list
 *
 * @param root : the head node
 * @param new_node : the node will be insert
 *
 * @return : return a pointer to head node;
 */
tree_t* create_node_list (tree_t* root, tree_t* new_node)
{
	if (root != NULL && new_node != NULL) {
		DEBUG_CREATE_NODE_LIST ("In %s, root->name=%s, new_node->name=%s\n",
								__FUNCTION__, root->common.name, new_node->common.name);
	}
	assert (new_node != NULL);
	assert (root != NULL);

	if (node_in_list(root, new_node) != NULL) {
		return root;
	}
	tree_t* tail = find_tail (root);
	assert (tail->common.sibling == NULL);
	tail->common.sibling = new_node;
	return root;
}

/**
 * @brief get_node_num : get the sum number about sibling node
 *
 * @param root : the first node
 *
 * @return : return the num about silbing node
 */
int get_list_num (tree_t* root) {
	if (root == NULL) {
		return 0;
	}

	int num = 0;
	tree_t* node = root;
	while (node != NULL) {
		num++;
		//printf("name: %s, type: %d\n", node->common.name, node->common.type);
		node = node->common.sibling;
	}
	return num;
}

tree_t* create_template_list(tree_t* head, tree_t* templates) {
    if ((head == NULL) && (templates != NULL)) {
        return templates;
    }
    else if ((head != NULL) && (templates != NULL)) {
        while (templates != NULL) {
            create_node_list(head, templates);
            templates = templates->common.sibling;
        }
    }

    return head;
}

tree_t* get_obj_default_param(tree_t* head, tree_t* new, const char* name) {
    /* In simics, although a register can defined many times
     * but the size can defined only once */
	if (head == NULL && new != NULL) {
		return new;
	}
	else if (!strcmp(name, "array")) {
		/* TODO: should check the value of array is the same ? */
		return head;
	}
	else if ((head != NULL) && (new != NULL)) {
		fprintf(stderr, "error: duplicate assignment to parameter '%s'\n", name);
		/* TODO: handle the error */
		exit(-1);
	}

    return head;
}

obj_spec_t* get_obj_spec(obj_spec_t* head, tree_t* node) {
    /* In simics, the same object can be defined several times,
     * but their parameters can be defined only one time*/
    obj_spec_t* tmp = head;
    obj_spec_t* pre_spec = head;

    if ((tmp == NULL) && (node != NULL)) {
        tmp = (obj_spec_t*)gdml_zmalloc(sizeof(obj_spec_t));
        tmp->node = node;
    }
    else if ((tmp != NULL) && (node != NULL)) {
        /* scan all the defined about object */
        while(tmp) {
            /* the describe is corresponing the desc parameter,
             * it can only describe once*/
            if((tmp->node->spec.desc) && (node->spec.desc)) {
                fprintf(stderr, "error: duplicate assignment to parameter 'desc'\n");
                /* TODO: handle the error */
                exit(-1);
            }
            pre_spec = tmp;
            tmp = tmp->next;
        }
        tmp = (obj_spec_t*)gdml_zmalloc(sizeof(obj_spec_t));
        tmp->node = node;
        pre_spec->next = tmp;
        tmp = head;
    }

    return tmp;
}

char* get_obj_desc(obj_spec_t* spec) {
	if (spec == NULL) {
		return NULL;
	}

    obj_spec_t* tmp = spec;
    tree_t* node = NULL;
    while (tmp) {
        node = tmp->node;
        if (node->spec.desc) {
            tree_t* desc = node->spec.desc;
            DEBUG_AST("In %s, line = %d, obj_desc: %s\n",
                    __FUNCTION__, __LINE__, desc->string.pointer);
            return (char*)(desc->string.pointer);
        }
        tmp = tmp->next;
    }

	return NULL;
}

int get_int_value(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		return node->int_cst.value;
	}
	else {
		expr_t* expr = check_expr(node, table);
		if (expr->is_const == 0) {
			PERROR("%s(%s) is non-constant value", node->common.location,
				node->common.name, TYPENAME(node->common.type));
		}
		else if (expr->type->common.categ == INT_T) {
			return expr->val->int_v.value;
		}
		else {
			PERROR("%s(%s) isn't integer type", node->common.location,
				node->common.name, TYPENAME(node->common.type));
		}
	}
}

arraydef_attr_t* get_range_arraydef(tree_t* node, symtab_t table, arraydef_attr_t* array) {
	if (node == NULL) {
		return NULL;
	}
	if (node->array.ident) {
		tree_t* ident = node->array.ident;
		if (ident->common.type == IDENT_TYPE) {
			array->ident = ident->ident.str;
		}
		else {
			PERROR("the array identifier%s isn't IDENT_TYPE",
					ident->common.location, ident->common.name);
		}
	}
	else {
		PERROR("can't find array identifier %s ", node->common.location, node->common.name);
	}

	if (node->array.expr) {
		array->low = get_int_value(node->array.expr, table);
	}
	else {
		PERROR("the array \"%s\" need start array number",
				node->common.location, node->array.ident->common.name);
	}
	if (node->array.expr_end) {
		array->high = get_int_value(node->array.expr_end, table);
	}
	else {
		PERROR("the array \"%s\"need end array number",
				node->common.location, node->array.ident->common.name);
	}

	return array;
}

arraydef_attr_t* get_arraydef(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	arraydef_attr_t* arraydef = (arraydef_attr_t*)gdml_zmalloc(sizeof(arraydef_attr_t));
	if ((node->array.is_fix) == 1) {
		arraydef->fix_array = 1;
		arraydef->high = get_int_value(node->array.expr, table);
	}
	else {
		arraydef = get_range_arraydef(node, table, arraydef);
	}

	return arraydef;
};

symtab_t get_obj_block_table(tree_t* spec) {
	if (spec == NULL) {
		return NULL;
	}
	if (spec->spec.block) {
		tree_t* block = spec->spec.block;
		return (block->block.table);
	}
	else {
		return NULL;
	}
}

char* get_const_string(tree_t* node) {
	if (node == NULL) {
		return NULL;
	}

	DEBUG_AST("In %s, line = %d, str: %s\n",
			__func__, __LINE__, node->string.pointer);

	return (char*)(node->string.pointer);
}

int get_param_num(tree_t* node) {
	if (node == NULL) {
		return 0;
	}
	int num = 0;
	tree_t* tmp = node;
	while (tmp != NULL) {
		num++;
		tmp = tmp->common.sibling;
	}

	DEBUG_AST("In %s, line = %d, param num: %d\n",
			__func__, __LINE__, num);
	return num;
}

params_t** get_param_list(tree_t* node, int param_num, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	params_t** list = gdml_zmalloc(param_num * (sizeof(params_t*)));
	tree_t* tmp = node;
	int i = 0;
	while (tmp != NULL) {
		/* FIXME: parsing the param */
		list[i] = get_param_decl(tmp, table);
		i++;
		tmp = tmp->common.sibling;
	}
	return list;
}

method_params_t* get_method_params(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	method_params_t* params = (method_params_t*)gdml_zmalloc(sizeof(method_params_t));
	if (node->params.in_params) {
		params->in_argc = get_param_num(node->params.in_params);
		params->in_list = get_param_list(node->params.in_params, (params->in_argc), table);
	}

	if (node->params.ret_params) {
		params->ret_argc = get_param_num(node->params.ret_params);
		params->ret_list = get_param_list(node->params.ret_params, (params->ret_argc), table);
	}

	return params;
}

#if 0
paramspec_t* get_paramspec(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return NULL;
	}
	paramspec_t* spec = (paramspec_t*)gdml_zmalloc(sizeof(paramspec_t));
	spec->is_default = node->paramspec.is_default;
	spec->is_auto = node->paramspec.is_auto;
	if (node->paramspec.string) {
		spec->str = get_const_string(node->paramspec.string);
		spec->type = CONST_STRING_TYPE;
	}
	if (spec->is_auto) {
		spec->type = NO_TYPE;
	}
	if (node->paramspec.expr) {
		spec->expr = parse_expression(&(node->paramspec.expr), table);
		spec->type = spec->expr->final_type;
		DEBUG_AST("paramspec is expression expr type: %d\n", spec->type);
	}

	return spec;
}
#endif

paramspec_t* get_param_spec(tree_t* node, symtab_t table) {
	assert(table != NULL);
	if (node == NULL) {
		return NULL;
	}
	paramspec_t* spec = (paramspec_t*)gdml_zmalloc(sizeof(paramspec_t));
	param_value_t* value = (param_value_t*)gdml_zmalloc(sizeof(param_value_t));
	spec->value = value;

	if (node->paramspec.is_auto) {
		value->type = PARAM_TYPE_NONE;
		value->flag = PARAM_FLAG_AUTO;
	}

	if (node->paramspec.string) {
		value->is_const = 1;
		value->type = PARAM_TYPE_STRING;
		value->u.string = strdup(node->paramspec.string->string.pointer);
	}

	if (node->paramspec.is_default) {
		value->flag = PARAM_FLAG_DEFAULT;
	}

	if (node->paramspec.expr) {
		expr_t* expr = check_expr(node->paramspec.expr, table);
		spec->expr_node = node->paramspec.expr;
		if (expr->is_undefined) {
			value->type = PARAM_TYPE_UNDEF;
		}
		else if (expr->type->common.categ == INT_T) {
			value->is_const = expr->is_const ? 1 : 0;
			value->type = PARAM_TYPE_INT;
			if (expr->val) {
				value->u.integer = expr->val->int_v.value;
			}
		}
		else if (expr->type->common.categ == DOUBLE_T) {
			value->is_const = expr->is_const ? 1 : 0;
			value->type = PARAM_TYPE_FLOAT;
			if (expr->val)
				value->u.floating = expr->val->d;
		}
		else {
			if (table->no_check) {
				value->type = PARAM_TYPE_NONE;
				value->flag = PARAM_FLAG_NONE;
			}
			else {
				value->type = PARAM_TYPE_NONE;
				value->flag = PARAM_FLAG_NONE;
			}
		}
	}

	return spec;
}

int get_size(tree_t* node, symtab_t table) {
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		int size = node->int_cst.value;
		if ((size > 0) && (size <= 8)) {
			return size;
		}
		else {
			PWARN("the size(%d) out", node->common.location, size);
		}
	}
	else {
		expr_t* expr = check_expr(node, table);
		if (expr->is_const == 0) {
			PERROR("%s is non-constan value", expr->node->common.location, expr->node->common.name);
		}
		if (expr->type->common.categ == INT_T) {
			if (expr->val->int_v.out_64bit) {
				PERROR("the size of expression \"%s\" is out",
						node->common.location, expr->node->common.name);
			}
			return expr->val->int_v.value;
		}
		else {
			PERROR("the offset final size in other type", node->common.location);
		}
	}

	return 0;
}

int get_offset(tree_t* node, symtab_t table) {
	/* we do not need to check the offset as it will be
	checked when the code generated*/
	return  0;
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		int offset = node->int_cst.value;
		return offset;
	}
	else if (node->common.type == UNDEFINED_TYPE) {
		return -1;
	}
	else {
		expr_t* expr = check_expr(node, table);

		if (expr->is_const == 0) {
			PERROR("%s is non-constan value",
					node->common.location, expr->node->common.name);
		}
		if (expr->type->common.categ == INT_T) {
			if (expr->val->int_v.out_64bit) {
				PERROR("the size of offset \"%s\" is out",
						node->common.location, expr->node->common.name);
			}
			return expr->val->int_v.value;
		}
		else {
			PERROR("the offset final size in other type", node->common.location);
		}
	}

	return 0;
}

#include <unistd.h>
void parse_undef_list(symtab_t table) {
	assert(table != NULL);
	undef_var_t* var = table->undef_list;
	tree_t* tmp = NULL;
	table->pass_num = 1;
	while (var) {
		tmp = var->node;
		if (tmp->common.parse)
			tmp->common.parse(tmp, table);
		var = var->next;
	}

	if ((table->sibling == NULL) && (table->child == NULL))
		return;
	if (table->sibling) {
		parse_undef_list(table->sibling);
	}
	if (table->child) {
		parse_undef_list(table->child);
	}

	return;
}

void parse_register_attr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	attr->reg.size = get_size(node->reg.sizespec, table);
	attr->reg.offset = get_offset(node->reg.offset, table);
	if (node->reg.array) {
		attr->reg.is_array = 1;
		attr->reg.arraydef = get_arraydef(node->reg.array, table);
	}
	attr->common.desc = get_obj_desc(node->reg.spec);

	return;
}

static void parse_bitrange(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	tree_t* tmp = node;
	bitrange_attr_t* attr = tmp->common.attr;
	if (tmp->array.expr) {
		attr->expr = check_expr(tmp->array.expr, table);
	}
	if (tmp->array.expr_end) {
		attr->expr_end = check_expr(tmp->array.expr_end, table);
	}

	return;
}

void parse_field_attr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	tree_t* node_range = node->field.bitrange;
	if (node_range) {
		parse_bitrange(node_range, table);
		attr->field.bitrange = node_range->common.attr;
	}

	return;
}

void parse_connect_attr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	attr->connect.arraydef = get_arraydef(node->connect.arraydef, table);

	return;
}

void parse_attribute_attr(tree_t* node, symtab_t table) {
	assert(node != NULL);
	assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	attr->attribute.arraydef = get_arraydef(node->attribute.arraydef, table);

	return;
}

void parse_group_attr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	attr->group.arraydef = get_arraydef(node->group.array, table);

	return;
}

void parse_port_attr(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	object_attr_t* attr = node->common.attr;
	attr->port.arraydef = get_arraydef(node->port.array, table);

	return;
}

static void parse_object(tree_t* node, symtab_t table) {
	assert(table != NULL);
	/* the symbols, parameters, constant, expressions that
	 * in the current table are the sibling about the object node*/
	tree_t* tmp = node;
	while (tmp) {
		if ((tmp->common.parse) && (tmp->common.parsed == 0)) {
			tmp->common.parsed = 1;
			tmp->common.parse(tmp, table);
		}
		tmp = tmp->common.sibling;
	}

	return;
}

void parse_device(tree_t* node, symtab_t table) {
	/* In device parsing, we only parse the symbols and
	 * calculate the expressions that in device table,
	 * the other table of object, they will be parsed in
	 * other objects, and they will be parsed two times
	 * as some variables can be used befor defined*/
	/* the first time to parse the symbols and expressions */
	parse_object(node, table);

	/* the second time to parse symbols and expressions */
	parse_undef_list(table);

	return;
}

void parse_obj_spec(obj_spec_t* spec, symtab_t table) {
	assert(table != NULL);
	if (spec == NULL) {
		return;
	}
	tree_t* spec_node = NULL;
	tree_t* block = NULL;
	tree_t* statement = NULL;
	while (spec) {
		spec_node = spec->node;
		block = spec_node->spec.block;
		if (block != NULL) {
			statement = block->block.statement;
			parse_object(statement, table);
		}
		spec = spec->next;
	}

	/* the second time to parse symbols */
	parse_undef_list(table);

	return;
}

void parse_bank(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* parsing the symbols that in bank table */
	obj_spec_t* spec = node->bank.spec;
	parse_obj_spec(spec, table);

	return;
}

static cdecl_t* new_int() {
	cdecl_t* type = (cdecl_t*)gdml_zmalloc(sizeof(cdecl_t));
	type->common.categ = INT_T;
	type->common.size = sizeof(int) * 8;

	return type;
}

static void  insert_i_into_obj(symtab_t table) {
	assert(table != NULL);
	cdecl_t* type = new_int();
	symbol_insert(table, "i", PARAMETER_TYPE, type);

	return ;
}

static void insert_index_into_obj(symtab_t table) {
	assert(table != NULL);
	cdecl_t* type = new_int();
	symbol_insert(table, "index", PARAMETER_TYPE, type);

	return;
}

static void insert_array_index_into_obj(arraydef_attr_t* array, symtab_t table) {
	assert(array != NULL); assert(table != NULL);
	if ((array->fix_array == 0) && (table->table_num != 0)) {
		cdecl_t* type = new_int(); 
		symbol_insert(table, array->ident, PARAMETER_TYPE, type);
		insert_index_into_obj(table);	
	} else if ((array->fix_array == 1) && (table->table_num != 0)){
		insert_i_into_obj(table);
		insert_index_into_obj(table);
	}

	return;
}

void parse_register(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	object_attr_t* attr = node->common.attr;
	if (attr->reg.is_array) {
		insert_array_index_into_obj(attr->reg.arraydef, table);
	}

	obj_spec_t* spec = node->reg.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_field(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	obj_spec_t* spec = node->field.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_connect(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	object_attr_t* attr = node->common.attr;
	if (attr->connect.is_array) {
		insert_array_index_into_obj(attr->connect.arraydef, table);
	}

	obj_spec_t* spec = node->connect.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_interface(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	obj_spec_t* spec = node->interface.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_attribute(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	object_attr_t* attr = node->common.attr;
	if (attr->attribute.is_array) {
		insert_array_index_into_obj(attr->attribute.arraydef, table);
	}

	obj_spec_t* spec = node->attribute.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_event(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	obj_spec_t* spec = node->event.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_group(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	object_attr_t* attr = node->common.attr;
	if (attr->group.is_array) {
		insert_array_index_into_obj(attr->group.arraydef, table);
	}

	obj_spec_t* spec = node->group.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_port(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	obj_spec_t* spec = node->port.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_implement(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	obj_spec_t* spec = node->implement.spec;
	parse_obj_spec(spec, table);

	return;
}

void parse_bitorder(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	static int bitorder_num = 0;

	/* user can defined birorder only once, but if usr not defined it
	 * it will be le. actually, in simics, it define a bitorder with le value
	 * and usr can define other value.
	 * */
	if (bitorder_num > 2) {
		error("name collision on 'bitorder'\n");
	}
	bitorder_attr_t* attr = NULL;
	if (symbol_defined(table, "bitorder")) {
		symbol_t symbol = symbol_find(table, "bitorder", BITORDER_TYPE);
		attr = symbol->attr;
	} else {
		attr = (bitorder_attr_t*)gdml_zmalloc(sizeof(bitorder_attr_t));
		attr->name = strdup("bitorder");
	}
	attr->endian = node->bitorder.endian;
	bitorder_num++;

	if ((strcmp(attr->endian, "le") == 0) || (strcmp(attr->endian, "be") == 0)) {
		attr->endian = node->ident.str;
		attr->common.node = node;
		node->common.attr = attr;
		symbol_insert(table, "bitorder", BITORDER_TYPE, attr);
	}
	else {
		free((void*)(attr->name)); free((void*)attr);
		error("illegal bitorder: '%s'", node->ident.str);
	}

	return;
}

void parse_parameter(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	if (symbol_defined(table, node->ident.str))
	   error( "duplicate assignment to parameter '%s'\n", node->ident.str);

	parameter_attr_t* attr = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
	attr->is_original = 1;
	attr->name = node->ident.str;
	attr->common.node = node;
	attr->param_spec = get_param_spec(node->param.paramspec, table);
	node->common.attr = attr;
	symbol_insert(table, node->ident.str, PARAMETER_TYPE, attr);

	return;
}

void parse_obj_if_else(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/*if one node is object if node, we should go
	 * with the following steps to parsing it*/
	/* step 1: check the condition expression */
	tree_t* cond_node = node->if_else.cond;
	check_expr(cond_node, table);

	/* step 2: goto the if block to parse expressions and symbols */
	tree_t* if_node = node->if_else.if_block;
	symtab_t if_table = node->if_else.if_table;
	if_table->pass_num = if_table->parent->pass_num;
	if (if_node->common.parse)
		if_node->common.parse(if_node, if_table);

	/* step 3: if the shape about if grammar is :
	 *  if (cond) {
	 *		...
	 *  }
	 *  else if (cond) {
	 *		...
	 *  }
	 *  else {
	 *		...
	 *  }
	 *  we should goto parsing the else if block
	 *  actually the else if block node is the if node*/
	if (node->if_else.else_if) {
		tree_t*  else_if_node = node->if_else.else_if;
		if (else_if_node->common.parse)
			else_if_node->common.parse(else_if_node, table);
	}

	/* step 4: goto else blok to parse */
	if (node->if_else.else_block) {
		tree_t* else_node = node->if_else.else_block;
		symtab_t else_table = node->if_else.else_table;
		else_table->pass_num = else_table->parent->pass_num;
		if(else_node->common.parse)
			else_node->common.parse(else_node, else_table);
	}

	return;
}

void parse_method(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	method_attr_t* attr = NULL;
	if (symbol_defined(table, node->method.name)) {
		symbol_t symbol = symbol_find(table, node->method.name, METHOD_TYPE);
		attr = symbol->attr;
		//symtab_t method_table = attr->table;
		attr->table = node->method.block->block.table;
		//attr->table->sibling = method_table;
		//error("duplicate definition of method '%s'\n", node->method.name);
		return;
	}

	attr = (method_attr_t*)gdml_zmalloc(sizeof(method_attr_t));
	attr->name = node->ident.str;
	attr->is_extern = node->method.is_extern;
	attr->is_default = node->method.is_default;
	attr->method_params = get_method_params(node->method.params, table);
	attr->table = node->method.block->block.table;
	attr->common.node = node;
	node->common.attr = attr;
	symbol_insert(table, node->ident.str, METHOD_TYPE, attr);
	/* insert the parameters of method into method table */
	params_insert_table(attr->table, attr->method_params);

	return;
}

void parse_method_block(tree_t* node) {
	assert(node != NULL);
	method_attr_t* attr = node->common.attr;
	symtab_t table = attr->table;
	tree_t* block = node->method.block;
	if (attr->is_parsed) {
		return;
	}
	/* as one method can be called many times, but we should parse their
	element only once, so we should have on status bit to mark it*/
	attr->is_parsed = 1;
	DEBUG_AST("parse method '%s' block, table num: %d----------------------------------------\n", attr->name, table->table_num);

	if (block->block.statement == NULL)
		return;
	/*goto the method block to parse elements
	 * and calculate expressions */
	tree_t* tmp = block->block.statement;
	while (tmp) {
		if (tmp->common.parse) {
			tmp->common.parse(tmp, table);
		}
		tmp = tmp->common.sibling;
	}

	/* if the table have undefined symbol, will go to the second
	 * time to parse them as symbol can be used before defined */
	parse_undef_list(table);

	return;
}

void parse_loggroup(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	if (symbol_defined(table, node->loggroup.name))
		error("name collision on '%s'\n", node->loggroup.name);

	symbol_insert(table, node->ident.str, LOGGROUP_TYPE, NULL);

	return;
}

void parse_constant(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	if (symbol_defined(table, node->assign.decl->ident.str))
		error("duplicate assignment to parameter '%s'\n", node->assign.decl->ident.str);

	constant_attr_t* attr = (constant_attr_t*)gdml_zmalloc(sizeof(constant_attr_t));
	attr->name = node->assign.decl->ident.str;
	attr->common.node = node;
	node->common.attr = attr;

	/*the constant symbol must be defined before used*/
	attr->value = check_expr(node->assign.expr, table);
	if (attr->value->is_const) {
		symbol_insert(table, node->assign.decl->ident.str, CONSTANT_TYPE, attr);
	} else {
		fprintf(stderr, "none-constant exprsion '%s'\n", attr->name);
		exit(-1);
	}

	return;
}

void parse_extern_decl(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	parse_extern_cdecl_or_ident(node, table);

	return;
}

void parse_typedef(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	parse_typedef_cdecl(node, table);

	return;
}

void parse_struct_top(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	if (symbol_defined(table, node->ident.str))
		error("name collision on '%s'\n", node->ident.str);

	struct_attr_t* attr = (struct_attr_t*)gdml_zmalloc(sizeof(struct_attr_t));
	attr->name = strdup(node->ident.str);
	attr->table = node->struct_tree.table;
	attr->common.node = node;
	node->common.attr = attr;
	symbol_insert(table, node->ident.str, STRUCT_TYPE, attr);

	/* parse the struct elements, and insert them into table*/
	parse_top_struct_cdecl(node, table, attr);

	return;
}

void parse_data(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	parse_data_cdecl(node, table);

	return;
}

void parse_local(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	parse_local_decl(node, table);

	return;
}

void parse_expr(tree_t* node, symtab_t table) {
	assert(table != NULL); assert(table != NULL);

	tree_t* tmp = node;
	check_expr(tmp, table);

	return;
}

static void parse_compound_statement(tree_t* node) {
	assert(node != NULL);

	symtab_t table = node->block.table;
	tree_t* statement = node->block.statement;
	while (statement) {
		if (statement->common.parse)
			statement->common.parse(statement, table);
		statement = statement->common.sibling;
	}

	return;
}

void parse_if_else(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* if one node is if node, should go with
	 * the following steps to parsing it, as the
	 * if grammar can be the two style with brace
	 * and no brace that diff with object if */

	/* step 1: check the condition expression */
	tree_t* cond = node->if_else.cond;
	check_expr(cond, table);

	/* step 2: goto the if block to parse elements
	 * as if block has two style:
	 * (1). if (cond)
	 *			statement
	 * (2). if (cond) {
	 *			statements
	 *		}*/
	tree_t* if_node = node->if_else.if_block;
	symtab_t if_table = NULL;
	if (if_node->common.type != BLOCK_TYPE) {
		if (if_node->common.parse)
			if_node->common.parse(if_node, node->if_else.if_table);
	} else {
		parse_compound_statement(if_node);
	}

	/* step 3: if the style if:
	 * if (cond)		|	if (cond) {
	 *		statement	|		statement
	 *	else if (cond)	|	}
	 *		statement	|	else if (cond) {
	 *	...				|		statement
	 *	else			|	}
	 *		statement	|	else if (cond)
	 *					|		statement
	 *					|	else {
	 *					|		statement
	 *					|	}
	 * like this, we should goto else if block
	 * to parse elements and calculate expressions
	 * but actually, it is a if node*/
	if (node->if_else.else_if) {
		tree_t* else_if_node = node->if_else.else_if;
		else_if_node->common.parse(else_if_node, table);
	}

	/* step 4: goto the else block to parse
	 * elements and calculate expressions*/
	tree_t* else_block = node->if_else.else_block;
	/* not have the else block */
	if (else_block== NULL)
		return;

	/* else block have two style:
	 * (1). else
	 *			statement
	 * (2). else {
	 *			statements
	 *		}*/
	if (else_block->common.type != BLOCK_TYPE) {
		if (else_block->common.parse)
			else_block->common.parse(else_block, node->if_else.else_table);
	} else {
		parse_compound_statement(else_block);
	}

	return;
}

void parse_do_while(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	/* as the do while or while grammar can be:
	 * (1). do {				| (1).	do
	 *			statements		|			statement
	 *		} while(cond)		|		while(cond)
	 * (2). while(cond) {		| (2). while(cond)
	 *			statements					statement
	 *		}
	 **/
	/* check the condition expression */
	tree_t* cond = node->do_while.cond;
	check_expr(cond, table);

	/* goto the block table to parsing elements
	 * and calculate expressions*/
	tree_t* block_node = node->do_while.block;
	if (block_node->common.type != BLOCK_TYPE) {
		if (block_node->common.parse)
			block_node->common.parse(block_node, node->do_while.table);
	} else {
		parse_compound_statement(block_node);
	}

	return;
}

void parse_for(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	check_comma_expr(node->for_tree.init, table);
	check_expr(node->for_tree.cond, table);
	check_comma_expr(node->for_tree.update, table);

	tree_t* block = node->for_tree.block;
	if (block->common.type != BLOCK_TYPE) {
		if (block->common.parse)
			block->common.parse(block, node->for_tree.table);
	} else {
		parse_compound_statement(block);
	}

	return;
}

void parse_switch(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	tree_t* cond = node->switch_tree.cond;
	check_expr(cond, table);

	tree_t* block = node->switch_tree.block;
	if (block->common.type != BLOCK_TYPE) {
		if (block->common.parse)
			block->common.parse(block, node->switch_tree.table);
	} else {
		parse_compound_statement(block);
	}

	return;
}

void parse_delete(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* the function of delete is to deallocate the
	 * memory pointed by the result of evaluating expr */
	tree_t* expr_node = node->delete_tree.expr;
	expr_t* expr = check_expr(expr_node, table);
	if(expr->type->common.categ != POINTER_T)
		error("delete expression not pointer\n");

	return;
}

void parse_try_catch(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* goto try block to parse elements and
	 * calculate expressions */
	tree_t* try_block = node->try_catch.try_block;
	if (try_block->common.type != BLOCK_TYPE) {
		if (try_block->common.parse)
			try_block->common.parse(try_block, node->try_catch.try_table);
	} else {
		parse_compound_statement(try_block);
	}

	/* goto catch block to pase elements
	 * and calculate expressions */
	tree_t* catch_block = node->try_catch.catch_block;
	if (catch_block->common.type != BLOCK_TYPE) {
		if (catch_block->common.parse)
			catch_block->common.parse(catch_block, node->try_catch.catch_table);
	} else {
		parse_compound_statement(catch_block);
	}

	return;
}

static void parse_call_inline_method(symtab_t table, tree_t* call_expr, tree_t* ret_expr, int inline_method) {
	assert(call_expr != NULL);
	object_t *obj = NULL; tree_t* block = NULL;
	tree_t *node = NULL; method_attr_t *method_attr = NULL;
	symbol_t method_sym = get_call_expr_info(call_expr, table);
	if (method_sym) {
		method_attr = method_sym->attr;
		node = method_attr->common.node;
		block = node->method.block;
		change_current_table(table);
		check_method_param(method_sym, call_expr, ret_expr, inline_method);
		restore_current_table();
		if(method_sym && (method_sym->type == METHOD_TYPE)) {
			obj = (object_t *)method_sym->owner;
			if(!obj) {
				error("method object cannot empty\n");
			} else {
				if (!block_empty(block))
					add_object_method(obj, method_sym->name);
			}
		} else {
			error("method not right %p, %d\n", method_sym, method_sym->type);
		}
	} else {
		error("method not defined\n");
	}

	return;
}

void parse_call(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	parse_call_inline_method(table, node->call_inline.expr, node->call_inline.ret_args, 0);

	return;
}

void parse_inline(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	parse_call_inline_method(table, node->call_inline.expr, node->call_inline.ret_args, 1);

	return;
}

void parse_after_call(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* Grammar:
	 *		after '(' expression ')' call expression */

	/* parse after expression */
	tree_t* cond = node->after_call.cond;
	check_expr(cond, table);

	/* parse call expression */
	tree_t* call_expr = node->after_call.call_expr;
	parse_call_inline_method(table, call_expr, NULL, 0);
	/* the call method expression will be implemented at
	 * the function of translate_call*/

	return;
}

void parse_assert(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	/* Grammar:
	 *		assert expression */
	/* In assert, we should only check the symbol is defined or not
	 * and check the type is ok*/
	tree_t* expr_node = node->assert_tree.expr;
	check_expr(expr_node, table);

	return;
}

static void check_log_type(const char* str) {
	const char* log_type[] =
	{"\"info\"", "\"error\"", "\"undefined\"", "\"spec_violation\"", "\"unimplemented\"", NULL};

	int i = 0;
	while (log_type[i]) {
		if (strcmp(str, log_type[i]) == 0)
			return;
		else
			i++;
	}
	error(" invalid log type: '%s'\n", str);

	return;
}

static void check_log_level(tree_t* node, symtab_t table) {
	if (node == NULL)
		return;

	tree_t* level = node;
	expr_t* expr = check_expr(level, table);
	if (expr->type->common.categ == INT_T) {
		if (expr->is_const) {
			int value = expr->val->int_v.value;
			if ((value > 0) && (value < 5))
				return;
			else
				error("out of log leve(1..4) : %d\n", value);
		}
	}
	else
		error("invalid log level\n");

	return;
}

static void check_log_group(tree_t* node, symtab_t table) {
	assert(table != NULL);
	if (node == NULL)
		return;

	tree_t* group = node;
	expr_t* expr = check_expr(group, table);

	return;
}

extern int scanfstr(const char *str, char ***typelist);
static void parse_log_format(const char* format, tree_t* args) {
	if ((format == NULL) || (args == NULL)) return;
	char** typelist = NULL;
	int arg_num = 0;
	int argc = 0;

	argc = scanfstr(format, &typelist);
	arg_num = get_list_num(args);

	if (argc > arg_num)
		warning("warning: too few arguments for format\n");

	return;
}

void parse_log(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	/* Grammar:
	 *		log string_literal ',' expression ',' expression ':' string_literal ',' log_args
	 *		log string_literal ',' expression ':' string_literal log_args
	 *and it equal to: log log-type, level, groups: format-string, e1, ..., eN; */

	/* the log-type must be one of the string constants:
	 * "info", "error", "undefined", "spec_violation", "unimplemented"*/
	const char* log_type = node->log.log_type;
	check_log_type(log_type);

	/* the value of level must be an interger from 1 to 4,
	 * if omitted, the default level is 1. The meaning
	 * about 1 to 4, please refer to dml reference manual */
	tree_t* level = node->log.level;
	check_log_level(level, table);

	/* the groups must be declared using the loggroup
	 * if omitted, the default value is 0 */
	tree_t* group = node->log.group;
	check_log_group(group, table);

	/* check the log format and the log args, that
	 * like the printf's format, and args*/
	tree_t* log_args = node->log.args;
	parse_log_args(log_args, table);
	parse_log_format(node->log.format, log_args);

	return;
}

void parse_select(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	/* Grammar:
	 * select ident in '(' expression ')' where '(' expression ')' statement else statement */
	tree_t* ident = node->select.ident;
	/* insert the ident into table */
	select_attr_t* attr = (select_attr_t*)gdml_zmalloc(sizeof(select_attr_t));
	attr->ident = ident->ident.str;
	attr->common.node = node;
	symbol_insert(node->select.where_table, ident->ident.str, SELECT_TYPE, attr);

	tree_t* in_expr = node->select.in_expr;
	attr->in_expr = check_expr(in_expr, table);
	attr->type = attr->in_expr->type->common.categ;

	tree_t* cond = node->select.cond;
	/* as we insert the ident into where block, so
	 * check the conditional expression and find
	 * the symbol from where table*/
	check_expr(cond, node->select.where_table);

	/* goto the statement to parse elements
	 * and calculate expressions */
	tree_t* where_block = node->select.where_block;
	if (where_block->common.type != BLOCK_TYPE) {
		if (where_block->common.parse)
			where_block->common.parse(where_block, node->select.where_table);
	} else {
		parse_compound_statement(where_block);
	}

	tree_t* else_block = node->select.else_block;
	if (else_block->common.type != BLOCK_TYPE) {
		if (else_block->common.parse)
			else_block->common.parse(else_block, node->select.else_table);
	} else {
		parse_compound_statement(else_block);
	}

	return;
}

void parse_foreach(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/* Grammar:
	 *		foreach ident in '(' expression ')' statement
	 */
	/* insert ident into table of foreach */
	tree_t* ident = node->foreach.ident;
	foreach_attr_t* attr = (foreach_attr_t*)gdml_zmalloc(sizeof(foreach_attr_t));
	attr->ident = ident->ident.str;
	attr->common.node = node;
	attr->table = node->foreach.table;
	node->common.attr = attr;
	symbol_insert(node->foreach.table, ident->ident.str, FOREACH_TYPE, attr);

	tree_t* in_expr = node->foreach.in_expr;
	attr->expr = check_expr(in_expr, table);
	attr->table = node->foreach.table;

	symbol_t list = get_expression_sym(in_expr);
	param_value_t *val = NULL;
	symtab_t saved = NULL;
	int len, i;
	len = i = 0;
	if(list->type != ARRAY_TYPE){
		if(list->type == PARAMETER_TYPE ) {
			val = list->attr;
			if(val->type == PARAM_TYPE_LIST) {
				len = val->u.list.size;
			} else {
				my_DBG("error type in foreach\n");
			}
		} else {
			my_DBG("error type in foreach\n");
		}
	}
	symbol_t tmp = symbol_find(attr->table, ident->ident.str, FOREACH_TYPE);
	if(val->u.list.vector[0].type == PARAM_TYPE_REF) {
		symbol_set_type(tmp, OBJECT_TYPE);
	}
	tree_t* block = node->foreach.block;
	for(i = 0; i < len; i++) {
		symbol_set_value(tmp, val->u.list.vector[i].u.ref);
		if (block->common.type != BLOCK_TYPE) {
			if (block->common.parse) {
				block->common.parse(block, node->foreach.table);
			}
		} else {
			parse_compound_statement(block);
		}
	}

	/* goto the statement to parse elements
	 * and calculate expressions */

	symbol_set_type(tmp, FOREACH_TYPE);
	symbol_set_value(tmp, attr);

	return;
}

void parse_label(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	/*Grammar:
	 *		ident ':' statement */
	tree_t* ident = node->label.ident;
	if (symbol_defined(table, ident->ident.str))
		error("duplicate definition of label '%s'\n", ident->ident.str);
	label_attr_t* attr = (label_attr_t*)gdml_zmalloc(sizeof(label_attr_t));
	attr->name = ident->ident.str;
	attr->table = node->label.table;
	attr->common.node = node;
	symbol_insert(table, ident->ident.str, LABEL_TYPE, attr);

	/* goto block to parse elements
	 * and calculate expressions */
	tree_t* block = node->label.block;
	if (block->common.type != BLOCK_TYPE) {
		if (block->common.parse)
			block->common.parse(block, node->label.table);
	} else {
		parse_compound_statement(block);
	}

	return;
}

void parse_case(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);
	/* Grammar:
	 *		CASE expression ':' statement */
	tree_t* expr_node = node->case_tree.expr;
	expr_t* expr = check_expr(expr_node, table);
	if (!is_int_type(expr->type))
		error("case label does not reduce to an integer constant\n");

	tree_t* block = node->case_tree.block;
	if (block->common.type != BLOCK_TYPE) {
		if (block->common.parse)
			block->common.parse(block, node->case_tree.table);
	} else {
		parse_compound_statement(block);
	}

	return;
}

void parse_default(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	tree_t* block = node->default_tree.block;
	if (block->common.type != BLOCK_TYPE) {
		if (block->common.parse)
			block->common.parse(block, node->default_tree.table);
	} else {
		parse_compound_statement(block);
	}

	return;
}

void parse_goto(tree_t* node, symtab_t table) {
	assert(node != NULL); assert(table != NULL);

	tree_t* ident = node->goto_tree.label;
	const char* label_name = ident->ident.str;
	symbol_t symbol = symbol_find(table, label_name, LABEL_TYPE);
	if (symbol == NULL)
		error("label ‘%s’ used but not defined\n", label_name);

	return;
}

void print_templates(symtab_t table) {
	assert(table);
	struct template_table* tmp = table->template_table;

	int i = 0;
	while ((tmp) != NULL) {
		DEBUG_TEMPLATES("templates name(%d): %s, table_num: %d\n", i++, tmp->template_name, tmp->table->table_num);
		tmp = tmp->next;
	}

	return;
}

static void insert_undef_template(symtab_t table, const char* name) {
	assert(table != NULL); assert(name != NULL);
	undef_template_t* new_undef = (undef_template_t*)gdml_zmalloc(sizeof(undef_template_t));
	new_undef->name = strdup(name);
	undef_template_t* tmp = table->undef_temp;
	if (table->undef_temp == NULL) {
		table->undef_temp = new_undef;
	} else {
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new_undef;
	}

	return;
}

extern symtab_t root_table;
void add_template_to_table(symtab_t table, const char* template, int second_check) {
	assert(table != NULL);
	assert(template != NULL);

	struct template_table* pre_temp_table = table->template_table;
	struct template_table* temp_table = NULL;
	struct template_table* new_table = NULL;
	char* template_name = NULL;

	temp_table = table->template_table;

	while ((temp_table) != NULL) {
		template_name = temp_table->template_name;
		DEBUG_AST("In %s, line = %d, trave templates: %s\n", __func__, __LINE__, template_name);
		if (strcmp(template, template_name) == 0) {
			/*FIXME: should handle the error */
			warning("re-load the template \"%s\"", template);
			return;
//			exit(-1);
		}
		pre_temp_table = temp_table;
		temp_table = temp_table->next;
	}

	new_table = (struct template_table*)gdml_zmalloc(sizeof(struct template_table));

	symbol_t symbol = symbol_find(root_table, template, TEMPLATE_TYPE);
	if (symbol == NULL) {
		if (second_check == 0) {
			insert_undef_template(table, template);
			free(new_table);
			return;
		} else {
			error("can't find the template \"%s\"", template);
		}
	}
	template_attr_t* attr = symbol->attr;
	new_table->table = attr->table;
	new_table->template_name = strdup(template);

	if (new_table->table != NULL) {
		if (table->template_table == NULL) {
			table->template_table = new_table;
		}
		else {
			DEBUG_AST("pre_temp_table : %s\n", pre_temp_table->template_name);
			//table->template_table  = new_table;
			pre_temp_table->next = new_table;
		}
	}
	else {
		free(new_table->template_name);
		free(new_table);
	}

	return;
}

void check_undef_template(symtab_t table) {
	assert(table != NULL);
	if (table->undef_temp == NULL) {
		return;
	}
	undef_template_t* temp = table->undef_temp;
	while (temp) {
		add_template_to_table(table, temp->name, 1);
		temp = temp->next;
	}

	return;
}

void add_templates_to_table(symtab_t table, char** templates, int num) {
	assert(table);

	if (num == 0) {
		return;
	}

	int i = 0;

	for (i = 0; i < num; i++) {
		DEBUG_AST("\nIn %s, line = %d, num: %d, templates: %s, table_num: %d\n",
				__FUNCTION__, __LINE__, i, templates[i], table->table_num);

		add_template_to_table(table, templates[i], 0);
	}

	return;
}

void get_object_template_table(symtab_t table, tree_t* node) {
	DEBUG_AST("In %s, line = %d, node(%d): %s\n",
			__FUNCTION__, __LINE__, node->common.type, node->common.name);

	assert(table != NULL);
	assert(node != NULL);
	char** templates = NULL;
	int template_num = 0;
	struct object_common* attr = NULL;

	switch(node->common.type) {
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
				attr = (struct object_common*)(node->common.attr);
				templates = (char**)(attr->templates);
				template_num = attr->templates_num;
				break;
		case TEMPLATE_TYPE:
			break;
		default:
//			fprintf(stderr, "unknown object type(%d) : %s",
//					node->common.type, node->common.name);
			/* FIXME handle the error */
			PWARN("unknown object type (%s, %s)", node->common.location,
					node->common.name, TYPENAME(node->common.type));
			break;
	}

	add_templates_to_table(table, templates, template_num);

	return;
}

char** get_templates(char** templates, tree_t* head, int num) {
	if (head == NULL) {
		return templates;
	}
	if (templates != NULL)
		free(templates);

	templates = gdml_zmalloc(num * sizeof(char*));
	tree_t* node = head;
	int i = 0;
	while (node != NULL) {
		if (((node->common.type) == DML_KEYWORD_TYPE) ||
				((node->common.type) == IDENT_TYPE)) {
			templates[i++] = (char*)(node->ident.str);
			DEBUG_AST("identifier:  %s : %s\n", node->ident.str, templates[i - 1]);
		}
		else {
//			fprintf(stderr, "The templates'type is(%d) : %s: %s\n",
//					node->common.type, node->common.name, node->ident.str);
			warning("the templates' type is %s (%s, %s)",
					TYPENAME(node->common.type), node->common.name, node->ident.str);
		}
		node = node->common.sibling;
	}

	return templates;
}

/**
 * @brief print_pos : print the node position int syntax tree
 *
 * @param i: the node position
 */
void print_pos (int i)
{
	while ((i--) > 0) {
		printf ("|   ");
	}
}

/**
 * @brief print_sibling : print the sibling tree node information
 *
 * @param node : the tree node
 * @param pos : tree node position
 */
void print_sibling(tree_t* node, int pos) {
	if (node->common.sibling) {
		tree_t* sibling = node->common.sibling;
		if (sibling->common.print_node)
			sibling->common.print_node(sibling, pos);
	}

	return;
}

/**
 * @brief print_bitorder : print the bitorder tree node
 *
 * @param node : pointer of bitorder tree node
 * @param pos : tree node position
 */
void print_bitorder(tree_t* node, int pos) {
	/* Grammar : BITORDER ident */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->bitorder.endian, pos);

	/* print sibling tree node information  */
	print_sibling(node, pos);

	return;
}

/**
 * @brief print_string : print string tree node
 *
 * @param node : pointer to string
 * @param pos : tree node position
 */
void print_string(tree_t* node, int pos) {
	/*
	 * Grammar :
	 *		'=' STRING_LITERAL ';'
	 *		| STRING_LITERAL
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->string.pointer, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_interger : print integer literal tree node
 *
 * @param node : pointer to tree node
 * @param pos : tree node position in syntax tree
 */
void print_interger(tree_t* node, int pos) {
	/* Grammar : INTEGER_LITERAL */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->int_cst.int_str, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_quote : print quote tree node information
 *
 * @param node : pointer to quote tree node
 * @param pos : tree node position in syntax tree
 */
void print_quote(tree_t* node, int pos) {
	/* Grammar: '$' objident */
	tree_t* objident = node->quote.ident;
	print_pos(pos);
	printf("[%s : $%s : %d]\n",
			node->common.name, objident->ident.str, pos);

	print_sibling(node, pos);
}

/**
 * @brief print_ternary : print ternary tree node information
 *
 * @param node : pointer to ternary tree node
 * @param pos : tree node position in syntax tree
 */
void print_ternary(tree_t* node, int pos) {
	/*ternary grammar : condition '?' expression ':' expression */
	if (node->ternary.cond) {
		tree_t* cond = node->ternary.cond;
		cond->common.print_node(cond, pos);
	}
	else {
		printf("Ternary operation need the condition expression\n");
	}
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "?", pos);

	if (node->ternary.expr_true) {
		tree_t* true_expr = node->ternary.expr_true;
		true_expr->common.print_node(true_expr, pos);
	}
	else {
		printf("Ternary operation need the true expression\n");
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, ":", pos);

	if (node->ternary.expr_false) {
		tree_t* false_expr = node->ternary.expr_false;
		false_expr->common.print_node(false_expr, pos);
	}
	else {
		printf("Ternary operation need the false expression\n");
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_binary : print binary tree node information
 *
 * @param node : pointer to binary tree node
 * @param pos : tree node position in syntax tree
 */
void print_binary(tree_t* node, int pos) {
	/* binary grammar: expression operator expression */
	if (node->binary.left) {
		tree_t* left =  node->binary.left;
		left->common.print_node(left, pos);
	}
	else {
		printf("The binary(%s) need left expression\n",
				node->binary.operat);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->binary.operat, pos);

	if (node->binary.right) {
		tree_t* right = node->binary.right;
		right->common.print_node(right, pos);
	}
	else {
		printf("The binary(%s)need right expression\n",
				node->binary.operat);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_unary : print unary tree node information
 *
 * @param node : pointer to unary tree node
 * @param pos : tree node position in syntax tree
 */
void print_unary(tree_t* node, int pos) {
	/* grammar: operator(~) expression */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->unary.operat, pos);

	if (node->unary.expr) {
		tree_t* expr = node->unary.expr;
		DEBUG_BLACK("Line = %d, name: %s\n", __LINE__, expr->common.name);
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_ctypedecl_simple  : print tree node ctypedecl_simple
 *
 * @param node : pointer to ctypedecl_simple tree node
 * @param pos : tree node position
 */
void print_ctypedecl_simple(tree_t* node, int pos) {
	   /* Grammar : '(' ctypedecl_ptr ')' | NULL */
	if (node->cdecl_brack.decl_list) {
		tree_t* list_head = node->cdecl_brack.decl_list;
		list_head->common.print_node(list_head, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_stars : print stars tree node information
 *
 * @param node : pointer to stars tree node
 * @param pos : tree node position int syntax tree
 */
void print_stars(tree_t* node, int pos) {
	/*
	 * Grammar :
	 *		'*' CONST stars
	 *		| '*' stars
	 *		| NULL
	 */
	if (node->stars.is_const) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "* const", pos);
	}
	else {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "*", pos);
	}

	if (node->stars.stars) {
		tree_t* stars = node->stars.stars;
		stars->common.print_node(stars, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_ctypedecl_ptr : print ctypedecl_ptr information
 *
 * @param node : pointer to ctypedecl_ptr tree node
 * @param pos : tree node position in syntax tree
 */
void print_ctypedecl_ptr(tree_t* node, int pos) {
	/* Grammar : stars ctypedecl_array */
	if (node->ctypedecl_ptr.stars) {
		tree_t* stars = node->ctypedecl_ptr.stars;
		stars->common.print_node(stars, pos);
	}

	if (node->ctypedecl_ptr.array) {
		tree_t* array = node->ctypedecl_ptr.array;
		array->common.print_node(array, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_ctypedecl : print ctypedecl tree node information
 *
 * @param node : pointer to ctypedecl tree node
 * @param pos : tree node position in syntax tree
 */
void print_ctypedecl(tree_t* node, int pos) {
	/* Grammar : const_opt basetype ctypedecl_ptr  */
	if (node->ctypedecl.const_opt) {
		tree_t* const_opt = node->ctypedecl.const_opt;
		const_opt->common.print_node(const_opt, pos);
	}
	if (node->ctypedecl.basetype) {
		tree_t* basetype = node->ctypedecl.basetype;
		basetype->common.print_node(basetype, pos);
	}
	if (node->ctypedecl.ctypedecl_ptr) {
		tree_t* ctypedecl_ptr = node->ctypedecl.ctypedecl_ptr;
		ctypedecl_ptr->common.print_node(ctypedecl_ptr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_after_call : print after_call tree node information
 *
 * @param node : pointer to after_call tree node
 * @param pos : tree node position in syntax tree
 */
void print_after_call(tree_t* node, int pos) {
	/* Grammar : AFTER '(' expression ')' CALL expression  */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "after(", pos);

	if (node->after_call.cond) {
		tree_t* cond = node->after_call.cond;
		cond->common.print_node(cond, pos);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "call", pos);

	if (node->after_call.call_expr) {
		tree_t* expr = node->after_call.call_expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_new : print new tree node information
 *
 * @param node : pointer to new tree node
 * @param pos : tree node position in syntax tree
 */
void print_new(tree_t* node, int pos) {
	/*
	 * Grammar :
	 *		NEW ctypedecl
	 *		| NEW ctypedecl '[' expression ']'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "new", pos);

	if (node->new_tree.type) {
		tree_t* type = node->new_tree.type;
		type->common.print_node(type, pos);
	}

	if (node->new_tree.count) {
		tree_t* count = node->new_tree.count;
		count->common.print_node(count, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_typeoparg : print typeoparg tree node information
 *
 * @param node : pointer to typeoparg tree node
 * @param pos : tree node position in syntax tree
 */
void print_typeoparg(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		ctypedecl
	 *		| '(' ctypedecl ')'
	 */
	print_pos(pos);
	if (node->typeoparg.ctypedecl) {
		printf("[%s : %s : %d]\n",
				node->common.name, "typeoparg", pos);
		tree_t* ctypedecl = node->typeoparg.ctypedecl;
		ctypedecl->common.print_node(ctypedecl, pos);
	}

	if (node->typeoparg.ctypedecl_brack) {
		printf("[%s : %s : %d]\n",
				node->common.name, "typeoparg_brack", pos);
		tree_t* ctypedecl_brack = node->typeoparg.ctypedecl_brack;
		ctypedecl_brack->common.print_node(ctypedecl_brack, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_sizeoftype : print sizeoftype tree node information
 *
 * @param node : pointer to sizeoftype tree node
 * @param pos : tree node position in syntax tree
 */
void print_sizeoftype (tree_t* node, int pos) {
	/* Grammar: SIZEOFTYPE typeoparg */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "sizeoftype", pos);

	if (node->sizeoftype.typeoparg) {
		tree_t* typeoparg = node->sizeoftype.typeoparg;
		typeoparg->common.print_node(typeoparg, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_sizeof : print sizeof tree node information
 *
 * @param node : pointer to sizeof tree node
 * @param pos : tree node position in syntax tree
 */
void print_sizeof(tree_t* node, int pos) {
	/* Grammar: SIZEOF expression */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "sizeof", pos);

	if (node->sizeof_tree.expr) {
		tree_t* expr = node->sizeof_tree.expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_cast : print cast tree node information
 *
 * @param node : pointer to cast tree node
 * @param pos : tree node position in syntax tree
 */
void print_cast(tree_t* node, int pos) {
	/* Grammar: CAST '(' expression ',' ctypedecl ')' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "cast(", pos);
	if (node->cast.expr) {
		tree_t* expr = node->cast.expr;
		expr->common.print_node(expr, pos);
	}
	if (node->cast.ctype) {
		tree_t* ctype = node->cast.ctype;
		DEBUG_BLACK("Line = %d, name: %s\n", __LINE__, ctype->common.name);
		ctype->common.print_node(ctype, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_float_literal : print float_literal tree node information
 *
 * @param node : pointer to float_literal tree node
 * @param pos : tree node position in syntax tree
 */
void print_float_literal(tree_t* node, int pos) {
	/* Grammar: FLOAT_LITERAL */
	print_pos(pos);
	printf("[%s : %s : %d-------------------------------]\n",
			node->common.name, node->float_cst.float_str, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_expr_brack : print expression with brackets tree node information
 *
 * @param node : pointer to expression tree node
 * @param pos : tree node position in syntax tree
 */
void print_expr_brack(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		'(' expression ')'
	 *		| expression '(' expression_list ')'
	 */
	tree_t* expr = NULL;
	if (node->expr_brack.expr) {
		expr = node->expr_brack.expr;
		expr->common.print_node(expr, pos);
	}
	if (node->expr_brack.expr_in_brack) {
		expr = node->expr_brack.expr_in_brack;
		if (expr->common.print_node)
			expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_undefined : print undefined tree node information
 *
 * @param node : pointer to undefined tree node
 * @param pos : tree node position in syntax tree
 */
void print_undefined(tree_t* node, int pos) {
	/* Grammar: UNDEFINED */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
		node->common.name, "undefined", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_component : print component tree node information
 *
 * @param node : pointer to component tree node
 * @param pos : tree node position in syntax tree
 */
void print_component(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		expression '.' objident
	 *		| expression METHOD_RETURN objident
	 */
	if (node->component.expr) {
		tree_t* expr = node->component.expr;
		expr->common.print_node(expr, pos);
	}
	else {
		fprintf(stderr, "The component need expression\n");
	}

	print_pos(pos);
	if ((strcmp(node->common.name, "dot")) == 0) {
		printf("[%s : %s : %d]\n",
			node->common.name, ".", pos);
	}
	else if ((strcmp(node->common.name, "pointer")) == 0) {
		printf("[%s : %s : %d]\n",
			node->common.name, "->", pos);
	}

	if (node->component.ident) {
		tree_t* ident = node->component.ident;
		ident->common.print_node(ident, pos);
	}
	else {
		fprintf(stderr, "The component need identifier\n");
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_paramspec : print paramspec tree node information
 *
 * @param node : pointer to paramspec tree node
 * @param pos : tree node position in syntax tree
 */
void print_paramspec(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		';'
	 *		| '=' expression ';'
	 *		| '=' STRING_LITERAL ';'
	 *		| DEFAULT expression ';'
	 *		| AUTO ';' */
	tree_t* spec = NULL;
	if ((node->paramspec.string) != NULL) {
		spec = node->paramspec.string;
		spec->common.print_node(spec, pos);
	}
	else if (node->paramspec.is_auto) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
			node->common.name, "auto", pos);
	}
	else if (((node->paramspec.is_default) == 0) &&
			(node->paramspec.expr != NULL)) {
		spec = node->paramspec.expr;
		spec->common.print_node(spec, pos);
	}
	else if (node->paramspec.is_default) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
			node->common.name, "default", pos);
		spec = node->paramspec.expr;
		spec->common.print_node(spec, pos);
	}
	else {
		printf("something wrong!\n");
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_select: print select tree node information
 *
 * @param node : pointer to select tree node
 * @param pos : tree node position in syntax tree
 */
void print_select(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		SELECT ident IN '(' expression ')' WHERE '(' expression ')' statement ELSE statement
	 */
	tree_t* ident = node->select.ident;
	print_pos(pos);
	printf("[%s : %s IN : %d]\n",
		node->common.name, ident->ident.str, pos);

	if (node->select.in_expr) {
		tree_t* expr = node->select.in_expr;
		expr->common.print_node(expr, pos);
	}

	print_pos(pos);
	printf("[%s : %s IN : %d]\n",
		node->common.name, "where", pos);

	if (node->select.cond) {
		tree_t* cond = node->select.cond;
		cond->common.print_node(cond, pos);
	}

	if (node->select.where_block) {
		tree_t* block = node->select.where_block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos - 1));
		}
	}

	print_pos(pos);
	printf("[%s : %s IN : %d]\n",
		node->common.name, "else", pos);

	if (node->select.else_block) {
		tree_t* block = node->select.else_block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos - 1));
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_connect : print select tree node information
 *
 * @param node : pointer to connect tree node
 * @param pos : tree node position in syntax tree
 */
void print_connect(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		CONNECT objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->connect.name, pos);

	if (node->connect.arraydef) {
		tree_t* arraydef = node->connect.arraydef;
		arraydef->common.print_node(arraydef, pos);
	}

	if (node->connect.templates) {
		tree_t* templates = node->connect.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->connect.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_interface : print select tree node information
 *
 * @param node : pointer to interface tree node
 * @param pos : tree node position in syntax tree
 */
void print_interface(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		INTERFACE objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->interface.name, pos);

	if (node->interface.templates) {
		tree_t* templates = node->interface.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->interface.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_attribute : print attribute tree node information
 *
 * @param node : pointer to attribute tree node
 * @param pos : tree node position in syntax tree
 */
void print_attribute(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		ATTRIBUTE objident istemplate object_spec
	 *		| ATTRIBUTE objident '[' arraydef ']' istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->attribute.name, pos);

	if (node->attribute.arraydef) {
		tree_t* arraydef = node->attribute.arraydef;
		arraydef->common.print_node(arraydef, pos);
	}
	if (node->attribute.templates) {
		tree_t* templates = node->attribute.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->attribute.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_event : print event tree node information
 *
 * @param node : pointer to event tree node
 * @param pos : tree node position in syntax tree
 */
void print_event(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		EVENT objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->event.name, pos);

	if (node->event.templates) {
		tree_t* templates = node->event.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->event.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_group : print group tree node information
 *
 * @param node : pointer to group tree node
 * @param pos : tree node position in syntax tree
 */
void print_group(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		GROUP objident istemplate object_spec
	 *		| GROUP objident '[' arraydef ']' istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->group.name, pos);

	if (node->group.array) {
		tree_t* array = node->group.array;
		array->common.print_node(array, pos);
	}
	if (node->group.templates) {
		tree_t* templates = node->group.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->group.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_port : print port tree node information
 *
 * @param node : pointer to port tree node
 * @param pos : tree node position in syntax tree
 */
void print_port(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		PORT objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->port.name, pos);

	if (node->port.templates) {
		tree_t* templates = node->port.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->port.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_implement : print implement tree node information
 *
 * @param node : pointer to implement tree node
 * @param pos : tree node position in syntax tree
 */
void print_implement(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		IMPLEMENT objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->implement.name, pos);

	if (node->implement.templates) {
		tree_t* templates = node->implement.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->implement.spec;
    if (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_parameter : print parameter tree node information
 *
 * @param node : pointer to parameter tree node
 * @param pos : tree node position in syntax tree
 */
void print_parameter(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		PARAMETER objident paramspec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->param.name, pos);
	if ((node->param.paramspec) != NULL) {
		tree_t* spec = NULL;
		spec = node->param.paramspec;
		spec->common.print_node(spec, pos);
	}

	print_sibling(node, pos);

	return ;
}

/**
 * @brief print_array : print array tree node information
 *
 * @param node : pointer to array tree node
 * @param pos : tree node position in syntax tree
 */
void print_array(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		'[' expression ']'
	 *		| '[' expression ':' expression ']'
	 *		| cdecl3 '[' expression ']'
	 *		| '[' expression_list ']'
	 */
	if (node->array.decl) {
		tree_t* decl = node->array.decl;
		decl->common.print_node(decl, pos);
	}

	if (node->array.expr) {
		tree_t* expr = node->array.expr;
		expr->common.print_node(expr, pos);
	}

	if (node->array.expr_end) {
		tree_t* expr_end = node->array.expr_end;
		expr_end->common.print_node(expr_end, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_arraydef : print arraydef tree node information
 *
 * @param node : pointer to arraydef tree node
 * @param pos : tree node position in syntax tree
 */
void print_arraydef(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		| expression
	 *		| ident IN expression RANGE_SIGN expression
	 */
	if (node->array.is_fix) {
		tree_t* expr = node->array.expr;
		expr->common.print_node(expr, pos);
	}
	else {
		if (node->array.ident) {
			tree_t* ident = node->array.ident;
			print_pos(pos);
			printf("[%s : %s IN : %d]\n",
					node->common.name, ident->ident.str, pos);
		}
		if (node->array.expr) {
			tree_t* expr = node->array.expr;
			expr->common.print_node(expr, pos);
		}
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "range_sign", pos);
		if (node->array.expr_end) {
			tree_t* expr_end = node->array.expr_end;
			expr_end->common.print_node(expr_end, pos);
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_bitfields_decls : print bitfields_decls tree node information
 *
 * @param node : pointer to bitfields_decls tree node
 * @param pos : tree node position in syntax tree
 */
void print_bitfields_decls(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		bitfields_decls cdecl '@' '[' expression ':' expression ']' ';'
	 */
	if (node->bitfields_dec.decl) {
		tree_t* decl = node->bitfields_dec.decl;
		decl->common.print_node(decl, pos);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "@", pos);

	if (node->bitfields_dec.start) {
		tree_t* start = node->bitfields_dec.start;
		start->common.print_node(start, pos);
	}

	if (node->bitfields_dec.end) {
		tree_t* end = node->bitfields_dec.end;
		end->common.print_node(end, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_bitfields : print bitfields tree node information
 *
 * @param node : pointer to bitfields tree node
 * @param pos : tree node position in syntax tree
 */
void print_bitfields(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		BITFIELDS INTEGER_LITERAL '{' bitfields_decls '}'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->bitfields.name, pos);

	if (node->bitfields.block) {
		tree_t* block = node->bitfields.block;
		block->common.print_node(block, (pos - 1));
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_register : print register tree node information
 *
 * @param node : pointer to register tree node
 * @param pos : tree node position in syntax tree
 */
void print_register(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		REGISTER objident sizespec offsetspec istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->reg.name, pos);
	if (node->reg.array) {
		tree_t* array = node->reg.array;
		array->common.print_node(array, pos);
	}
	if (node->reg.sizespec) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "sizespec", pos);
		tree_t* sizespec = node->reg.sizespec;
		sizespec->common.print_node(sizespec, pos);
	}
	if (node->reg.offset) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "@", pos);
		tree_t* offset = node->reg.offset;
		offset->common.print_node(offset, pos);
	}
	if (node->reg.templates) {
		tree_t* templates = (tree_t*)(node->reg.templates);
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->reg.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_bank : print bank tree node information
 *
 * @param node : pointer to bank tree node
 * @param pos : tree node position in syntax tree
 */
void print_bank(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		BANK maybe_objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->bank.name, pos);
	if (node->bank.templates) {
		tree_t* templates = node->bank.templates;
		templates->common.print_node(templates, pos);
	}

	obj_spec_t* spec = node->bank.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_try_catch : print try_catch tree node information
 *
 * @param node : pointer to try_catch tree node
 * @param pos : tree node position in syntax tree
 */
void print_try_catch(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		TRY statement CATCH statement
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "try", pos);

	if (node->try_catch.try_block) {
		tree_t* try = node->try_catch.try_block;
		if (try->common.type == BLOCK_TYPE) {
			tree_t* statement = try->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos -1));
		}
		else {
			if (try->common.print_node)
				try->common.print_node(try, (pos - 1));
		}
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "catch", pos);

	if (node->try_catch.catch_block) {
		tree_t* catch = node->try_catch.catch_block;
		if (catch->common.type == BLOCK_TYPE) {
			tree_t* statement = catch->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (catch->common.print_node)
				catch->common.print_node(catch, (pos -1));
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_bit_slic : print bit_slic tree node information
 *
 * @param node : pointer to bit_slic tree node
 * @param pos : tree node position in syntax tree
 */
void print_bit_slic(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		'[' expression_list ']'
	 *		| expression '[' expression endianflag ']'
	 */
	if (node->bit_slic.expr) {
		tree_t* expr = node->bit_slic.expr;
		expr->common.print_node(expr, pos);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "[", pos);

	if (node->bit_slic.bit) {
		tree_t* bit = node->bit_slic.bit;
		bit->common.print_node(bit, pos);
	}

	if (node->bit_slic.bit_end) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
			node->common.name, ":", pos);
		tree_t* bit_end = node->bit_slic.bit_end;
		bit_end->common.print_node(bit_end, pos);
	}

	if (node->bit_slic.endian) {
		tree_t* endian = node->bit_slic.endian;
		endian->common.print_node(endian, pos);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
		node->common.name, "]", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_expr_assign : print expr_assign tree node information
 *
 * @param node : pointer to expr_assign tree node
 * @param pos : tree node position in syntax tree
 */
void print_expr_assign(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		expression '=' expression
	 */
	if (node->expr_assign.left) {
		tree_t* left = node->expr_assign.left;
		left->common.print_node(left, pos);
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->expr_assign.assign_symbol, pos);

	if (node->expr_assign.right) {
		tree_t* right = node->expr_assign.right;
		right->common.print_node(right, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_assign : print assign tree node information
 *
 * @param node : pointer to assign tree node
 * @param pos : tree node position in syntax tree
 */
void print_assign(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		CONSTANT ident '=' expression ';'
	 */
	if (node->assign.is_constant) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "constant", pos);
	}
	if (node->assign.is_const) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "const", pos);
	}
	if (node->assign.is_static) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "static", pos);
	}
	if (node->assign.is_local) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "local", pos);
	}

	if (node->assign.decl) {
		tree_t* decl = node->assign.decl;
		decl->common.print_node(decl, pos);
	}
	if (node->assign.expr) {
		tree_t* expr = node->assign.expr;
		expr->common.print_node(expr, pos);
	}
	if (node->assign.expr_right) {
		tree_t* expr_right = node->assign.expr_right;
		expr_right->common.print_node(expr_right, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_ident : print identifier tree node information
 *
 * @param node : pointer to identifier tree node
 * @param pos : tree node position in syntax tree
 */
void print_ident(tree_t* node, int pos) {
	/* Grammar: IDENTIFIER */
	print_pos(pos);
	printf("[%s : %s: %d]\n",
			node->common.name, node->ident.str, pos);

	print_sibling(node, pos);

	return ;
}

/**
 * @brief print_cdecl_brak : print cdecl_brack tree node information
 *
 * @param node : pointer to cdecl_brack tree node
 * @param pos : tree node position in syntax tree
 */
void print_cdecl_brak(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		'(' cdecl2 ')'
	 *		 | cdecl3 '(' cdecl_list ')'
	 */
	if (node->cdecl_brack.cdecl) {
		tree_t* cdecl = node->cdecl_brack.cdecl;
		cdecl->common.print_node(cdecl, pos);
	}
	if (node->cdecl_brack.decl_list) {
		tree_t* cdecl = node->cdecl_brack.decl_list;
		cdecl->common.print_node(cdecl, pos);
	}

	print_sibling(node, pos);

	return ;
}

/**
 * @brief print_ellipsis : print ellipsis tree node information
 *
 * @param node : pointer to ellipsis tree node
 * @param pos : tree node position in syntax tree
 */
void print_ellipsis(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		ELLIPSIS
	 *		| cdecl_list2 ',' ELLIPSIS
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "...", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_header : print head tree node information
 *
 * @param node : pointer to head tree node
 * @param pos : tree node position in syntax tree
 */
void print_header(tree_t* node, int pos) {
	/* Grammar: HEADER */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->head.str, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_footer : print foot tree node information
 *
 * @param node : pointer to foot tree node
 * @param pos : tree node position in syntax tree
 */
void print_footer(tree_t* node, int pos) {
	/* Grammar: HEADER */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->foot.str, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_while : print while tree node information
 *
 * @param node : pointer to while tree node
 * @param pos : tree node position in syntax tree
 */
void print_while(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		WHILE '(' expression ')' statement
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "while", pos);
	if (node->do_while.cond) {
		tree_t* cond = node->do_while.cond;
		cond->common.print_node(cond, pos);
	}

	if (node->do_while.block) {
		tree_t* block = node->do_while.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, pos);
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, pos);
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_delete : print while tree node information
 *
 * @param node : pointer to delete tree node
 * @param pos : tree node position in syntax tree
 */
void print_delete(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		DELETE expression ';'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "delete", pos);

	if (node->delete_tree.expr) {
		tree_t* expr = node->delete_tree.expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_goto : print while tree node information
 *
 * @param node : pointer to goto tree node
 * @param pos : tree node position in syntax tree
 */
void print_goto(tree_t* node, int pos) {
	/* Grammar: GOTO ident ';' */
	tree_t* label = node->goto_tree.label;

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, label->ident.str, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_default : print default tree node information
 *
 * @param node : pointer to default tree node
 * @param pos : tree node position in syntax tree
 */
void print_default(tree_t* node, int pos) {
	/* Grammar: DEFAULT ':' statement */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "default:", pos);
	if (node->default_tree.block) {
		tree_t* block = node->default_tree.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, pos);
		}
		else{
			if (block->common.print_node)
				block->common.print_node(block, pos);
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_switch : print switch tree node information
 *
 * @param node : pointer to switch tree node
 * @param pos : tree node position in syntax tree
 */
void print_switch(tree_t* node, int pos) {
	/* Grammar: SWITCH '(' expression ')' statement */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "switch", pos);

	if (node->switch_tree.cond) {
		tree_t* cond = node->switch_tree.cond;
		cond->common.print_node(cond, pos);
	}

	if (node->switch_tree.block) {
		tree_t* block = node->switch_tree.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, pos);
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, pos);
		}
	}

	print_sibling(node, pos);

	return;
}



void print_case(tree_t* node, int pos) {
   print_pos(pos);
    printf("[%s : %s : %d]\n",
           node->common.name, "case", pos);
   
   if (node->case_tree.expr) {
       tree_t* expr = node->case_tree.expr;
       expr->common.print_node(expr, pos);
   }

   if (node->case_tree.block) {
       tree_t* block = node->case_tree.block;
       if ((block->common.type) == BLOCK_TYPE) {
           tree_t* statement = block->block.statement;
		   if (statement && statement->common.print_node)
			   statement->common.print_node(statement, pos);
       }
       else {
		   if (block->common.print_node)
			   block->common.print_node(block, pos);
       }
   }

   print_sibling(node, pos);

   return;
}


/**
 * @brief print_for : print for tree node information
 *
 * @param node : pointer to for tree node
 * @param pos : tree node position in syntax tree
 */
void print_for(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		FOR '(' comma_expression_opt ';' expression_opt ';' comma_expression_opt ')' statement
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "for", pos);

	if (node->for_tree.init) {
		tree_t* init = node->for_tree.init;
		init->common.print_node(init, pos);
	}

	if (node->for_tree.cond) {
		tree_t* cond = node->for_tree.cond;
		cond->common.print_node(cond, pos);
	}

	if (node->for_tree.update) {
		tree_t* update = node->for_tree.update;
		update->common.print_node(update, pos);
	}

	if (node->for_tree.block) {
		tree_t* block = node->for_tree.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, pos);
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, pos);
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_do_while : print do ... while ... tree node information
 *
 * @param node : pointer to do... while... tree node
 * @param pos : tree node position in syntax tree
 */
void print_do_while(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		DO statement WHILE '(' expression ')' ';'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "do", pos);
	if (node->do_while.block) {
		tree_t* block = node->do_while.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, pos);
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, pos);
		}
	}

	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "while", pos);

	if (node->do_while.cond) {
		tree_t* cond = node->do_while.cond;
		cond->common.print_node(cond, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_layout : print layout  tree node information
 *
 * @param node : pointer to layout tree node
 * @param pos : tree node position in syntax tree
 */
void print_layout(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		LAYOUT STRING_LITERAL '{' layout_decls '}'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->layout.name, pos);
	if (node->layout.block) {
		tree_t* block = node->layout.block;
		if ((block->common.type) == BLOCK_TYPE) {
			tree_t* layout_decl = block->block.statement;
			if (layout_decl && layout_decl->common.print_node)
				layout_decl->common.print_node(layout_decl, (pos - 1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos - 1));
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_typeof : print typeof tree node information
 *
 * @param node : pointer to typeof tree node
 * @param pos : tree node position in syntax tree
 */
void print_typeof(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		TYPEOF expression
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "typeof", pos);
	if (node->typeof_tree.expr) {
		tree_t* expr = node->typeof_tree.expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_assert : print assert tree node information
 *
 * @param node : pointer to assert tree node
 * @param pos : tree node position in syntax tree
 */
void print_assert(tree_t* node, int pos) {
	/* Grammar: ASSERT expression ';' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, "data", pos);
	if (node->assert_tree.expr) {
		tree_t* expr = node->assert_tree.expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_cdecl : print cdecl tree node information
 *
 * @param node : pointer to cdecl tree node
 * @param pos : tree node position in syntax tree
 */
void print_cdecl(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		DATA cdecl ';'
	 *		| TYPEDEF cdecl ';'
	 *		| EXTERN TYPEDEF cdecl ';'
	 *		| basetype cdecl2
	 *		| CONST basetype cdecl2
	 *		| CONST cdecl2
	 *		| '*' cdecl2
	 *		| VECT cdecl2
	 */
	if (node->cdecl.is_data) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "data", pos);
	}
	if (node->cdecl.is_extern) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "extern", pos);
	}
	if (node->cdecl.is_typedef) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "typedef", pos);
	}
	if (node->cdecl.is_const) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "const", pos);
	}
	if (node->cdecl.is_point) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "*", pos);
	}
	if (node->cdecl.is_point_access) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "->", pos);
	}
	if (node->cdecl.is_vect) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "vect", pos);
	}

	if (node->cdecl.basetype) {
		tree_t* basetype = node->cdecl.basetype;
		DEBUG_BLACK("Line: %d, name: %s\n", __LINE__, basetype->common.name);
		basetype->common.print_node(basetype, pos);
	}
	if (node->cdecl.decl) {
		tree_t* decl = node->cdecl.decl;
		//printf("Line: %d, name: %s\n", __LINE__, decl->common.name);
		decl->common.print_node(decl, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_method_params : print method params tree node information
 *
 * @param node : pointer to method params tree node
 * @param pos : tree node position in syntax tree
 */
void print_method_params(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		'(' cdecl_or_ident_list ')'
	 *		| METHOD_RETURN '(' cdecl_or_ident_list ')'
	 *		| '(' cdecl_or_ident_list ')' METHOD_RETURN '(' cdecl_or_ident_list ')'
	 */
	if(node->params.in_params) {
		tree_t* in_params = node->params.in_params;
		in_params->common.print_node(in_params, pos);
	}

	if (node->params.ret_params) {
		print_pos(pos);
		printf("[%s : %s: %d]\n",
				node->common.name, "->", pos);
		tree_t* ret_params = node->params.ret_params;
		ret_params->common.print_node(ret_params, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_throw : print throw tree node information
 *
 * @param node : pointer to throw tree node
 * @param pos : tree node position in syntax tree
 */
void print_throw(tree_t* node, int pos) {
	/* Grammar: THROW ';' */
	print_pos(pos);
	printf("[%s : %s: %d]\n",
			node->common.name, "throw", pos);

	print_sibling(node, pos);
}

/**
 * @brief print_local_keyword : print local keyword tree node information
 *
 * @param node : pointer to local keyword tree node
 * @param pos : tree node position in syntax tree
 */
void print_local_keyword(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		LOCAL
	 *		| AUTO
	 */
	print_pos(pos);
	printf("[%s : %s: %d]\n",
		node->common.name, node->local_keyword.name, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_local : print local tree node information
 *
 * @param node : pointer to local tree node
 * @param pos : tree node position in syntax tree
 */
void print_local(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		local_keyword cdecl
	 *		|  STATIC cdecl ';'
	 *		| local_keyword cdecl '=' expression ';'
	 *		| STATIC cdecl '=' expression ';'
	 */
	if (node->local_tree.local_keyword) {
		tree_t* local_keyword = node->local_tree.local_keyword;
		local_keyword->common.print_node(local_keyword, pos);
	}

	if (node->local_tree.is_static) {
		print_pos(pos);
		printf("[%s : %s: %d]\n",
			node->common.name, "static", pos);
	}

	if (node->local_tree.cdecl) {
		tree_t* cdecl = node->local_tree.cdecl;
		cdecl->common.print_node(cdecl, pos);
	}

	if (node->local_tree.expr) {
		tree_t* expr = node->local_tree.expr;
		expr->common.print_node(expr, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_if_else : print if... else... tree node information
 *
 * @param node : pointer to if...else... tree node
 * @param pos : tree node position in syntax tree
 */
void print_if_else(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		IF '(' expression ')' '{' object_statements '}'
	 *		| IF '(' expression ')' '{' object_statements '}' ELSE '{' object_statements '}'
	 *		| IF '(' expression ')' '{' object_statements '}' ELSE object_if
	 *		| IF '(' expression ')' statement
	 *		| IF '(' expression ')' statement ELSE statement
	 */
	print_pos(pos);
	printf("[%s : %s: %d]\n",
			node->common.name, "if", pos);
	if (node->if_else.cond) {
		tree_t* cond = node->if_else.cond;
		cond->common.print_node(cond, pos);
	}
	else {
		fprintf(stderr, "There need the condition of if\n");
	}

	if (node->if_else.if_block) {
		tree_t* if_block = node->if_else.if_block;
		if (if_block->common.type == BLOCK_TYPE) {
			tree_t* statement = if_block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (if_block->common.print_node)
				if_block->common.print_node(if_block, (pos - 1));
		}
	}

	if (node->if_else.else_if) {
		print_if_else(node->if_else.else_if, pos);
	}

	if (node->if_else.else_block) {
		print_pos(pos);
		printf("[%s : %s: %d]\n",
				node->common.name, "else", pos);
		tree_t* block = node->if_else.else_block;
		if (block->common.type == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos - 1));
		}
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_method : print method tree node information
 *
 * @param node : pointer to method tree node
 * @param pos : tree node position in syntax tree
 */
void print_method (tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		METHOD objident method_params compound_statement
	 *		| METHOD objident method_params DEFAULT compound_statement
	 *		| METHOD EXTERN objident method_params compound_statement
	 *		| METHOD EXTERN objident method_params DEFAULT compound_statement
	 */
	int is_extern = node->method.is_extern;
	int is_default = node->method.is_default;
	print_pos(pos);
	printf("[%s : %s : %s : %s : %d]\n",
			node->common.name, is_extern ? "extern" : "",
			node->method.name, is_default ? "default": "", pos);
	if (node->method.params) {
		tree_t* params = node->method.params;
		params->common.print_node(params, pos);
	}
	if (node->method.block) {
		tree_t* block = node->method.block;
		tree_t* statement = block->block.statement;
		if (statement && (statement->common.print_node))
			statement->common.print_node(statement, --pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_loggroup : print loggroup tree node information
 *
 * @param node : pointer to loggroup tree node
 * @param pos : tree node position in syntax tree
 */
void print_loggroup(tree_t* node, int pos) {
	/* Grammar: LOGGROUP ident ';' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->loggroup.name, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_device : print device tree node information
 *
 * @param node : pointer to device tree node
 * @param pos : tree node position in syntax tree
 */
void print_device(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		DEVICE objident ';' syntax_modifiers device_statements
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->device.name, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_log : print log tree node information
 *
 * @param node : pointer to log tree node
 * @param pos : tree node position in syntax tree
 */
void print_log(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		LOG STRING_LITERAL ',' expression ',' expression ':' STRING_LITERAL ',' log_args ';'
	 *		| LOG STRING_LITERAL ',' expression ':' STRING_LITERAL log_args ';'
	 *		| LOG STRING_LITERAL ':' STRING_LITERAL log_args ';'
	 */
	print_pos(pos);
	if (node->log.log_type) {
		printf("[%s : %s : %s : %d]\n",
			node->common.name, "log", node->log.log_type, pos);
	}
	else {
		printf("[%s : %s : %d]\n",
			node->common.name, "log", pos);
	}

	if (node->log.level) {
		tree_t* level = node->log.level;
		level->common.print_node(level, pos);
	}

	if (node->log.group) {
		tree_t* group = node->log.group;
		group->common.print_node(group, pos);
	}

	if (node->log.format) {
		print_pos(pos);
		printf("%s\n", node->log.format);
	}

	if (node->log.args) {
		tree_t* args = node->log.args;
		args->common.print_node(args, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_break : print break tree node information
 *
 * @param node : pointer to break tree node
 * @param pos : tree node position in syntax tree
 */
void print_break(tree_t* node, int pos) {
	/* Grammar: BREAK ';' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
		node->common.name, "break", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_continue : print continue tree node information
 *
 * @param node : pointer to continue tree node
 * @param pos : tree node position in syntax tree
 */
void print_continue(tree_t* node, int pos) {
	/* Grammar: CONTINUE ';' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
		node->common.name, "continue", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_return : print return tree node information
 *
 * @param node : pointer to return tree node
 * @param pos : tree node position in syntax tree
 */
void print_return(tree_t* node, int pos) {
	/* Grammar: RETURN ';' */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
		node->common.name, "return", pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_error: print error tree node information
 *
 * @param node : pointer to error tree node
 * @param pos : tree node position in syntax tree
 */
void print_error(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		ERROR ';'
	 *		| ERROR STRING_LITERAL ';'
	 */
	print_pos(pos);
	if (node->error.str) {
		printf("[%s : %s : %d]\n",
			node->common.name, node->error.str, pos);
	}
	else {
		printf("[%s : %s : %d]\n",
			node->common.name, "error", pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_call_inline : print call_inline tree node information
 *
 * @param node : pointer to call_inline tree node
 * @param pos : tree node position in syntax tree
 */
void print_call_inline(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		CALL expression returnargs ';'
	 *		| INLINE expression returnargs ';'
	 */
	if (node->common.type == CALL_TYPE) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "call", pos);
	}
	if (node->common.type == INLINE_TYPE) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "inline", pos);
	}

	if (node->call_inline.expr) {
		tree_t* expr = node->call_inline.expr;
		expr->common.print_node(expr, pos);
	}
	if (node->call_inline.ret_args) {
		print_pos(pos);
		printf("[%s : %s : %d]\n",
				node->common.name, "->", pos);
		tree_t* ret_args = node->call_inline.ret_args;
		ret_args->common.print_node(ret_args, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_foreach : print foreach tree node information
 *
 * @param node : pointer to foreach tree node
 * @param pos : tree node position in syntax tree
 */
void print_foreach(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		FOREACH ident IN '(' expression ')' statement
	 */
	if (node->foreach.ident) {
		tree_t* ident = node->foreach.ident;
		print_pos(pos);
		printf("[%s : %s IN : %d]\n",
				node->common.name, ident->ident.str, pos);
	}
	else {
		fprintf(stderr, "foreach need identifier\n");
	}

	if (node->foreach.in_expr) {
		tree_t* expr = node->foreach.in_expr;
		expr->common.print_node(expr, pos);
	}

	if (node->foreach.block) {
		tree_t* block = node->foreach.block;
		if (block->common.type == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos -1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos - 1));
		}
	}

	print_sibling(node, pos);

	return;
}

void print_label(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		ident ':' statement
	 */
	if (node->label.ident) {
		tree_t* ident = node->label.ident;
		print_pos(pos);
		printf("[%s : %s IN : %d]\n",
				node->common.name, ident->ident.str, pos);
	}

	if (node->label.block) {
		tree_t* block = node->label.block;
		if (block->common.type == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			if (statement && statement->common.print_node)
				statement->common.print_node(statement, (pos - 1));
		}
		else {
			if (block->common.print_node)
				block->common.print_node(block, (pos -1));
		}
	}

	return;
}

/**
 * @brief print_list : print all list node information from list second node
 *
 * @param head : the list head node
 * @param pos : node's position in syntax tree
 */
void print_list(tree_t* head, int pos) {
	tree_t* node = head;
	node->common.print_node(node, pos);
	while (node->common.sibling != NULL) {
		node = node->common.sibling;
		DEBUG_BLACK("IN %s, line = %d, name: %s, pos: %d\n", __FUNCTION__, __LINE__, node->common.name, pos);
		node->common.print_node(node, pos);
	}

	return;
}

/**
 * @brief print_field : print field tree node information
 *
 * @param node : pointer to field tree node
 * @param pos : tree node position in syntax tree
 */
void print_field(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		FIELD objident bitrange istemplate object_spec
	 *		| FIELD objident istemplate object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->field.name, pos);

	if (node->field.bitrange) {
		tree_t* bitrange = node->field.bitrange;
		DEBUG_BLACK("Line: %d, name: %s\n", __LINE__, bitrange->common.name);
		bitrange->common.print_node(bitrange, pos);
	}

	if (node->field.templates) {
		tree_t* templates = node->field.templates;
		templates->common.print_node(templates, pos);
	}

    obj_spec_t* spec = node->field.spec;
    while (spec) {
        tree_t* obj_spec = spec->node;
        obj_spec->common.print_node(obj_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_obj_block : print obj_block tree node information
 *
 * @param node : pointer to obj_block tree node
 * @param pos : tree node position in syntax tree
 */
void print_obj_block(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		object_desc '{' object_statements '}'
	 */
	if (node->block.statement != NULL) {
		tree_t* statement = node->block.statement;
		statement->common.print_node(statement, --pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_object_desc : print object_desc tree node information
 *
 * @param node : pointer to object_desc tree node
 * @param pos : tree node position in syntax tree
 */
void print_object_desc (tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		object_desc '{' object_statements '}'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->string.pointer, pos);

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_object_spec : print object_spec tree node information
 *
 * @param node : pointer to object_spec tree node
 * @param pos : tree node position in syntax tree
 */
void print_object_spec(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		object_desc ';
	 *		| object_desc '{' object_statements '}'
	 */
	if (node->spec.desc != NULL) {
		tree_t* desc = node->spec.desc;
		desc->common.print_node(desc, pos);
	}
	if (node->spec.block != NULL) {
		tree_t* block = node->spec.block;
		block->common.print_node(block, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_template : print template tree node information
 *
 * @param node : pointer to template tree node
 * @param pos : tree node position in syntax tree
 */
void print_template(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		TEMPLATE objident object_spec
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->temp.name, pos);

    obj_spec_t* spec = node->temp.spec;
    tree_t* temp_spec = NULL;
    while (spec) {
        temp_spec = spec->node;
        temp_spec->common.print_node(temp_spec, pos);
        spec = spec->next;
    }

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_struct : print struct tree node information
 *
 * @param node : pointer to struct tree node
 * @param pos : tree node position in syntax tree
 */
void print_struct(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		STRUCT '{' struct_decls '}'
	 *		|  STRUCT ident '{' struct_decls '}'
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->struct_tree.name, pos);

	if (node->struct_tree.block) {
		tree_t* block = node->struct_tree.block;
		if (block->common.print_node)
			block->common.print_node(block, --pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_dml : print dml tree node information
 *
 * @param node : pointer to dml tree node
 * @param pos : tree node position in syntax tree
 */
void print_dml(tree_t* node, int pos) {
	/*
	 * Grammar:
	 *		DML FLOAT_LITERAL';' dml
	 */
	print_pos(pos);
	printf("[%s : %s : %d]\n",
			node->common.name, node->dml.version, pos);
	if (node->common.child) {
		tree_t* child = node->common.child;
		if (child->common.print_node)
			child->common.print_node(child, pos);
	}

	print_sibling(node, pos);

	return;
}

/**
 * @brief print_ast : print the syntax tree information
 *
 * @param root : the root of syntax tree
 */
void print_ast (tree_t* root)
{
	int init_pos = 29;
	tree_t* it = root;
	tree_t* child = NULL;
	printf ("begin print the ast(total node num is %d):\n", node_num);
	printf ("---------------------\n");
	root->common.print_node(root, init_pos);

	return;
}

symbol_t get_symbol_from_root_table(const char* name, type_t type);
void check_reg_table(void) {
	symbol_t symbol = get_symbol_from_root_table("regs", 0);
	object_attr_t* attr = symbol->attr;
	symtab_t table = attr->common.table;
	printf("regs table: %d, type: %d, BANK_TYPE: %d\n", table->table_num, symbol->type, BANK_TYPE);
#if 0
	symbol_t reg = symbol_find(table, "UartDR", REGISTER_TYPE);
	object_attr_t* reg_attr = reg->attr;
	symtab_t reg_table = reg_attr->common.table;
	printf("reg_table: %d, parent: 0x%x\n",
		reg_table->table_num, reg_table->parent);
	printf("reg_table parent: %d\n", reg_table->parent->table_num);
#endif

	return;
}







