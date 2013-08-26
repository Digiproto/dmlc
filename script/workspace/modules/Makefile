# The file is created by eJim Lee (lyjforjob@gmail.com)

$(shell sed -e 's/\$$\(\w\+\)/\$$(\1)/g' -e 's/["]//g' ../config > ../config.tmp)
sinclude ../config.tmp
$(shell rm -rf ../config.tmp)

SKYEYE_DIR_IN := ../targets/skyeye
QEMU_DIR_IN := $(QEMU_DIR)/hw/simics
HOST := $(shell uname)

# find all modules
MODULES := $(shell find . -maxdepth 1 -type d)
MODULES := $(basename $(patsubst ./%,%,$(MODULES)))
# create clean rule
MODULES_CLEAN := $(patsubst %,%_dev_clean,$(MODULES))
MODULES_DISTCLEAN := $(patsubst %,%_dev_distclean,$(MODULES))

# modify target list
# remove reserve keywords
SPECIAL_WORDS := clean distclean all
TARGETS := $(filter-out $(SPECIAL_WORDS),$(MAKECMDGOALS))

# skyeye or qemu target goals
ifneq ($(SKYEYE_BUILD),)
ALL_SKYEYE_DEV := $(patsubst %,%_skyeye_dev,$(MODULES))
TARGETS_SKYEYE_DEV := $(patsubst %,%_skyeye_dev,$(TARGETS))
ifeq ($(HOST),Linux)
ALL_SKYEYE_OBJ := $(foreach var,$(MODULES),$(SKYEYE_DIR_IN)/$(var)/lib$(var).so)
TARGETS_SKYEYE_OBJ := $(foreach var,$(TARGETS),$(SKYEYE_DIR_IN)/$(var)/lib$(var).so)
else
ALL_SKYEYE_OBJ := $(foreach var,$(MODULES),$(SKYEYE_DIR_IN)/$(var)/lib$(var).dll)
TARGETS_SKYEYE_OBJ := $(foreach var,$(TARGETS),$(SKYEYE_DIR_IN)/$(var)/lib$(var).dll)
endif
endif # SKYEYE_BUILD
ifneq ($(QEMU_BUILD),)
ALL_QEMU_OBJ := $(foreach var,$(MODULES),$(QEMU_DIR_IN)/$(var)/$(var).o)
TARGETS_QEMU_OBJ := $(foreach var,$(TARGETS),$(QEMU_DIR_IN)/$(var)/$(var).o)
ALL_QEMU_DEV := $(patsubst %,%_qemu_dev,$(MODULES))
TARGETS_QEMU_DEV := $(patsubst %,%_qemu_dev,$(TARGETS))
endif # QEMU_BUILD

# all target goals
ALL_OBJ := $(ALL_SKYEYE_OBJ) $(ALL_QEMU_OBJ)
TARGETS_OBJ := $(TARGETS_SKYEYE_OBJ) $(TARGETS_QEMU_OBJ)
ALL_DEV := $(ALL_SKYEYE_DEV) $(ALL_QEMU_DEV)
TARGETS_DEV := $(TARGETS_SKYEYE_DEV) $(TARGETS_QEMU_DEV)

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
	@echo [normal] finish compile QEMU and Skyeye device.

# for make {device_name}
$(TARGETS): $(TARGETS_OBJ)
	@echo [normal] finish compile QEMU and Skyeye device.

# create each module rule
define PROGRAM_TEMPLATE

ifneq ($(SKYEYE_BUILD),)
$(SKYEYE_DIR_IN)/$(1)/lib$(1).so $(SKYEYE_DIR_IN)/$(1)/lib$(1).dll: $(GOAL_DEV)
	@cd ../targets/skyeye && $(MAKE) $(1)
$(1)_skyeye_dev:
	@cd $(1) && make skyeye
endif # SKYEYE_BUILD

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

$(1)_dev_clean:
	@cd $(1) && make clean
$(1)_dev_distclean:
	@cd $(1) && make distclean
endef

# create rule for all module
$(foreach var,$(MODULES),$(eval $(call PROGRAM_TEMPLATE,$(var))))

clean: $(MODULES_CLEAN)

distclean: $(MODULES_DISTCLEAN)

.PHONY: all clean distclean $(MODULES)
