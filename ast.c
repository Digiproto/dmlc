
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

#ifdef AST_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

/**
 * @brief get_digits: get the digits from string
 *
 * @param str: the string of digits
 * @param base: the carinal number of  digits: 16,10,8
 *
 * @return: the integer of digits
 */
long get_digits (char *str, int base)
{
	char *endptr = NULL;
	long value = 0;

	/* To distinguish success/failure after call */
	errno = 0;

	value = strtol (str, &endptr, base);

	/* Check for various possible errors */
	if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
		|| (errno != 0 && value == 0)) {
		perror ("strtol");
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
long int strtoi (char *str)
{
	char *stop = NULL;
	char *first_pos = NULL;
	long int value = 0;
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

static node_t *find_tail (node_t * head)
{
	node_t *it = head;
	while (it->sibling != NULL) {
		it = it->sibling;
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
node_t *find_node (node_t * node, int type)
{
	if (node == NULL) {
		return NULL;
	}
	if ((node->type) == type) {
		DBG ("node find, name: %s\n", node->name);
		return node;
	}
	else if (((node->child) != NULL) && ((node->type) != type)) {
		return find_node (node->child, type);
	}
	else if (((node->sibling) != NULL) && ((node->type) != type)) {
		return find_node (node->sibling, type);
	}
	else {
		printf ("Not find the node.\n");
		return NULL;
	}
}

void add_child (node_t * parent, node_t * child)
{
	assert (parent != NULL);
	DEBUG_ADD_CHILD ("In %s, child->name=%s\n", __FUNCTION__, child->name);
	parent->child = child;
}

static int node_num = 0;

node_t *create_node (char *name, int type)
{
	node_t *node = (node_t *) malloc (sizeof (node_t));
	node->name = strdup (name);
	node->sibling = NULL;
	node->child = NULL;
	DEBUG_CREATE_NODE ("In %s, name=%s, type = %d\n", __FUNCTION__, name, type);
	node->type = type;
	node_num++;
	return node;
}

/**
 * @brief concentrate the two node list
 *
 * @param root
 * @param new_node
 *
 * @return
 */
node_t *create_node_list (node_t * root, node_t * new_node)
{
	if (root != NULL && new_node != NULL) {
		DEBUG_CREATE_NODE_LIST ("In %s, root->name=%s, new_node->name=%s\n",
								__FUNCTION__, root->name, new_node->name);
	}
	assert (new_node != NULL);
	assert (root != NULL);

	node_t *tail = find_tail (root);
	assert (tail->sibling == NULL);
	tail->sibling = new_node;
	return root;
}

/**
 * @brief
 *
 * @param node
 */
void print_pos (int i)
{
	while ((i--) > 0)
		printf ("|    ");
}

int init_pos = 20;
void print_node (node_t * node, int pos)
{
	if (node->child != NULL) {
		pos--;
		//printf("pos=%d, father is %s, child is %s\n", pos, node->name, node->child->name);
		print_node (node->child, pos);
		pos++;
		//printf("after pos=%d, father is %s, child is %s\n", pos, node->name, node->child->name);
	}
	print_pos (pos);
	printf ("[%s, %d, pos=%d]\n", node->name, node->type, pos);
	node_t *sibling = node->sibling;
	if (sibling) {
		//printf("the first child is %s, current sibling=%s\n", node->name, sibling->name);
		print_node (sibling, pos);
		sibling = sibling->sibling;
	}
}

/**
 * @brief print the whole ast for debug purpose
 */
void print_ast (node_t * root)
{
	node_t *it = root;
	node_t *child = NULL;
	printf ("begin print the ast(total node num is %d):\n", node_num);
	printf ("---------------------\n");
	print_node (root, init_pos);
}
