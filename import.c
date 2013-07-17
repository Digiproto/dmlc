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

struct file_stack* push_file_stack(struct file_stack* top, const char* name)
{
	struct file_stack* new_stack = malloc(sizeof(struct file_stack));
	assert((new_stack != NULL) && (name != NULL));
	new_stack->name = strdup(name);
	new_stack->next = top;
	return new_stack;
}

struct file_stack* pop_file_stack(struct file_stack* top)
{
	assert(top != NULL);
	struct file_stack* tmp = top->next;
	free(top->name);
	free(top);
	return tmp;
}
