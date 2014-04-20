%include "constants.S"

section .text
global kmain
extern main

kmain:

[BITS 16]
reset_drive:
	mov ah, 0
	int 13h
	or ah, ah
	jnz reset_drive

	;; load the last missing sectors, clobbering the bootsector
	mov ax, 0
	mov es, ax
	mov bx, KERNEL+512

	mov ah, 02h
	mov al, 30h
	mov ch, 0
	mov cl, 03h
	mov dh, 0
	int 13h
	or ah, ah
	jnz reset_drive

	;; Load the memory map
	mov ebp,0
	xor ebx,ebx
	xor ax,ax
	mov es,ax
	mov di, MEM_MAP
	mov edx,SMAP_MAGIC
	
get_entry:	
	mov eax,0e820h
	mov ecx,24
	int 15h
	jc end_entry
	cmp ebx,0
	je end_entry
	add di,24
	inc ebp
	jmp get_entry
end_entry:	
	
	mov [MEM_MAP_SIZE],ebp
	
	cli
	xor ax,ax
	mov ds,ax
	lgdt [gdt_desc]

	mov eax,cr0
	or eax,1
	mov cr0,eax

	jmp CODE_SEGMENT:protected_mode

[BITS 32]
protected_mode:
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov es, ax
	mov esp, KERNEL_STACK
	
        call main
	jmp $
	
	ALIGN 8
gdt:
	dq 0

	;; GDT code segment
	dw 0FFFFh 		; segment size 0-15
	dw 0			; segment base 0-15
	db 0 			; segment base 16-23
	db 10011010b		; 0: present ; 1-2: dpl 0 ; 3: code/data segment ; 4-7: segment type
	db 11001111b 		; 0-3: segment size 16-19 ; 
	db 0			; segment base 24-31

	;; GDT data segment
	dw 0FFFFh
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0
gdt_end:

gdt_desc:
	dw gdt_end-gdt
	dd gdt
	

