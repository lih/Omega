%include "constants.s"
[BITS 16]

[ORG 0x7C00]

;; 	;; Load the memory map
;; 	mov ebp,0
;; 	xor ebx,ebx
;; 	xor ax,ax
;; 	mov es,ax
;; 	mov di, MEM_MAP
;; 	mov edx,SMAP_MAGIC
	
;; get_entry:	
;; 	mov eax,0e820h
;; 	mov ecx,24
;; 	int 15h
;; 	jc end_entry
;; 	add di,24
;; 	inc ebp
;; 	jmp get_entry
;; end_entry:	
	
;; 	mov [MEM_MAP_SIZE],ebp
	
reset_drive:
	mov ah, 0
	int 13h
	or ah, ah
	jnz reset_drive
	
	;; load the first sector of our kernel at a proper location
	mov ax, 0
	mov es, ax
	mov bx, KERNEL

	mov ah, 02h
	mov al, 01h
	mov ch, 0
	mov cl, 02h
	mov dh, 0
	int 13h
	or ah, ah
	jnz reset_drive

	;; once loaded, jump to the main kernel function
	jmp KERNEL

	times (510-($-$$)) db 0
	dw 0aa55h
