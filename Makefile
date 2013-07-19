# Written by Michael.Kang(blackfin.kang@gmail.com)

CFLAGS= -g -O2 -Werror-implicit-function-declaration -Werror=return-type -Wmissing-field-initializers -Wuninitialized
//CFLAGS=
SRC = ./
PLATFORM ?= qemu
p = $(shell echo $(PLATFORM))
INCLUDE = -I. -I./backend/
ifeq ($(LANG),)
# on windows
LIBRARY_DIR = -L/include
LIBRARY = -lregex
else
# on linux
LIBRARY_DIR = 
LIBRARY = 
endif
C_FILES := Parser.c Lexer.c symbol.c ast.c  dmlc.c  debug_color.c stack.c expression.c decl.c parse_str.c pre_parse_dml.c import.c
gen_objs := object.c ref.c $(wildcard gen_*.c) 
backend = $(wildcard ./backend/*.c) $(wildcard ./backend/$(p)/*.c)

dmlc: $(C_FILES) $(gen_objs) $(backend)
	@echo ===== start build =====
	gcc $(CFLAGS) $^ -o $@ $(INCLUDE) $(LIBRARY_DIR) $(LIBRARY)
#	gcc $(CFLAGS) -g -o dml  Parser.c Lexer.c symbol.c ast.c  dmlc.c qemu_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c \
#		object.c qemu_platform.c gen_common.c ref.c qemu_object_headfile.c  gen_once_noplatform.c gen_object.c gen_utility.c gen_expression.c \
#		gen_method_protos.c pre_parse_dml.c import.c

#c_files: Parser.c Lexer.c symbol.c ast.c  dmlc.c qemu_code_gen.c debug_color.c stack.c expression.c decl.c parse_str.c object.c \
#		 gen_common.c qemu_platform.c ref.c qemu_object_headfile.c gen_once_noplatform.c gen_object.c gen_utility.c pre_parse_dml.c import.c
Lexer.c: gdml.lex
	flex gdml.lex
Parser.c: gdml.y
	bison -d gdml.y
#module_test: symbol
#
#symbol: symbol.c
#	gcc -g -DSYMBOL_DEBUG -o symbol symbol.c
install: dmlc include/gdml
	@echo ===== start install =====
	@echo install dmlc [to /usr/bin]
	@cp -r dmlc     /usr/bin
	@echo install include/gdml [to /usr/include]
	@cp -r include/gdml /usr/include
uninstall: /usr/bin/dmlc /usr/include/gdml
	@echo ===== start uninstall =====
	@echo uninstall dmlc [from /usr/bin]
	@rm -rf /usr/bin/dmlc
	@echo uninstall lib/gdml [from /usr/include]
	@rm -rf /usr/include/gdml
check: dmlc ../include/gdml output
	@echo ===== start check =====
	./dmlc testcase/simple-device.dml
../include/gdml:
	cp -r include ../
output:
	mkdir output
clean:
	@echo ===== start clean =====
	-rm Parser.c Lexer.c Lexer.h Parser.h gdml.tab.h dmlc tags -r -f
	-rm symbol -rf
	-rm output/* -rf
.PHONY: clean check install uninstall
