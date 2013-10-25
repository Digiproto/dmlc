# The file created by eJim Lee (lyjforjob@gmail.com)

MODULES := $(shell find modules -maxdepth 1 -type d)
MODULES := $(basename $(patsubst modules/%,%,$(MODULES)))

all: modules
	@cd $^ && make $(MAKECMDGOALS_MODE)

$(MAKECMDGOALS): modules
	@cd $^ && make $(MAKECMDGOALS)

.PHONY: distclean clean test $(MODULES)
