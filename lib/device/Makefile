targets = vic_device uart_device

targets_dir = $(patsubst %,%_dir,$(targets))
targets_clean = $(patsubst %,%_clean,$(targets))
targets_distclean = $(patsubst %,%_distclean,$(targets))
targets_ins = $(patsubst %,%_ins,$(targets))

all: $(targets_dir)
	@echo "[END] finish compiling all devices"

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

.PHONY: clean distclean install
