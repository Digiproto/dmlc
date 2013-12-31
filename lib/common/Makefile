CC = gcc
AR = ar
INCLUDES =
LIBRARY_DIRS = -L.
LIBRARYS = -lthread_tmp -lpthread
CFLAGS = -g -Wall -Werror
build_dir = build

ifeq ($(shell uname),Linux)
CFLAGS := $(CFLAGS) -fPIC
else
endif

cfiles = $(wildcard *.c)
#cfiles = thread_tmp.c
#cfiles = schedule_tmp.c
bins = $(patsubst %.c,$(build_dir)/%.o,$(cfiles))
targets = $(patsubst %.c,lib%.a,$(cfiles))

#special_cfiles = schedule_tmp.c
special_cfiles =
special_bins = $(patsubst %.c,$(build_dir)/%.o,$(special_cfiles))
special_targets = $(patsubst %.c,%,$(special_cfiles))

all: $(targets) $(special_targets)
	@echo "[END] finish compiling all common library"

libschedule_tmp.a: $(build_dir)/schedule_tmp.o $(build_dir)/thread_tmp.o
	@echo "[ARx] $@"
	@$(AR) -rcs $@ $^

lib%.a: $(build_dir)/%.o
	@echo "[AR] $@"
	@$(AR) -rcs $@ $^

lib%.so: $(build_dir)/%.o
	@echo "[LD] $@"
	@$(CC) -shared $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

$(build_dir)/%.o: %.c
	@echo "[CC] $@"
	@$(CC) -c      $< $(CFLAGS) $(INCLUDES) -o $@

%: $(build_dir)/%.o
	@echo "[LD] $@"
	@$(CC)         $< $(CFLAGS) $(INCLUDES) $(LIBRARY_DIRS) $(LIBRARYS) -o $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
sinclude $(bins:.o=.d)
sinclude $(special_bins:.o=.d)
endif
endif

$(build_dir)/%.d: %.c
	@mkdir -p $(build_dir)
	@$(CC) -MM $^ $(CFLAGS) $(INCLUDES) $(LIBRARYS) 2>/dev/null | sed -e 's#.*\.o#$(patsubst %.d,%.o,$@)#' > $@

distclean: clean
	rm -rf build
	rm -rf $(targets)
	rm -rf $(special_targets)
	rm -rf lib*.so lib*.a

clean:
	rm -rf build/*

install:

.PHONY: clean distclean install
