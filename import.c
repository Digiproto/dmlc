/*
 * import.c:
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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "import.h"
#include "ast.h"

struct file_stack* push_file_stack(struct file_stack* top, const char* name)
{
	assert(name != NULL);

	if(top != NULL) {
		/* file stack is not empty. */
		struct file_stack* tmp = top->file_history;
		/* stop import, if the file have been imported before. */
		while(tmp != NULL) {
			if(strcmp(tmp->name, name) == 0) {
				printf("Have imported a same file: %s\n", name);
				return NULL;
			}
			tmp = tmp->next;
		}
	}

	struct file_stack* new_stack = gdml_zmalloc(sizeof(struct file_stack));
	new_stack->name = strdup(name);
	new_stack->next = top;
	struct file_stack* new_history = gdml_zmalloc(sizeof(struct file_stack));
	new_history->name = new_stack->name;
	new_history->next = NULL;

	if(top != NULL) {
		new_stack->file_history = top->file_history;
		/* insert new file name into file history list end.*/
		struct file_stack* tmp = top->file_history;
		while(tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = new_history;
	}else{
		new_stack->file_history = new_history;
	}
	return new_stack;
}

struct file_stack* pop_file_stack(struct file_stack* top)
{
	assert(top != NULL);
	/* node may be used as error information, so it can't be free. */
	return top->next;
}
