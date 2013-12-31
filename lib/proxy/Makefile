CC = gcc
AR = ar
prefix ?= /opt/dml_all
INCLUDES = -I../common -I../include -I../dml_common -I$(prefix)/include
LIBRARY_DIRS = -L../common -L../dml_common -L$(prefix)/lib -L../loader
LIBRARYS = -ldml_common -lthread_tmp -lschedule_tmp -lpthread -ljson_loader
CFLAGS = -g -Wall -Werror
build_dir = build

ifeq ($(shell uname),Linux)
CFLAGS := $(CFLAGS) -fPIC
suffix = so
else
LIBRARYS := $(LIBRARYS) -lwsock32
suffix = dll
endif

cfiles = $(wildcard *.c)
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
	@$(CC) -c      $< $(CFLAGS) $(INCLUDES) -o $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
sinclude $(bins:.o=.d)
sinclude $(special_bins:.o=.d)
endif
endif

$(build_dir)/%.d: %.c
	@mkdir -p $(build_dir)
	@$(CC) -MM $^ $(CFLAGS) $(INCLUDES) $(LIBRARYS) 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

install: $(targets)
	@echo "[INS] to $(prefix)/lib"
	@mkdir -p $(prefix)/lib
	@cp -r *.$(suffix) $(prefix)/lib

distclean: clean
	rm -rf build
	rm -rf $(targets)
	rm -rf lib*.so lib*.a

clean:
	rm -rf build/*

.PHONY: clean distclean install
