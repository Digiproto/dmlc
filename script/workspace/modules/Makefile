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
MODULES_DEV := $(patsubst %,%_dev,$(MODULES))
# create clean rule
MODULES_CLEAN := $(patsubst %,%_dev_clean,$(MODULES))
MODULES_DISTCLEAN := $(patsubst %,%_dev_distclean,$(MODULES))

# modify target list
# remove reserve keywords
SPECIAL_WORDS := clean distclean all
TARGETS := $(filter-out $(SPECIAL_WORDS),$(MAKECMDGOALS))
TARGETS_DEV := $(patsubst %,%_dev,$(TARGETS))

# skyeye or qemu target goals
ifneq ($(SKYEYE_BUILD),)
ALL_SKYEYE_SRC := $(foreach var,$(MODULES),$(SKYEYE_DIR_IN)/$(var)/$(var).c)
TARGETS_SKYEYE_SRC := $(foreach var,$(TARGETS),$(SKYEYE_DIR_IN)/$(var)/$(var).c)
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
ALL_QEMU_SRC := $(foreach var,$(MODULES),$(QEMU_DIR_IN)/$(var)/$(var).c)
TARGETS_QEMU_SRC := $(foreach var,$(TARGETS),$(QEMU_DIR_IN)/$(var)/$(var).c)
endif # QEMU_BUILD

# all target goals
ALL_OBJ := $(ALL_SKYEYE_OBJ) $(ALL_QEMU_OBJ)
TARGETS_OBJ := $(TARGETS_SKYEYE_OBJ) $(TARGETS_QEMU_OBJ)
ALL_SRC := $(ALL_SKYEYE_SRC) $(ALL_QEMU_SRC)
TARGETS_SRC := $(TARGETS_SKYEYE_SRC) $(TARGETS_QEMU_SRC)

all: $(ALL_SRC)
ifneq ($(QEMU_BUILD),)
	@./setMakefile.qemu $(MODULES)
	@echo [normal] configure QEMU
	@cd $(QEMU_DIR) && ./configure $(QEMU_CONF_FLAGS) 1>/dev/null
	@echo [normal] make QEMU
	@cd $(QEMU_DIR) && $(MAKE) 1>/dev/null
endif # QEMU_BUILD
ifneq ($(SKYEYE_BUILD),)
	@cd ../targets/skyeye && $(MAKE)
endif # SKYEYE_BUILD

$(TARGETS): $(TARGETS_SRC)
ifneq ($(QEMU_BUILD),)
	@./setMakefile.qemu $(TARGETS)
	@echo [normal] configure QEMU
	@cd $(QEMU_DIR) && ./configure $(QEMU_CONF_FLAGS) 1>/dev/null
	@echo [normal] make QEMU
	@cd $(QEMU_DIR) && $(MAKE) 1>/dev/null
endif # QEMU_BUILD
ifneq ($(SKYEYE_BUILD),)
	@cd ../targets/skyeye && $(MAKE) $(TARGETS)
endif # SKYEYE_BUILD

# create each module rule
define PROGRAM_TEMPLATE

#$(SKYEYE_DIR_IN)/$(1)/lib$(1).so $(SKYEYE_DIR_IN)/$(1)/lib$(1).dll:
#$(QEMU_DIR_IN)/$(1)/$(1).o:

ifneq ($(SKYEYE_BUILD),)
$(SKYEYE_DIR_IN)/$(1)/$(1).c:
	@cd $(1) && make skyeye
endif # SKYEYE_BUILD

ifneq ($(QEMU_BUILD),)
$(QEMU_DIR_IN)/$(1)/$(1).c:
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
