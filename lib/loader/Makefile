CC = gcc
CPP = g++
AR = ar
prefix ?= /opt/dml_all
INCLUDES = -I. -I../common -I../include -I$(prefix)/include -I../dml_common
LIBRARY_DIRS = -L../common -L$(prefix)/lib -L../dml_common
LIBRARYS = -ldml_common -ldl -ljson -lpthread
CFLAGS = -g -Wall -Werror
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
bins := $(filter-out $(build_dir)/test.o,$(bins))
target = libjson_loader.$(suffix)

special_cppfiles = test.cpp
special_bins := $(patsubst %.cpp,$(build_dir)/%.o,$(special_cppfiles))
CFLAGS := $(CFLAGS) -DTEST_DEBUG
special_target = libtest.$(suffix)
#special_target = test

all: $(target) $(special_target)

$(special_target): $(special_bins)
	@echo "[LD] $@"
#	@$(CPP) $^ $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) \
#		-ljson_loader -L. -Wl,-rpath=. -Wl,-rpath=../dml_common -o $@
	@$(CPP) -shared $^ $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) \
		-ljson_loader -L. -Wl,-rpath=$(prefix)/lib -o $@

$(target): $(bins)
	@echo "[LD] $@"
	@$(CC) -shared $^ $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

$(build_dir)/%.o: %.cpp
	@echo "[CPP] $@"
	@$(CPP) -c      $< $(CFLAGS) $(INCLUDES) -o $@

$(build_dir)/%.o: %.c
	@echo "[CC] $@"
	@$(CC) -c      $< $(CFLAGS) $(INCLUDES) -o $@

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
	@cp libtest.$(suffix) $(prefix)/lib

distclean: clean
	rm -rf build
	rm -rf $(target) $(special_target)

clean:
	rm -rf build/*

.PHONY: clean distclean
