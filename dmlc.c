
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
#include "ast.h"
#include "Parser.h"
#include "Lexer.h"
#include "symbol.h"
#include "pre_parse_dml.h"
#include "code_gen.h"
#include "import.h"

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

struct file_stack* filestack_top = NULL;
symtab_t root_table = NULL;
extern void gen_qemu_code(tree_t *root, const char *out_dir);
extern void gen_skyeye_code(tree_t *root, const char *out_dir);
int main (int argc, char *argv[])
{
	/* initial extra library directory. */
	char* extra_library_dir = NULL;
	int file_num = 1;

	/* check argument. */
	if (argc != 2) {
		if(argc == 4) {
			int i;
			for(i = 1; i < argc; i++) {
				if(strcmp(argv[i], "-L") == 0)
					break;
				else
					file_num = i;
			}
			/* -L + library directory. */
			if(i < (argc - 1)) {
				if(access(argv[i + 1], F_OK)) {
					perror(argv[i + 1]);
					return -1;
				}
				extra_library_dir = argv[i + 1];
				goto normal;
			}
		}
		fprintf (stderr, "Usage: %s source [-L library]\n", argv[0]);
		return -1;
	}

normal:
	set_import_dir(argv[0], argv[file_num], extra_library_dir);

	insert_pre_dml_struct();
	/* main ast */
	tree_t* ast = get_ast (argv[file_num]);
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
