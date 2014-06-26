# The file is created by eJim Lee (lyjforjob@gmail.com)

$(shell sed -e 's/\$$\(\w\+\)/\$$(\1)/g' -e 's/["]//g' ../config > ../config.tmp)
sinclude ../config.tmp
$(shell rm -rf ../config.tmp)

#SYSTEMC_DIR_IN := ../targets/systemc
QEMU_DIR_IN := $(QEMU_DIR)/hw/simics
HOST := $(shell uname)

# find all modules
MODULES := $(shell find . -maxdepth 1 -type d)
MODULES := $(basename $(patsubst ./%,%,$(MODULES)))
# create clean rule
MODULES_CLEAN := $(patsubst %,%_dev_clean,$(MODULES))
MODULES_DISTCLEAN := $(patsubst %,%_dev_distclean,$(MODULES))
# create test rule
MODULES_TEST := $(patsubst %,%_test,$(MODULES))

# modify target list
# remove reserve keywords
SPECIAL_WORDS := clean distclean all test
TARGETS := $(filter-out $(SPECIAL_WORDS),$(MAKECMDGOALS))

# systemc or qemu target goals
#ifneq ($(SYSTEMC_BUILD),)
#ALL_SYSTEMC_DEV := $(patsubst %,%_systemc_dev,$(MODULES))
#TARGETS_SYSTEMC_DEV := $(patsubst %,%_systemc_dev,$(TARGETS))
#ifeq ($(HOST),Linux)
#ALL_SYSTEMC_OBJ := $(foreach var,$(MODULES),$(SYSTEMC_DIR_IN)/$(var)/lib$(var).so)
#TARGETS_SYSTEMC_OBJ := $(foreach var,$(TARGETS),$(SYSTEMC_DIR_IN)/$(var)/lib$(var).so)
#else
#ALL_SYSTEMC_OBJ := $(foreach var,$(MODULES),$(SYSTEMC_DIR_IN)/$(var)/lib$(var).dll)
#TARGETS_SYSTEMC_OBJ := $(foreach var,$(TARGETS),$(SYSTEMC_DIR_IN)/$(var)/lib$(var).dll)
#endif
#endif # SYSTEMC_BUILD
ifneq ($(QEMU_BUILD),)
ALL_QEMU_OBJ := $(foreach var,$(MODULES),$(QEMU_DIR_IN)/$(var)/$(var).o)
TARGETS_QEMU_OBJ := $(foreach var,$(TARGETS),$(QEMU_DIR_IN)/$(var)/$(var).o)
ALL_QEMU_DEV := $(patsubst %,%_qemu_dev,$(MODULES))
TARGETS_QEMU_DEV := $(patsubst %,%_qemu_dev,$(TARGETS))
endif # QEMU_BUILD

# all target goals
#ALL_OBJ := $(ALL_SYSTEMC_OBJ) $(ALL_QEMU_OBJ)
#TARGETS_OBJ := $(TARGETS_SYSTEMC_OBJ) $(TARGETS_QEMU_OBJ)
#ALL_DEV := $(ALL_SYSTEMC_DEV) $(ALL_QEMU_DEV)
#TARGETS_DEV := $(TARGETS_SYSTEMC_DEV) $(TARGETS_QEMU_DEV)
ALL_OBJ := $(ALL_QEMU_OBJ)
TARGETS_OBJ := $(TARGETS_QEMU_OBJ)
ALL_DEV := $(ALL_QEMU_DEV)
TARGETS_DEV := $(TARGETS_QEMU_DEV)

# configure and make QEMU after generating all C codes
# GOAL is all targets needed.
ifneq ($(MAKECMDGOALS),)
ifneq ($(MAKECMDGOALS),all)
GOAL_OBJ := $(TARGETS_OBJ)
GOAL_DEV := $(TARGETS_DEV)
GOAL := $(TARGETS)
else
GOAL_OBJ := $(ALL_OBJ)
GOAL_DEV := $(ALL_DEV)
GOAL := $(MODULES)
endif
else
GOAL_OBJ := $(ALL_OBJ)
GOAL_DEV := $(ALL_DEV)
GOAL := $(MODULES)
endif

# only for make or make all
all: $(ALL_OBJ)
	@echo [normal] finish compile QEMU and SystemC device.

# for make {device_name}
$(TARGETS): $(TARGETS_OBJ)
	@echo [normal] finish compile QEMU and SystemC device.

# create each module rule
define PROGRAM_TEMPLATE

#ifneq ($(SYSTEMC_BUILD),)
#$(SYSTEMC_DIR_IN)/$(1)/lib$(1).so $(SYSTEMC_DIR_IN)/$(1)/lib$(1).dll: $(GOAL_DEV)
#	@cd ../targets/systemc && $(MAKE) $(1)
#$(1)_systemc_dev:
#	@cd $(1) && make systemc
#endif # SYSTEMC_BUILD

ifneq ($(QEMU_BUILD),)
$(QEMU_DIR_IN)/$(1)/$(1).o: $(GOAL_DEV)
	@./setMakefile.qemu $(GOAL)
	@echo [normal] configure QEMU
	@cd $(QEMU_DIR) && ./configure $(QEMU_CONF_FLAGS) 1>/dev/null
	@echo [normal] make QEMU
	@cd $(QEMU_DIR) && $(MAKE) 1>/dev/null
$(1)_qemu_dev:
	@cd $(1) && make qemu
endif # QEMU_BUILD

$(1)_test:
	@cd $(1) && make test
$(1)_dev_clean:
	@cd $(1) && make clean
$(1)_dev_distclean:
	@cd $(1) && make distclean
endef

# create rule for all module
$(foreach var,$(MODULES),$(eval $(call PROGRAM_TEMPLATE,$(var))))

# make test
test: $(MODULES_TEST)

clean: $(MODULES_CLEAN)

distclean: $(MODULES_DISTCLEAN)

.PHONY: all clean distclean test $(MODULES)
