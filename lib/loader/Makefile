CC = gcc
CPP = g++
AR = ar
prefix ?= /opt/dml_all
INCLUDES = -I. -I../common -I../include -I$(prefix)/include -I../dml_common
LIBRARY_DIRS = -L../common -L$(prefix)/lib -L../dml_common
LIBRARYS = -ldml_common -ldl -ljson -lpthread
CFLAGS = -g
build_dir = build

ifeq ($(shell uname),Linux)
CFLAGS := $(CFLAGS) -fPIC
suffix = so
else
suffix = dll
endif

cfiles = $(wildcard *.c) $(wildcard bus/*.c)
cppfiles = $(wildcard *.cpp) $(wildcard bus/*.cpp)
bins := $(patsubst %.c,$(build_dir)/%.o,$(cfiles))
bins := $(patsubst %.cpp,$(build_dir)/%.o,$(cppfiles)) $(bins)
target = libjson_loader.$(suffix)

$(target): $(bins)
	@echo "[LD] $@"
	@$(CC) -shared $^ $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

$(build_dir)/%.o: %.cpp
	@echo "[CPP] $@"
	@$(CPP) -c      $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) -o $@

$(build_dir)/%.o: %.c
	@echo "[CC] $@"
	@$(CC) -c      $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
sinclude $(bins:.o=.d)
endif
endif

$(build_dir)/%.d: %.cpp
	@mkdir -p `dirname $@`
	@$(CPP) -MM $^ $(CFLAGS) $(INCLUDES) $(LIBRARYS) 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

$(build_dir)/%.d: %.c
	@mkdir -p `dirname $@`
	@$(CC) -MM $^ $(CFLAGS) $(INCLUDES) $(LIBRARYS) 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

install:
	@echo "[INS] to $(prefix)/lib"
	@mkdir -p $(prefix)/lib
	@cp libjson_loader.$(suffix) $(prefix)/lib

distclean: clean
	rm -rf build
	rm -rf $(targets)
	rm -rf lib*.$(suffix)

clean:
	rm -rf build/*

.PHONY: clean distclean
