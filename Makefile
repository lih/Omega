all: 
	@cook -l build-log

run: all
	cd dist && ./run fast
run-gui: all
	cd dist && ./run gui
disasm: all
	ndisasm -u -o 0x1200 .build/obj/kernel.bin

clean:
	cook clean
