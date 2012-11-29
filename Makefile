dmlc:gdml.tab.c lex.yy.c
	gcc -o dmlc gdml.tab.c lex.yy.c
lex.yy.c:gdml.lex
	flex gdml.lex
gdml.tab.c:gdml.y
	bison -d gdml.y
clean:
	rm gdml.tab.c lex.yy.c gdml.tab.h dmlc -r -f
