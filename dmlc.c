
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
* @file dmlc.c
* @brief main function for dmlc
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-12-21
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <libgen.h>
#include "ast.h"
#include "Parser.h"
#include "Lexer.h"
#include "symbol.h"
#include "pre_parse_dml.h"
#include "code_gen.h"
#include "dmlc.h"

#define QEMU 0

#if 0  /* libray directory has been moved to dmlc.h */
#if QEMU
const char *simics_dml_dir =
	"/opt/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0";
#else
const char *simics_dml_dir =
	"/opt/virtutech/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0";
#endif
#endif

const char *import_file_list[] = {
	"simics-auto-api-4_0.dml",
	"dml-builtins.dml",
	"simics/C.dml",
	NULL
};

extern int yyparse (yyscan_t scanner, tree_t** root_ptr);

/**
* @brief generate an ast from a dml source file.
*
* @param filename dml source file name
*
* @return the root node of ast
*/
tree_t* get_ast (const char *filename)
{
	FILE *file = fopen (filename, "r");
	if (file == NULL) {
		fprintf (stderr, "Can't open file %s.\n", filename);
		exit (EXIT_FAILURE);
	}

	yyscan_t scanner;
	tree_t* root = NULL;
	yylex_init (&scanner);
	yyrestart (file, scanner);
	struct file_stack* tmp_stack;
	tmp_stack = push_file_stack(filestack_top, filename);  // <<<<< push new file name
	if(tmp_stack != NULL) {
		filestack_top = tmp_stack;
		yyparse (scanner, &root);
		filestack_top = pop_file_stack(filestack_top);             // <<<<< pop top file name
		yylex_destroy (scanner);
	}

	fclose (file);
	//print_ast(root);
	return root;
}

void set_library_dir(const char *dir_r)
{
	int len;
	char *tmp = malloc(DIR_MAX_LEN);

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
	*(p + 1) = '\0';
#endif

	/* set library by linking bin path */
	assert(len + strlen(GDML_LIBRARY_DIR) < DIR_MAX_LEN);
	gdml_library_dir = strcat(tmp, GDML_LIBRARY_DIR);
}

char* get_file_dir(char* filename) {
	assert(filename != NULL);

	char tmp[256];
	strncpy(tmp, filename, strlen(filename));
	char* dir = dirname(tmp);

	return strdup(dir);
}

struct file_stack* filestack_top = NULL;
char *builtin_filename = NULL;
char* file_dir = NULL;
symtab_t root_table = NULL;
extern void gen_qemu_code(tree_t *root, const char *out_dir);
extern void gen_skyeye_code(tree_t *root, const char *out_dir);
int main (int argc, char *argv[])
{
	if (argc != 2) {
		fprintf (stderr, "Usage: %s source\n", argv[0]);
		exit (-1);
	}

	set_library_dir(argv[0]);

	int i = 0;
	int len = strlen(gdml_library_dir);
	char* file_path = NULL;
	/* add the full patch about import file */
	while (import_file_list[i] != NULL) {
		len += strlen(import_file_list[i]);
		file_path = (char*)gdml_zmalloc(len + 1);
		strcpy(file_path, gdml_library_dir);
		strcat(file_path, import_file_list[i]);
		import_file_list[i] = file_path;
		i++;
	}

	insert_pre_dml_struct();
	/* main ast */
	char *filename = argv[1];
	file_dir = get_file_dir(filename);
	tree_t* ast = get_ast (filename);
	assert (ast != NULL);
	if (root_table->type != DEVICE_TYPE) {
		fprintf(stderr, "a module should have device\n");
		exit(-1);
	}
	else {
		parse_undef_node(root_table);
	}
#ifndef RELEASE
	printf("node name: %s\n", ast->common.name);
	print_ast (ast);
	printf("Print the syntax tree ok!\n");
#endif
#if  1 || QEMU
	gen_code (ast, "./output/");
#else
	//generate_simics_code (ast, "./output/");
#endif

	return 0;
}
