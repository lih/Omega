%include "constants.S"

global nop,shutdown,outportb,inportb
	
nop:
	times 100 nop
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
	
shutdown:
	;; Shutdown the computer
	call enter_real_mode
	mov ax, 0x5307
	mov bx, 1
	mov cx, 3
	int 15h
	jmp $
	
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
