/*
 * stack.c:
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "symbol-common.h"
#include "ast.h"

stack_t* initstack() {
	stack_t* ps = (stack_t*)gdml_zmalloc(sizeof(stack_t));
	if (ps != NULL) {
		ps->top = NULL;
		ps->size = 0;
	}

	return ps;
}

int is_empty(stack_t* ps) {
	if ((ps->top == NULL) && (ps->size == 0)) {
		return 1;
	}
	else {
		return 0;
	}
}

int get_stack_size(stack_t* ps) {
	return ps->size;
}

stack_node_t* get_top(stack_t* ps) {
	return ps->top;
}

stack_node_t* push(stack_t* ps, symtab_t item) {
	//printf("\n\npush table num: %d\n", item->table_num);
	stack_node_t* node = (stack_node_t*)gdml_zmalloc(sizeof(stack_node_t));
	node->data = item;
	node->down = get_top(ps);
	ps->size++;
	ps->top = node;

	return node;
}

symtab_t pop(stack_t* ps) {
	stack_node_t* node = ps->top;
	//printf("\n\npop table num: %d\n", ps->top->data->table_num);
	if (is_empty(ps) != 1) {
		ps->size--;
		ps->top = ps->top->down;
	}

	return (node->data);
}

void stack_traverse(stack_t* ps) {
	stack_node_t* node = ps->top;
	int i = ps->size;
	while (i--) {
		node = node->down;
	}

	return;
}

#if 0
int main(int argc, char* argv[]) {
	stack_t* ps  = initstack();
	int i;
	stack_node_t* node;
	for (i = 0; i < 10; i++) {
		push(ps, i);
		node = get_top(ps);
		printf("%d", node->data);
	}
	printf("\n");

	stack_traverse(ps);

	for (i = 0; i < 10; i++) {
		printf("pop i : %d, data : %d\n", i, node->data);
		node  = pop(ps);
	}
	printf("hhhhhhhhhhhhhhhhhhhhh\n");

	return 0;
}
#endif
