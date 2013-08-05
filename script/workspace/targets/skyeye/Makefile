# The file is created by eJim Lee (lyjforjob@gmail.com)
MODULES := $(shell find . -maxdepth 1 -type d)
MODULES := $(basename $(patsubst ./%,%,$(MODULES)))
MODULES := $(patsubst %,%.dir,$(MODULES))
MODULES_CLEAN := $(patsubst %,%_clean,$(MODULES))
MODULES_DISTCLEAN := $(patsubst %,%_clean,$(MODULES))

all: $(MODULES)

%.dir: %
	@cd $< && make

clean: $(MODULES_CLEAN)

distclean: clean

%.dir_clean: %
	@cd $< && make clean

%.dir_distclean: %
	@cd $< && make distclean

.PHONY: clean distclean
