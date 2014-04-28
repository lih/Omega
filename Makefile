all: 
	@cook -l build-log

run: all
	@cd run && ./run fast
run-gui: all
	@cd run && ./run gui

disasm: all
	@ndisasm -u -o 0x1200 .build/obj/kernel.bin

clean:
	@cook clean
