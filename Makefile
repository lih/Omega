.PHONY: all docs clean disasm run run-gui

all: 
	@cook -l build-log

run: all
	@cd run && ./run fast
run-gui: all
	@cd run && ./run gui

disasm: all
	@ndisasm -u -o 0x1200 .build/obj/kernel.bin

docs:
	@cook docs

clean:
	@cook clean
