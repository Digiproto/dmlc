# Written by Michael.Kang(blackfin.kang@gmail.com)

dmlc:Parser.c Lexer.c symbol.c ast.c simics_code_gen.c dmlc.c qemu_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c
	gcc -g -o dmlc Parser.c Lexer.c symbol.c ast.c dmlc.c qemu_code_gen.c simics_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c
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
