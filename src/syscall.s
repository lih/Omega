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
%macro syscallstub_3 1
	mov edx,[esp+12]
	syscallstub_2 %1
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
	
