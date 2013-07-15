# Written by Michael.Kang(blackfin.kang@gmail.com)

CFLAGS= -g -O2 -Werror-implicit-function-declaration -Werror=return-type -Wmissing-field-initializers -Wuninitialized
//CFLAGS=
dmlc:Parser.c Lexer.c symbol.c ast.c simics_code_gen.c dmlc.c qemu_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c pre_parse_dml.c object.c
	gcc $(CFLAGS) -g -o dmlc Parser.c Lexer.c symbol.c ast.c dmlc.c qemu_code_gen.c simics_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c pre_parse_dml.c object.c
Lexer.c:gdml.lex
	flex gdml.lex
Parser.c:gdml.y
	bison -d gdml.y
module_test: symbol

symbol: symbol.c
	gcc -g -DSYMBOL_DEBUG -o symbol symbol.c
clean:
	rm Parser.c Lexer.c Lexer.h Parser.h gdml.tab.h dmlc tags -r -f
	rm symbol -rf
