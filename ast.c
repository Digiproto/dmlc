
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
		fprintf (stderr, "No digits were found\n");
		exit (EXIT_FAILURE);
	}

	if (*endptr != '\0') {
		fprintf (stderr, "Further characters after number(%s : %s)\n", str,
				 endptr);
		exit (EXIT_FAILURE);
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
		fprintf (stderr, "The string changed to int is NULL\n");
		return -1;
	}

	if (((first_pos = strstr (str, "0x")) != NULL) ||
		((first_pos = strstr (str, "0X")) != NULL)) {
		DBG ("Value: %s\n", str);
		value = get_digits (str, 16);
	}							//hex
	else if (((first_pos = strchr (str, '0')) != NULL) &&
			 ((str[first_pos - str + 1]) >= '0') &&
			 ((str[first_pos - str + 1]) < '9')) {
		DBG ("Value: %s\n", str);
		value = get_digits (str, 8);
	}							// Octal
	else {
		DBG ("Value: %s\n", str);
		value = get_digits (str, 10);
	}							//decimal

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
		printf ("Not find the node.\n");
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

static int node_num = 0;
/**
 * @brief create_node : create a tree node
 *
 * @param name : node name
 * @param type : node type
 * @param size : node size
 *
 * @return : return a pointer to created node
 */
tree_t* create_node (char *name, int type, int size)
{
	tree_t* node = (tree_t*)gdml_zmalloc(size) ;
	node->common.name = strdup (name);
	node->common.sibling = NULL;
	node->common.child = NULL;
	DEBUG_CREATE_NODE ("In %s, node: 0x%x, name=%s, type = %d\n", __FUNCTION__, node, name, type);
	node->common.type = type;
	node_num++;

	return node;
}

/**
 * @brief dml_keyword_node : make a node for dml keyword
 *
 * @param name: dml keyword
 *
 * @return : return a pointer to the node
 */
tree_t* dml_keyword_node(const char* name) {
    tree_t* node = create_node("dml_keyword", DML_KEYWORD_TYPE, sizeof(struct tree_ident));
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
 * @return : return a pointer to node
 */
tree_t* c_keyword_node (const char* name) {
    tree_t* node = create_node("c_keyword", C_KEYWORD_TYPE, sizeof(struct tree_ident));
    node->ident.str = strdup(name);
    node->ident.len = strlen(name);
	node->common.print_node = print_ident;

    return node;
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

char* get_obj_desc(tree_t* spec) {
	if (spec == NULL) {
		return NULL;
	}
	if (spec->spec.desc) {
		tree_t* tmp = spec->spec.desc;
		DEBUG_AST("In %s, line = %d, obj_desc: %s\n",
				__FUNCTION__, __LINE__, tmp->string.pointer);
		return (char*)(tmp->string.pointer);
	}
	else {
		return NULL;
	}
}

int get_int_value(tree_t* node) {
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		return node->int_cst.value;
	}
	else {
		fprintf(stderr, "In %s, line = %d, other node expression, name: %s, type: %d\n",
				__FUNCTION__, __LINE__, node->common.name, node->common.type);
		exit(-1);
	}
}

arraydef_attr_t* get_range_arraydef(tree_t* node, arraydef_attr_t* array) {
	if (node == NULL) {
		return NULL;
	}
	if (node->array.ident) {
		tree_t* ident = node->array.ident;
		if (ident->common.type == IDENT_TYPE) {
			array->ident = ident->ident.str;
		}
		else {
			fprintf(stderr, "The array identifier type: %d, name: %s\n",
					ident->common.type, ident->common.name);
			exit(-1);
		}
	}
	else {
		fprintf(stderr, "The array need identifier\n");
		exit(-1);
	}
	if (node->array.expr) {
		array->low = get_int_value(node->array.expr);
	}
	else {
		fprintf(stderr, "The array need start array number\n");
		exit(-1);
	}
	if (node->array.expr_end) {
		array->high = get_int_value(node->array.expr_end);
	}
	else {
		fprintf(stderr, "The array need end array number\n");
		exit(-1);
	}

	return array;
}

arraydef_attr_t* get_arraydef(tree_t* node) {
	if (node == NULL) {
		return NULL;
	}
	arraydef_attr_t* arraydef = (arraydef_attr_t*)gdml_zmalloc(sizeof(arraydef_attr_t));
	if ((node->array.is_fix) == 1) {
		arraydef->fix_array = get_int_value(node->array.expr);
	}
	else {
		arraydef = get_range_arraydef(node, arraydef);
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

	return node->string.pointer;
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
		spec->expr = parse_expression(node->paramspec.expr, table);
		spec->type = spec->expr->final_type;
		DEBUG_AST("paramspec is expression expr type: %d\n", spec->type);
	}

	return spec;
}

int get_size(tree_t* node) {
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		int size = node->int_cst.value;
		if ((size > 0) && (size <= 8)) {
			return size;
		}
		else {
			fprintf(stderr, "the size(%d) out\n", size);
		}
	}
	else {
		DEBUG_AST("The size is another type expression - node type: %d, name: %s\n", node->common.type, node->common.name);
		exit(-1);
	}

	return 0;
}

int get_offset(tree_t* node) {
	if (node == NULL) {
		return -1;
	}
	if (node->common.type == INTEGER_TYPE) {
		int offset = node->int_cst.value;
		return offset;
	}
	else {
		DEBUG_AST("The size is another type expression - node type: %d, name: %s\n", node->common.type, node->common.name);
		exit(-1);
	}

	return 0;
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

void add_template_to_table(symtab_t table, char* template) {
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
			fprintf(stderr, "re-load template: %s\n", template);
			/*FIXME: should handle the error */
			exit(-1);
		}
		pre_temp_table = temp_table;
		temp_table = temp_table->next;
	}

	new_table = (struct template_table*)gdml_zmalloc(sizeof(struct template_table));

	symbol_t symbol = symbol_find(table, template, TEMPLATE_TYPE);
	if (symbol == NULL) {
		fprintf(stderr, "can not find the template: %s\n", template);
		/* FIXME: should handle the error */
		exit(-1);
	}
	template_attr_t* attr = symbol->attr;
	new_table->table = attr->table;
	new_table->template_name = strdup(template);

	if (table->template_table == NULL) {
		table->template_table = new_table;
	}
	else {
		DEBUG_AST("pre_temp_table : %s\n", pre_temp_table->template_name);
		//table->template_table  = new_table;
		pre_temp_table->next = new_table;
	}

	DEBUG_ADD_TEMPLATE("In %s, line = %d, insert templates table: %s, num: %d\n",
			__func__, __LINE__, template, new_table->table->table_num);

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

		add_template_to_table(table, templates[i]);
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

	switch(node->common.type) {
		case BANK_TYPE:
			{
				bank_attr_t* attr = (bank_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case REGISTER_TYPE:
			{
				register_attr_t* attr = (register_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->templates_num;
				break;
			}
		case FIELD_TYPE:
			{
				field_attr_t* attr = (field_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case CONNECT_TYPE:
			{
				connect_attr_t* attr = (connect_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case INTERFACE_TYPE:
			{
				interface_attr_t* attr = (interface_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case ATTRIBUTE_TYPE:
			{
				attribute_attr_t* attr = (attribute_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case EVENT_TYPE:
			{
				event_attr_t* attr = (event_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case GROUP_TYPE:
			{
				group_attr_t* attr = (group_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case PORT_TYPE:
			{
				port_attr_t* attr = (port_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case IMPLEMENT_TYPE:
			{
				implement_attr_t* attr = (implement_attr_t*)(node->common.attr);
				templates = attr->templates;
				template_num = attr->template_num;
				break;
			}
		case TEMPLATE_TYPE:
			break;
		default:
			fprintf(stderr, "unknown object type(%d) : %s",
					node->common.type, node->common.name);
			/* FIXME handle the error */
			break;
	}

	add_templates_to_table(table, templates, template_num);

	return;
}

char** get_templates(tree_t* head) {
	if (head == NULL) {
		return NULL;
	}
	int num = get_list_num(head);
	char** templates = gdml_zmalloc(num * sizeof(char*));
	tree_t* node = head;
	int i = 0;
	while (node != NULL) {
		if (((node->common.type) == DML_KEYWORD_TYPE) ||
				((node->common.type) == IDENT_TYPE)) {
			templates[i++] = node->ident.str;
			DEBUG_AST("identifier:  %s : %s\n", node->ident.str, templates[i - 1]);
		}
		else {
			fprintf(stderr, "The templates'type is(%d) : %s: %s\n",
					node->common.type, node->common.name, node->ident.str);
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
		DEBUG_BLACK("Line: %d, name: %s\n", __LINE__, sibling->common.name);
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
	printf("[%s : %s : %d]\n",
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
		DEBUG_BLACK("Line = %d, name: %s\n", __LINE__, expr->common.name);
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
			statement->common.print_node(statement, (pos - 1));
		}
		else {
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
			statement->common.print_node(statement, (pos - 1));
		}
		else {
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

	if (node->connect.spec) {
		tree_t* obj_spec = node->connect.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->interface.spec) {
		tree_t* obj_spec = node->interface.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->attribute.spec) {
		tree_t* obj_spec = node->attribute.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->event.spec) {
		tree_t* obj_spec = node->event.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->group.spec) {
		tree_t* obj_spec = node->group.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->port.spec) {
		tree_t* obj_spec = node->port.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	if (node->implement.spec) {
		tree_t* obj_spec = node->implement.spec;
		obj_spec->common.print_node(obj_spec, pos);
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

	return 0;
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
		tree_t* templates = node->reg.templates;
		templates->common.print_node(templates, pos);
	}
	if (node->reg.spec) {
		tree_t* obj_spec = node->reg.spec;
		obj_spec->common.print_node(obj_spec, pos);
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
	if (node->bank.spec) {
		tree_t* obj_spec = node->bank.spec;
		obj_spec->common.print_node(obj_spec, pos);
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
			statement->common.print_node(statement, (pos -1));
		}
		else {
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
			statement->common.print_node(statement, (pos - 1));
		}
		else {
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

	return 0;
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
			statement->common.print_node(statement, pos);
		}
		else {
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
			statement->common.print_node(statement, pos);
		}
		else{
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
			statement->common.print_node(statement, pos);
		}
		else {
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
			statement->common.print_node(statement, pos);
		}
		else {
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
			statement->common.print_node(statement, pos);
		}
		else {
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
			layout_decl->common.print_node(layout_decl, (pos - 1));
		}
		else {
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
			statement->common.print_node(statement, (pos - 1));
		}
		else {
			if_block->common.print_node(if_block, (pos - 1));
		}
	}

	if (node->if_else.else_block) {
		print_pos(pos);
		printf("[%s : %s: %d]\n",
				node->common.name, "else", pos);
		tree_t* block = node->if_else.else_block;
		if (block->common.type == BLOCK_TYPE) {
			tree_t* statement = block->block.statement;
			statement->common.print_node(statement, (pos - 1));
		}
		else {
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
		//printf("LINE: %d, name: %s\n", __LINE__, statement->common.name);
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
			statement->common.print_node(statement, (pos -1));
		}
		else {
			block->common.print_node(block, (pos - 1));
		}
	}

	print_sibling(node, pos);

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

	if (node->field.spec) {
		tree_t* obj_spec = node->field.spec;
		obj_spec->common.print_node(obj_spec, pos);
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
	tree_t* temp_spec = node->temp.spec;
	temp_spec->common.print_node(temp_spec, pos);

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
			node->common.name, "struct", pos);
	if (node->struct_tree.ident) {
		tree_t* ident = node->struct_tree.ident;
		ident->common.print_node(ident, pos);
	}

	if (node->struct_tree.block) {
		tree_t* block = node->struct_tree.block;
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
		DEBUG_BLACK("In %s, line = %d, dml child name: %s\n",
				__FUNCTION__, __LINE__, child->common.name);
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
	int init_pos = 25;
	tree_t* it = root;
	tree_t* child = NULL;
	printf ("begin print the ast(total node num is %d):\n", node_num);
	printf ("---------------------\n");
	root->common.print_node(root, init_pos);

	return;
}
