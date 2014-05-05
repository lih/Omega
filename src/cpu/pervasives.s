%include "constants.S"

extern  gdt
global nop, flushGDT, \
	outportb,inportb,inportw,outportw, \
	setPageDirectory,enablePaging,disablePaging, \
	getTaskRegister,setTaskRegister,getPL

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

getTaskRegister:
	xor eax,eax
	str ax
	ret
setTaskRegister:
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
