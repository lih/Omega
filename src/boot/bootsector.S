%include "constants.S"
[BITS 16]

[ORG 0x7C00]

reset_drive:
	mov ah, 0
	int 13h
	or ah, ah
	jnz reset_drive

	;; load the first sector of our kernel at a proper location
	mov ax, 0
	mov es, ax
	mov bx, KERNEL_START

	mov ah, 02h
	mov al, 01h
	mov ch, 0
	mov cl, 02h
	mov dh, 0
	int 13h
	or ah, ah
	jnz reset_drive

	;; once loaded, jump to the main kernel function
	jmp KERNEL_START

	times (510-($-$$)) db 0
	dw 0aa55h
