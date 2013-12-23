targets = dml_common common proxy device loader

prefix ?= /opt/dml_all

targets_dir = $(patsubst %,%_dir,$(targets))
targets_clean = $(patsubst %,%_clean,$(targets))
targets_distclean = $(patsubst %,%_distclean,$(targets))
targets_ins = $(patsubst %,%_ins,$(targets))

all: $(targets_dir)
	@echo "[END] finish compiling all dirs"

%_dir: %
	cd $< && make

%_clean: %
	@cd $< && make clean

%_distclean: %
	@cd $< && make distclean

%_ins: %
	@cd $< && make install

clean: $(targets_clean)

distclean: clean $(targets_distclean)

install: $(targets_ins)
	@echo "[INS] include to $(prefix)/include"
	@mkdir -p $(prefix)/include
	@cp -r include/* $(prefix)/include

.PHONY: clean distclean install
