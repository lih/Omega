%include "constants.S"

extern idt,handleException,handleIRQ,handleSyscall
global enableInterrupts,disableInterrupts,loadInterrupts,reservedException,otherIRQ,unhandledException

enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret

loadInterrupts:
	lidt [idt]
	ret

unhandledException:
	cli

_unhandledException:	
	call handleException
	iret

	jmp _unhandledException
