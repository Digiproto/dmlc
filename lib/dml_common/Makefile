CC = gcc
PREFIX ?= /opt/dml_all
DIR_NAME = simics
INSTALL_DIR = $(PREFIX)
INCLUDES = -I.
LIBS = -ldl
#CFLAGS = -g -O2 -Werror-implicit-function-declaration -Werror=return-type \
#		-Wmissing-field-initializers -Wuninitialized
CFLAGS = -g -Wall -Werror
BUILD_DIR = build
C_FILES = $(wildcard *.c) 
HEADER= ./simics
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_FILES))
LIB_NAME = dml_common
HOST_CONFIG = simics/host-config.h

ifeq ($(shell uname),Linux)
CFLAGS := $(CFLAGS) -fPIC
suffix = so
else
suffix = dll
endif

LIB_TARGET = lib$(LIB_NAME).$(suffix)

all: gen_build_dir gen_install_info $(LIB_TARGET)
	@echo "[END] build finished."

gen_build_dir:
	@mkdir -p $(BUILD_DIR)
	@touch  $(HOST_CONFIG)

gen_install_info:
	@echo "#ifndef __HOST_INFO__" > $(HOST_CONFIG)
	@echo "#define __HOST_INFO__" >> $(HOST_CONFIG)
	@echo "#define SYSTEM_LIB \"$(INSTALL_DIR)/lib\"" >> $(HOST_CONFIG)
	@echo "#endif" >> $(HOST_CONFIG)

$(LIB_TARGET): $(OBJS)
	@echo [LD] $@
	@$(CC) -shared $^ $(CFLAGS) $(INCLUDES) -Wl,-soname,$@ -o $@

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $@ 
	@$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

ifndef ($(MAKECMDGOALS),clean)
ifndef ($(MAKECMDGOALS),distclean)
sinclude $(OBJS:.o=.d)
endif
endif

$(BUILD_DIR)/%.d: %.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) -MM $(CFLAGS) $(INCLUDES) $< 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

clean:
	rm -fr build/*
	rm -fr simics/host-config.h

distclean: clean
	rm -fr build 
	rm -rf $(LIB_TARGET)

install: all 
	@echo [install] to $(INSTALL_DIR)
	@mkdir -p $(INSTALL_DIR)
	@mkdir -p $(INSTALL_DIR)/lib
	@mkdir -p $(INSTALL_DIR)/doc
	@mkdir -p $(INSTALL_DIR)/include/simics
	@cp  -a $(LIB_TARGET) $(INSTALL_DIR)/lib/
	@cp  -r $(HEADER)/* $(INSTALL_DIR)/include/simics/

.PHONY: all clean distclean install gen_build_dir gen_install_info

