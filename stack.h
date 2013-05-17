/*
 * stack.h:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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

#ifndef __STACK_H__
#define __STACK_H__

#include "symbol-common.h"

typedef struct stack_node stack_node_t;

struct stack_node {
	symtab_t data;
	stack_node_t* down;
};

typedef struct stack {
	stack_node_t* top;
	int size;
}stack_t;

stack_t* initstack();
void destroy_stack(stack_t* ps);
int is_empty(stack_t* ps);
int get_stack_size(stack_t* ps);
stack_node_t* get_top(stack_t* ps);
stack_node_t* push(stack_t* ps, symtab_t item);
symtab_t pop(stack_t* ps);

#endif /* __STACK_H__ */
