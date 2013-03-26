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

#define DEBUG_AST 1
#define QEMU 0

#if QEMU
const char* simics_dml_dir = "/opt/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0";
#else
const char* simics_dml_dir = "/opt/virtutech/simics-4.0/simics-model-builder-4.0.16/amd64-linux/bin/dml/1.0";
#endif

const char* import_file_list[] = {
	"dml-builtins.dml",
	"simics-configuration.dml",
	NULL
};

/**
* @brief generate an ast from a dml source file.
*
* @param filename dml source file name
*
* @return the root node of ast
*/
node_t* get_ast(char* filename){
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("Can't open file %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	yyscan_t scanner;
	node_t* root = NULL;
	yylex_init(&scanner);
	yyrestart(file, scanner);
	yyparse(scanner, &root);
	yylex_destroy(scanner);
	
	fclose(file);
	#if DEBUG_AST
	//print_ast(root);
	#endif
	return root;
}
char* builtin_filename = NULL;
int main(int argc, char* argv[])
{
	if(argc != 2){
                fprintf(stderr,"Usage: %s source\n", argv[0]);
                exit(-1); 
        }
	
	char tmp[256];
	snprintf(tmp,256,"%s/%s",simics_dml_dir,import_file_list[0]);
	builtin_filename = tmp;
	//printf("In %s, builtin_filename=%s\n", __FUNCTION__, builtin_filename);
	//sleep(1);
	/* dml-builtins.dml */
	//node_t* import_ast = get_ast(builtin_filename);

	/* main ast */
        char* filename = argv[1];
        node_t* ast = get_ast(filename);
	assert(ast != NULL);
	print_ast(ast);
#if QEMU
	gen_qemu_code(ast, "./output/");
#else
	generate_simics_code(ast, "./output/");
#endif

	return 0;
}
