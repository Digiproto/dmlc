/*
 * import.h:
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

#ifndef __IMPORT_H__
#define __IMPORT_H__  1

struct file_stack
{
	char *name;
	struct file_stack *next;
	/* record files imported.
	 * it's not a global variable, because considered to reentrant. */
	struct file_stack *file_history;
};

/* Bison location imformation structure. */
/* Moved from gdml.y, because tree.h need it. */
typedef struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	struct file_stack* file;
} YYLTYPE;

extern struct file_stack* push_file_stack(struct file_stack* top, const char* name);
extern struct file_stack* pop_file_stack(struct file_stack* top);

#endif /* __IMPORT_H__ */
