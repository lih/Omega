%include "constants.S"
	
%macro syscallstub_0 1
	mov eax, %1
	int 48
	ret
%endmacro	
%macro syscallstub_1 1
	push ebx
	mov ebx,[esp+8]
	mov eax, %1
	int 48
	pop ebx
	ret
%endmacro	
%macro syscallstub_2 1
	push ebx
	push ecx
	mov ebx,[esp+12]
	mov ecx,[esp+16]	
	mov eax, %1
	int 48
	pop ecx
	pop ebx
	ret
%endmacro	
%macro syscallstub_3 1
	push ebx
	push ecx
	push edx
	mov ebx,[esp+16]
	mov ecx,[esp+20]	
	mov edx,[esp+24]
	mov eax, %1
	int 48
	pop edx	
	pop ecx
	pop ebx
	ret
%endmacro	

global syscall_die,syscall_spark,syscall_spawn,syscall_warp,syscall_alloc,syscall_acquire, \
	syscall_release,syscall_mapTo,syscall_mapFrom,syscall_anihilate,syscall_wait

syscall_die:
	syscallstub_0 SYS_DIE
syscall_spark:	
	syscallstub_2 SYS_SPARK
syscall_spawn:	
	syscallstub_0 SYS_SPAWN
syscall_warp:
	syscallstub_2 SYS_WARP
syscall_alloc:
	syscallstub_2 SYS_ALLOC
syscall_acquire:
	syscallstub_2 SYS_ACQUIRE
syscall_release:
	syscallstub_2 SYS_RELEASE
syscall_mapTo:
	syscallstub_3 SYS_MAPTO
syscall_mapFrom:
	syscallstub_3 SYS_MAPFROM
syscall_anihilate:
	syscallstub_1 SYS_ANIHILATE
syscall_wait:
	syscallstub_2 SYS_WAIT
	
