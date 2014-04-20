%include "constants.S"

extern idtp,handleISR,handleIRQ
global enableInterrupts,disableInterrupts,loadInterrupts,reservedISR,otherIRQ
	
enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret

loadInterrupts:
	lidt [idtp]
	ret

%macro dummyISR 1
global isr%1

isr%1:
	cli
	push byte 0
	push byte %1
	jmp commonISR
%endmacro

%macro fullISR 1
global isr%1

isr%1:	
	cli
	push byte %1
	jmp commonISR
%endmacro
	
	dummyISR 0
	dummyISR 1
	dummyISR 2
	dummyISR 3
	dummyISR 4
	dummyISR 5
	dummyISR 6
	dummyISR 7
	fullISR 8
	dummyISR 9
	fullISR 10
	fullISR 11
	fullISR 12
	fullISR 13
	fullISR 14
	dummyISR 15
	dummyISR 16
	dummyISR 17
	dummyISR 18

reservedISR:
	cli
	push byte 0
	push byte 19
	jmp commonISR

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
	add esp, 8     ; Cleans up the pushed error code and pushed ISR number
	iret
%endmacro

	commonStub ISR,handleISR
	commonStub IRQ,handleIRQ

