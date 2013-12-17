CC = gcc
PREFIX ?= /usr
DIR_NAME = simics
INSTALL_DIR = $(PREFIX)
INCLUDES = -I. -ldl
#CFLAGS = -g -O2 -Werror-implicit-function-declaration -Werror=return-type \
#		-Wmissing-field-initializers -Wuninitialized
CFLAGS = -g
BUILD_DIR = build
C_FILES = $(wildcard *.c) 
HEADER= ./simics
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_FILES))
LIB_NAME = dml_common
LIB_TARGET = lib$(LIB_NAME).so
HOST_CONFIG = simics/host-config.h

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
	@$(CC) -shared $(CFLAGS) $(INCLUDES) -Wl,-soname,$@ -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $@ 
	@$(CC) -fPIC $(CFLAGS) $(INCLUDES) -o $@ -c $<

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

