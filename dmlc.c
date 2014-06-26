
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
	/* init flex */
	yylex_init (&scanner);
	/* call flex for lexical analysis */
	yyrestart (file, scanner);
	struct file_stack* tmp_stack;
	/* push new file into stack */
	tmp_stack = push_file_stack(filestack_top, filename);
	if(tmp_stack != NULL) {
		filestack_top = tmp_stack;
		/* call bision for parsing*/
		yyparse (scanner, &root);
		/* pop top file name */
		filestack_top = pop_file_stack(filestack_top);
		yylex_destroy (scanner);
	}

	fclose (file);
	return root;
}

char* device_file_name = NULL;

/**
* @brief change the '-' to '_' in device file name.
*
* @param the device file name
*
* @return the device file name about changed
*/
static char* change_string_char(char device_file[]) {
	assert(device_file != NULL);
	char* pos = NULL;
	do {
		pos = strchr(device_file, '-');
		if (pos)
			*pos = '_';
	}while(pos != NULL);

	return device_file;
}

/**
* @brief get the device file name for generate.
*
* @param filename about device simics
*
* @return the name about device file
*/
static char* get_device_file_name(const char* filename) {
	assert(filename != NULL);
	const char* file = strrchr(filename, '/');
	file = (file == NULL) ? filename : (file + 1);
	const char* dml = strstr(file, ".dml");
	int len = (unsigned)(dml) - (unsigned)(file);
	char* device_file = gdml_zmalloc(len + 1);
	strncpy(device_file, file, len);
	device_file[len] = '\0';
	device_file = change_string_char(device_file);
	return device_file;
}

struct file_stack* filestack_top = NULL;
symtab_t root_table = NULL;
extern void gen_qemu_code(tree_t *root, const char *out_dir);
extern void gen_skyeye_code(tree_t *root, const char *out_dir);
int main (int argc, char *argv[])
{
	/* initial extra library directory. */
	char** extra_library_dirs = NULL;
	int file_num = 0;

	/* check argument. */
	int i;
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			/* options with '-' on top */
			if(strcmp(argv[i], "-I") == 0) {
				/* -I path set header file path */
				if(i + 1 < argc) {
					if(access(argv[i + 1], F_OK)) {
						perror(argv[i + 1]);
						fprintf (stderr, "Usage: %s source [-I library_path]\n", argv[0]);
						return -1;
					}
					extra_library_dirs = list_add_dir(extra_library_dirs, argv[i + 1]);
					i++;
				}else{
					fprintf (stderr, "[error] lack path after \"-I\"\n");
					fprintf (stderr, "Usage: %s source [-I library_path]\n", argv[0]);
					return -1;
				}
			}else if(argv[i][1] == 'I') {
				/* -Ipath set header file path */
				if(access(argv[i] + 2, F_OK)) {
					perror(argv[i] + 2);
					fprintf (stderr, "Usage: %s source [-I library_path]\n", argv[0]);
					return -1;
				}
				extra_library_dirs = list_add_dir(extra_library_dirs, argv[i] + 2);
			}else{
				/* unknown options */
				fprintf (stderr, "[error] unknown option \"%s\"\n", argv[i]);
				fprintf (stderr, "Usage: %s source [-I library_path]\n", argv[0]);
				return -1;
			}
		}else{
			if(file_num != 0) {
				fprintf (stderr, "[error] only support a file\n");
				fprintf (stderr, "Usage: %s source [-I library_path]\n", argv[0]);
				return -1;
			}
			file_num = i;
		}
	}
	if(file_num == 0) {
		fprintf (stderr, "Usage: %s source [-I library]\n", argv[0]);
		return -1;
	}
	set_import_dir(argv[0], argv[file_num], extra_library_dirs);

	//insert_pre_dml_struct();
	/* main ast */
	device_file_name = get_device_file_name(strdup(argv[file_num]));
	tree_t* ast = get_ast (argv[file_num]);
	assert (ast != NULL);
	if (root_table->type != DEVICE_TYPE) {
		fprintf(stderr, "a module should have device\n");
		exit(-1);
	}
//#define PRINT_AST 1
#ifdef PRINT_AST
	print_ast (ast);
	printf("Print the syntax tree ok!\n");
#endif
	gen_code (ast, "./output/");
	printf("generate code ok!\n");

	return 0;
}
