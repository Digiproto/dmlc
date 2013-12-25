CC = gcc
AR = ar
prefix ?= /opt/dml_all
INCLUDES = -I$(prefix)/include
LIBRARY_DIRS = -L$(prefix)/lib
LIBRARYS = -ldml_common
CFLAGS = -g -Wall -Werror
build_dir = build

ifeq ($(shell uname),Linux)
CFLAGS := $(CFLAGS) -fPIC
suffix = so
else
suffix = dll
endif

cfiles = $(filter-out %_protos.c,$(wildcard *.c))
bins = $(patsubst %.c,$(build_dir)/%.o,$(cfiles))
targets = $(patsubst %.c,lib%.$(suffix),$(cfiles))

special_cfiles =
special_bins = $(patsubst %.c,$(build_dir)/%.o,$(special_cfiles))
special_targets = $(patsubst %.c,lib%.a,$(special_cfiles))

all: $(special_targets) $(targets)
	@echo "[END] finish compiling all proxy"

lib%.a: $(build_dir)/%.o
	@echo "[AR] $@"
	@$(AR) -rcs $@ $^

lib%.$(suffix): $(build_dir)/%.o
	@echo "[LD] $@"
	@$(CC) -shared $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

$(build_dir)/%.o: %.c
	@echo "[CC] $@"
	@$(CC) -c      $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
sinclude $(bins:.o=.d)
sinclude $(special_bins:.o=.d)
endif
endif

$(build_dir)/%.d: %.c
	@mkdir -p $(build_dir)
	@$(CC) -MM $^ $(CFLAGS) $(INCLUDES) $(LIBRARYS) 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

distclean:
	rm -rf build
	rm -rf $(targets)
	rm -rf lib*.$(suffix) lib*.a
	rm -rf *.c *.cpp *.h

clean:
	rm -rf build/*

install:

.PHONY: clean distclean install
