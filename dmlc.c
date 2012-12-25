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
	print_ast(root);
	#endif
	return root;
}

int main(int argc, char* argv[])
{
	if(argc != 2){
                fprintf(stderr,"Usage: %s source\n", argv[0]);
                exit(-1);
        }
	
        char* filename = argv[1];
        node_t* ast = get_ast(filename);
	generate_code(ast);
	return 0;
}
