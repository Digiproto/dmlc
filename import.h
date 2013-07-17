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

#if 0
/* Source file current line is coming from.  */
extern char *input_filename;

/* Top-level source file.  */
extern char *main_input_filename;

/* Line number in current source file.  */
extern int lineno;

/* Stream for reading from input file.  */
extern FILE *finput;
#endif

struct file_stack
{
	char *name;
	struct file_stack *next;
//	int line;
//	int indent_level;
};

extern struct file_stack* push_file_stack(struct file_stack* top, const char* name);
extern struct file_stack* pop_file_stack(struct file_stack* top);

#if 0
/* Stack of currently pending input files.
   The line member is not accurate for the innermost file on the stack.  */
extern struct file_stack *input_file_stack;
#endif


#endif /* __IMPORT_H__ */
