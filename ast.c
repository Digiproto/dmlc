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
#include "ast.h"
/*
typedef struct Exp_list { 
  node*             elem;
  struct list* next;
} ast_list;
*/
#ifdef AST_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

static node_t* find_tail(node_t* head){
	node_t* it = head;
	while(it->sibling != NULL){
		it = it->sibling;
		DBG("In %s, head=0x%x, it=0x%x\n", __FUNCTION__, head, it);
	}
	return it;
}
#if 0
//node_t* root = NULL;
node_t* create_ast(char* name){
	node_t* root = (node_t *)malloc(sizeof(node_t));
	root->name = strdup(name);
	root->type = 0;
	root->sibling = NULL;
	return root;
}
#endif
void add_child(node_t* parent, node_t* child){
	assert(parent != NULL);
	DBG("In %s, child->name=%s\n", __FUNCTION__, child->name);
	parent->child = child;
}
static int node_num = 0;
node_t* create_node(char* name, int type){
	node_t* node = (node_t *)malloc(sizeof(node_t));
	node->name = strdup(name);
	node->sibling = NULL;
	node->child = NULL;
	DBG("In %s, name=%s, type = %d\n", __FUNCTION__, name, type);
	node->type = type;
	node_num ++;
	return node;
	/*
	current->child = node;
	current = node;	
	*/
}

/**
* @brief concentrate the two node list
*
* @param root
* @param new_node
*
* @return 
*/
node_t* create_node_list(node_t* root, node_t* new_node){
	if(root != NULL && new_node != NULL)
		DBG("In %s, root->name=%s, new_node->name=%s\n", __FUNCTION__, root->name, new_node->name);
	assert(new_node != NULL);
	assert(root != NULL);
		
	node_t* tail = find_tail(root);
	assert(tail->sibling == NULL);
	tail->sibling = new_node;
	return root;
}

/**
* @brief 
*
* @param node
*/
//char* init_pos = "\t\t\t\t\t\t";
void print_pos(int i){
	while(i--)
		printf("\t");
}
int init_pos = 6;
void print_node(node_t* node, int pos){
	//print_pos(pos);
	if(node->child != NULL){
		pos --;
		//printf("pos=%d, father is %s, child is %s\n", pos, node->name, node->child->name);
		print_node(node->child, pos);
		pos ++;
		//printf("after pos=%d, father is %s, child is %s\n", pos, node->name, node->child->name);
	}
	print_pos(pos);
	printf("[%s, %d, pos=%d]\n", node->name, node->type, pos);
	node_t* sibling = node->sibling;
	if(sibling){
		//printf("the first child is %s, current sibling=%s\n", node->name, sibling->name);
		print_node(sibling, pos);
		sibling = sibling->sibling;
	}
}
/**
* @brief print the whole ast for debug purpose
*/
void print_ast(node_t* root){
	node_t* it = root;
	node_t* child = NULL;
	printf("begin print the ast(total node num is %d):\n", node_num);
	printf("---------------------\n");
	print_node(root, init_pos);
	
	#if 0
	while(it != NULL){
		print()
		/* print its child firstly */
		if((child = it->child) != NULL){
			do{
			}while(it->)
		}
	}
	#endif
	#if 0
	if(root)
		printf("root->name=%s\n", root->name);
	if(root->child)
		printf("root->child->name=%s\n", root->child->name);
	if(root->child->sibling)
		printf("root->child->sibling->name=%s\n", root->child->sibling->name);
	if(root->child->sibling->sibling)
		printf("root->child->sibling->name=%s\n", root->child->sibling->sibling->name);
	if(root->child->sibling->sibling->child)
		printf("root->child->sibling->child->name=%s\n", root->child->sibling->sibling->child->name);
	#endif
}
