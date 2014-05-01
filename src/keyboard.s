global handleKeyboard
extern handleKey

handleKeyboard:
	cli

	mov dx,60h
	in al,dx
	movzx eax,al
	
	push eax
	call handleKey

	mov dx,20h
	mov al,20h
	out dx,al
	
	iret
	jmp handleKeyboard
