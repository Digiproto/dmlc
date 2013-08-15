# The file is created by eJim Lee (lyjforjob@gmail.com)
MODULES := $(shell find . -maxdepth 1 -type d)
MODULES := $(basename $(patsubst ./%,%,$(MODULES)))
MODULES_CLEAN := $(patsubst %,%_dev_clean,$(MODULES))
MODULES_DISTCLEAN := $(patsubst %,%_dev_distclean,$(MODULES))

all: $(MODULES)

define PROGRAM_TEMPLATE
$(1): $(1)_dev
$(1)_dev:
	@cd $(1) && make
$(1)_dev_clean:
	@cd $(1) && make clean
$(1)_dev_distclean:
	@cd $(1) && make distclean
endef

$(foreach var,$(MODULES),$(eval $(call PROGRAM_TEMPLATE,$(var))))

clean: $(MODULES_CLEAN)

distclean: $(MODULES_DISTCLEAN)

.PHONY: all clean distclean
