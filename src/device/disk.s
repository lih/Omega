global ata_read

ata_read:
	mov eax,[esp+12]
	mov ecx,[esp+8]
	mov edi,[esp+4]
	
	and eax, 0x0FFFFFFF
 
        mov ebx, eax         ; Save LBA in EBX
 
        mov dx, 1f6h         ; Port to send drive and bit 24 - 27 of LBA
        shr eax, 24          ; Get bit 24 - 27 in al
        or al, 11100000b     ; Set bit 6 in al for LBA mode
        out dx, al
	
        mov dx, 1f2h         ; Port to send number of sectors
        mov al, cl           ; Get number of sectors from CL
        out dx, al
	
        mov dx, 1f3h         ; Port to send bit 0 - 7 of LBA
        mov al, bl           ; Get LBA from EBX
        out dx, al
	
        mov dx, 1f4h         ; Port to send bit 8 - 15 of LBA
        mov ax, bx           ; Get LBA from EBX
        shr ax, 8            ; Get bit 8 - 15 in AL
        out dx, al
	
	mov dx, 1f5h       ; Port to send bit 16 - 23 of LBA
        mov eax, ebx         ; Get LBA from EBX
        shr eax, 16          ; Get bit 16 - 23 in AL
        out dx, al
	
        mov dx, 1f7h       ; Command port
        mov al, 20h         ; Read with retry.
        out dx, al
 
.still_going:
	in al, dx
        test al, 8           ; the sector buffer requires servicing.
        jz .still_going      ; until the sector buffer is ready.
	
        mov dx, 1f0h         ; Data port, in and out
        rep insw             ; in to [EDI]
 
        ret
