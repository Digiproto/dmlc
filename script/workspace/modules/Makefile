# The file created by eJim Lee (lyjforjob@gmail.com)
MODULES := $(shell find . -maxdepth 1 -type d)
MODULES := $(basename $(patsubst ./%,%,$(MODULES)))
MODULES := $(patsubst %,%.dir,$(MODULES))
MODULES_CLEAN := $(patsubst %,%_clean,$(MODULES))

all: $(MODULES)

%.dir: %
	@cd $< && make

clean: $(MODULES_CLEAN)

%.dir_clean: %
	@cd $< && make clean
