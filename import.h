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

/* store library directories and set priority. */
const char **import_dir_list;
const char *gdml_library_dir;

#ifndef DIR_MAX_LEN
#define DIR_MAX_LEN 1024
#endif

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
/* Import.h have higher priority in gdml.y. */
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
extern int link_dir_filename(char* buf, size_t n, const char* dir, const char* filename);
extern void set_import_dir(const char* execname, const char* filename, char** extradirs);
extern char** list_add_dir(char** dir_list, const char* dir);
extern int list_get_len(char** dir_list);

#endif /* __IMPORT_H__ */
