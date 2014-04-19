CSrc:=$(wildcard src/*.c)
ASMSrc:=src/kmain.s $(wildcard src/*.s)
Headers:=$(wildcard src/*.h)

all: exec/disk
clean:
	@rm bin/*

run: all exec/disk
	cd exec && bochs/bochs -q

exec/disk: bin/bootsector.bin bin/kernel.bin
	@echo Creating disk image...
	@cat $^ /dev/zero | dd if='/dev/stdin' of=$@ bs=512 count=128

bin/%.s.o: src/%.s
	@echo Compiling $@...
	@nasm -i src/ -f elf32 $< -o $@
bin/%.c.o: src/%.c $(Headers)
	@echo Compiling $@...
	@gcc -m32 -w -c $< -o $@

bin/bootsector.bin: src/bootsector.S
	@echo Compiling bootsector
	@nasm -i src/ $< -o $@
bin/kernel.bin: $(ASMSrc:src/%.s=bin/%.s.o) $(CSrc:src/%.c=bin/%.c.o)
	@echo Linking Kernel...
	@ld -T kernel.ld -m elf_i386 $^ -o $@
