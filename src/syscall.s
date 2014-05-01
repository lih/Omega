%include "constants.S"
	
%macro syscallstub_0 1
	mov eax, %1
	int 48
	ret
%endmacro	
%macro syscallstub_1 1
	mov ebx,[esp+4]
	syscallstub_0 %1
%endmacro	
%macro syscallstub_2 1
	mov ecx,[esp+8]
	syscallstub_1 %1
%endmacro	

global syscall_die,syscall_spawn,syscall_warp,syscall_alloc
syscall_die:
	syscallstub_0 SYS_DIE
syscall_spawn:	
	syscallstub_2 SYS_SPAWN
syscall_warp:
	syscallstub_2 SYS_WARP
syscall_alloc:
	syscallstub_1 SYS_ALLOC
	
