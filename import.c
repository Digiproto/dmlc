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
#include <unistd.h>
#include <libgen.h>
#include "import.h"
#include "ast.h"
#include "info_output.h"

#ifndef GDML_LIBRARY_DIR
#ifdef _WIN32
#define GDML_LIBRARY_DIR "..\\include\\gdml\\1.0"
#else
#define GDML_LIBRARY_DIR "../include/gdml/1.0"
#endif
#endif

const char *import_file_list[] = {
	"simics-auto-api-4_0.dml",
	"dml-builtins.dml",
	//"pci-to-pci.dml",
	"simics/C.dml",
	NULL
};

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

int link_dir_filename(char* buf, size_t n, const char* dir, const char* filename)
{
	int rt;
#ifdef _WIN32
	rt = snprintf(buf, n, "%s\\%s", dir, filename);
#else
	rt = snprintf(buf, n, "%s/%s", dir, filename);
#endif
	if(rt >= n) {
		warning("dir \"%s\" and file name \"%s\" is too long.", dir, filename);
		return -1;
	}
	return 0;
}

static const char* get_library_dir(const char *dir_r)
{
	int len;
	char tmp[DIR_MAX_LEN];

	/* if exe is on windows, dir is like "d:\mingw\msys\1.0\usr\bin\dmlc" */
#ifdef _WIN32
	/* exe is on windows */
	char *p = strrchr(dir_r, '\\');
	assert(p);
	len = p - dir_r + 1;
	memcpy(tmp, dir_r, len);
	tmp[len] = '\0';
#else
	/* exe is on linux */
	size_t rt = readlink("/proc/self/exe", tmp, DIR_MAX_LEN);
	if(rt < 0) {
		perror("readlink");
		exit(-1);
	}
	char *p = strrchr(tmp, '/');
	assert(p);
	len = p - tmp + 1;
	tmp[len] = '\0';
#endif

	/* set library by linking bin path */
	assert(len + strlen(GDML_LIBRARY_DIR) < DIR_MAX_LEN);
	char* tp = strcat(tmp, GDML_LIBRARY_DIR);
	assert(tp != NULL);
	gdml_library_dir =  strdup(tp);
	return gdml_library_dir;
}

static const char* get_file_dir(const char* filename) {
	assert(filename != NULL);

	char tmp[DIR_MAX_LEN];
	int n = strlen(filename);
	if(n >= DIR_MAX_LEN) {
		error("filename is to long. (>= %d)", DIR_MAX_LEN);
	}
	strncpy(tmp, filename, n + 1);
	char* dir = dirname(tmp);

	return strdup(dir);
}

void set_import_dir(const char* execname, const char* fullname, const char* extradir)
{
	const char *library_dir = get_library_dir(execname);
	const char *extra_library_dir = extradir;
	const char *file_dir = get_file_dir(fullname);
	int i;

	if(extradir) {
		/* format extra library directory. */
		char* tmp_str = strdup(extradir);
		i = strlen(extradir);
		do{
			i--;
			if(extradir[i] != '\\' && extradir[i] != '/')
				break;
		}while(i >= 0);
		if(i < 0) {
			error("extra library directory %s is wrong.", extradir);
		}
		tmp_str[i + 1] = '\0';
		extra_library_dir = tmp_str;
	}

	/* NULL is end. */
	if(extra_library_dir) {
		import_dir_list = gdml_zmalloc((3 + 1) * sizeof(char*));
		import_dir_list[0] = file_dir;
		import_dir_list[1] = extra_library_dir;
		import_dir_list[2] = library_dir;
		import_dir_list[3] = NULL;
	}else{
		import_dir_list = gdml_zmalloc((2 + 1) * sizeof(char*));
		import_dir_list[0] = file_dir;
		import_dir_list[1] = library_dir;
		import_dir_list[2] = NULL;
	}

	int dirlen = strlen(gdml_library_dir), len;
	/* add the full patch about import file */
	for(i = 0; import_file_list[i] != NULL; i++) {
		len = dirlen + strlen(import_file_list[i]);
		char* file_path = (char*)gdml_zmalloc(len + 2);
		int rt = link_dir_filename(file_path, len + 2, gdml_library_dir, import_file_list[i]);
		assert(rt == 0);
		import_file_list[i] = file_path;
	}
}
