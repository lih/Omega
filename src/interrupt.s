%include "constants.S"

extern idt,handleException,handleIRQ,handleSyscall
global enableInterrupts,disableInterrupts,loadInterrupts,reservedException,otherIRQ

enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret

loadInterrupts:
	lidt [idt]
	ret

%macro dummyException 1
global isr%1

isr%1:
	cli
	push byte 0
	push byte %1
	jmp commonException
%endmacro

%macro fullException 1
global isr%1

isr%1:	
	cli
	push byte %1
	jmp commonException
%endmacro
	
reservedException:
	cli
	push byte 0
	push byte 19
	jmp commonException

otherIRQ:
	cli
	push byte 0
	push byte 48
	jmp commonIRQ

%macro irq 1
global irq%1
irq%1:
	cli
	push byte 0
	push byte %1
	jmp commonIRQ
%endmacro

%macro syscall 1
global syscall%1
syscall%1:
	cli
	push byte 0
	push word %1
	jmp commonSyscall
%endmacro

		
%macro commonStub 2
common%1:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp   ; Push us the stack
	push eax
	mov eax, %2
	call eax       ; A special call, preserves the 'eip' register
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8     ; Cleans up the pushed error code and pushed Exception number
	iret
%endmacro

	commonStub Exception,handleException
	commonStub IRQ,handleIRQ
	
	dummyException 0
	dummyException 1
	dummyException 2
	dummyException 3
	dummyException 4
	dummyException 5
	dummyException 6
	dummyException 7
	fullException 8
	dummyException 9
	fullException 10
	fullException 11
	fullException 12
	fullException 13
	fullException 14
	dummyException 15
	dummyException 16
	dummyException 17
	dummyException 18
	dummyException 19
	dummyException 20
	dummyException 21
	dummyException 22
	dummyException 23
	dummyException 24
	dummyException 25
	dummyException 26
	dummyException 27
	dummyException 28
	dummyException 29
	dummyException 30
	dummyException 31
	dummyException 32
	irq 0
	irq 1
	irq 2
	irq 3
	irq 4
	irq 5
	irq 6
	irq 7
	irq 8
	irq 9
	irq 10
	irq 11
	irq 12
	irq 13
	irq 14
	irq 15
