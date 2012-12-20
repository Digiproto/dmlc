dmlc:Parser.c Lexer.c symbol.c ast.c
	gcc -g -o dmlc Parser.c Lexer.c symbol.c ast.c
Lexer.c:gdml.lex
	flex gdml.lex
Parser.c:gdml.y
	bison -d gdml.y
clean:
	rm Parser.c Lexer.c Lexer.h Parser.h gdml.tab.h dmlc tags -r -f
