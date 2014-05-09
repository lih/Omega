[BITS 16]
global int_mode13h, int_mode03h
	
int_mode13h:
	pusha
	mov al,13h
	mov ah,0
	int 10h
	popa
	ret
int_mode03h:
	pusha
	mov al,03h
	mov ah,0
	int 10h
	popa
	ret
