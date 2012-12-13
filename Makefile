dmlc:gdml.tab.c lex.yy.c symbol.c ast.c
	gcc -g -o dmlc gdml.tab.c lex.yy.c symbol.c ast.c
lex.yy.c:gdml.lex
	flex gdml.lex
gdml.tab.c:gdml.y
	bison -d gdml.y
clean:
	rm gdml.tab.c lex.yy.c gdml.tab.h dmlc tags -r -f
