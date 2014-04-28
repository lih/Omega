%include "constants.S"

extern  gdtDesc
global nop, flushGDT, \
	outportb,inportb,inportw,outportw, \
	setPageDirectory,enablePaging,disablePaging, \
	getTaskRegister,setTaskRegister
	
nop:
	times 100 nop
	ret

flushGDT:
	lgdt [gdtDesc]
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
	and eax,0xfffffc00
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
	
enter_real_mode:
	cli
	lgdt [rm_gdt_desc]
	lidt [rm_idt]
	
	jmp CODE_SEGMENT:mode_16bit

[BITS 16]
mode_16bit:
	xor eax,eax
	mov cr3,eax
	mov eax,cr0
	xor eax,1
	mov cr0,eax
	jmp CODE_SEGMENT:real_mode

real_mode:	
	mov ax,DATA_SEGMENT
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	ret
	
rm_gdt:
	dq 0

	;;  Real-mode GDT code segment
	dw 0FFFFh 		; segment size 0-15
	dw 0			; segment base 0-15
	db 0 			; segment base 16-23
	db 10011010b		; 0-3: segment type ; 4: present ; 5.6: dpl 0 ; 7: code/data segment
	db 00001111b 		; 0-3: segment size 16-19 ; 5: 32-bit segment ; 6: 16-bit segment ; 7: small
	db 0			; segment base 24-31

	;; real-mode GDT data segment
	dw 0FFFFh 		; segment size 0-15
	dw 0			; segment base 0-15
	db 0 			; segment base 16-23
	db 10010010b		; 0-3: segment type ; 4: present ; 5.6: dpl 0 ; 7: code/data segment
	db 00001111b 		; 0-3: segment size 16-19 ; 5: 32-bit segment ; 6: 16-bit segment ; 7: small
	db 0			; segment base 24-31
rm_gdt_end:

rm_gdt_desc:
	dw rm_gdt_end-rm_gdt-1
	dd rm_gdt

rm_idt:
	dw 0x03ff
	dd 0
