%include "constants.S"

extern  gdt, idt, kernelSpace
global nop, flushGDT, getIP, \
	outportb,inportb,inportw,outportw, \
	setPageDirectory,enablePaging,disablePaging, \
	getLifeRegister,setLifeRegister,getPL, compareAndSet, \
	realMode, protectedMode

getIP:
	mov eax,[esp]
	ret
	
realMode:
	push ebx
	push ecx
	mov ebx,[esp+12]
	mov ecx,esp

	call disablePaging
	lgdt [rmGDT]
	jmp CODE_SEGMENT:_16bitMode
[BITS 16]
_16bitMode:
	mov esp,KERNEL_START
	
	mov eax,cr0
	and eax,0fffffffeh
	mov cr0,eax
	jmp 0:_realMode
_realMode:	
	mov ax,0
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov fs,ax
	mov ss,ax
	lidt [rmIDT]

	call bx

	lgdt [gdt]
	mov eax,cr0
	or eax,1
	mov cr0,eax
	jmp CODE_SEGMENT:_pMode
[BITS 32]
_pMode:	
	mov ax,DATA_SEGMENT
	mov ss,ax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov esp,ecx

	lidt [idt]

	mov eax,[kernelSpace]
	mov cr3,eax
	call enablePaging
	
	pop ecx
	pop ebx
	ret

	ALIGN 4
	dw 0
rmIDT:
	dw 3ffh
	dd 0

	dw 0
rmGDT:
 	dw rmGDT_end-rmGDT_start-1
	dd rmGDT_start

rmGDT_start:
	dq 0

	;; GDT 16-bit code segment
	dw 0FFFFh 		; segment size 0-15
	dw 0			; segment base 0-15
	db 0 			; segment base 16-23
	db 10011010b		; 0: present ; 1-2: dpl 0 ; 3: code/data segment ; 4-7: segment type
	db 10001111b 		; 0-3: segment size 16-19 ; 
	db 0			; segment base 24-31

	;; GDT 16-bit data segment
	dw 0FFFFh
	dw 0
	db 0
	db 10010010b
	db 10001111b
	db 0
rmGDT_end:

compareAndSet:	
	mov edx,[esp+4]
	mov eax,[esp+8]
	lock cmpxchg [edx], eax
	ret

getPL:
	pushf
	pop eax
	ret
	
nop:
	times 100 nop
	ret

flushGDT:
	lgdt [gdt]
	jmp CODE_SEGMENT:flush_cs

flush_cs:
	mov ax,DATA_SEGMENT
	mov ss,ax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	ret

getLifeRegister:
	xor eax,eax
	str ax
	ret
setLifeRegister:
	mov ax,[esp+4]
	ltr ax
	ret
	
enablePaging:
	mov eax, cr4
	or eax, 0x00000010
	mov cr4, eax
	mov eax,cr0
	or eax,80000000h
	mov cr0,eax
	ret

disablePaging:
	mov eax,cr0
	and eax,7fffffffh
	mov cr0,eax
	mov eax, cr4
	and eax, 0xffffffef
	mov cr4, eax
	ret

setPageDirectory:
	mov eax,[esp+4]
	and eax,0xfffff000
	mov cr3,eax
	ret

inportb:
	mov edx,[esp+4]
	xor eax,eax
	in al,dx
	ret

outportb:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,al
	ret

inportw:
	mov edx,[esp+4]
	xor eax,eax
	in ax,dx
	ret

outportw:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,ax
	ret
