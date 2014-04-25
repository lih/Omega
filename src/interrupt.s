%include "constants.S"

extern idtp,handleISR,handleIRQ,handleSyscall
global enableInterrupts,disableInterrupts,loadInterrupts,reservedISR,otherIRQ
	
enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret

loadInterrupts:
	mov eax,[esp+4]
	lidt [eax]
	ret

%macro dummyISR 1
global isr%1

isr%1:
	cli
	push byte 0
	push byte %1
	jmp commonISR
%endmacro

%macro fullISR 1
global isr%1

isr%1:	
	cli
	push byte %1
	jmp commonISR
%endmacro
	
reservedISR:
	cli
	push byte 0
	push byte 19
	jmp commonISR

otherIRQ:
	cli
	push byte 0
	push byte 48
	jmp commonIRQ

%macro irq 1
global irq%1
irq%1:
	cli
	push byte 0
	push byte %1
	jmp commonIRQ
%endmacro

%macro syscall 1
global syscall%1
syscall%1:
	cli
	push byte 0
	push word %1
	jmp commonSyscall
%endmacro

	
%macro commonStub 2
common%1:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp   ; Push us the stack
	push eax
	mov eax, %2
	call eax       ; A special call, preserves the 'eip' register
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8     ; Cleans up the pushed error code and pushed ISR number
	iret
%endmacro

	commonStub ISR,handleISR
	commonStub IRQ,handleIRQ
	commonStub Syscall,handleSyscall
	
	dummyISR 0
	dummyISR 1
	dummyISR 2
	dummyISR 3
	dummyISR 4
	dummyISR 5
	dummyISR 6
	dummyISR 7
	fullISR 8
	dummyISR 9
	fullISR 10
	fullISR 11
	fullISR 12
	fullISR 13
	fullISR 14
	dummyISR 15
	dummyISR 16
	dummyISR 17
	dummyISR 18
	dummyISR 19
	dummyISR 20
	dummyISR 21
	dummyISR 22
	dummyISR 23
	dummyISR 24
	dummyISR 25
	dummyISR 26
	dummyISR 27
	dummyISR 28
	dummyISR 29
	dummyISR 30
	dummyISR 31
	dummyISR 32
	irq 0
	irq 1
	irq 2
	irq 3
	irq 4
	irq 5
	irq 6
	irq 7
	irq 8
	irq 9
	irq 10
	irq 11
	irq 12
	irq 13
	irq 14
	irq 15
	syscall 0
	syscall 1
	syscall 2
	syscall 3
	syscall 4
	syscall 5
	syscall 6
	syscall 7
	syscall 8
	syscall 9
	syscall 10
	syscall 11
	syscall 12
	syscall 13
	syscall 14
	syscall 15
	syscall 16
	syscall 17
	syscall 18
	syscall 19
	syscall 20
	syscall 21
	syscall 22
	syscall 23
	syscall 24
	syscall 25
	syscall 26
	syscall 27
	syscall 28
	syscall 29
	syscall 30
	syscall 31
	syscall 32
	syscall 33
	syscall 34
	syscall 35
	syscall 36
	syscall 37
	syscall 38
	syscall 39
	syscall 40
	syscall 41
	syscall 42
	syscall 43
	syscall 44
	syscall 45
	syscall 46
	syscall 47
	syscall 48
	syscall 49
	syscall 50
	syscall 51
	syscall 52
	syscall 53
	syscall 54
	syscall 55
	syscall 56
	syscall 57
	syscall 58
	syscall 59
	syscall 60
	syscall 61
	syscall 62
	syscall 63
	syscall 64
	syscall 65
	syscall 66
	syscall 67
	syscall 68
	syscall 69
	syscall 70
	syscall 71
	syscall 72
	syscall 73
	syscall 74
	syscall 75
	syscall 76
	syscall 77
	syscall 78
	syscall 79
	syscall 80
	syscall 81
	syscall 82
	syscall 83
	syscall 84
	syscall 85
	syscall 86
	syscall 87
	syscall 88
	syscall 89
	syscall 90
	syscall 91
	syscall 92
	syscall 93
	syscall 94
	syscall 95
	syscall 96
	syscall 97
	syscall 98
	syscall 99
	syscall 100
	syscall 101
	syscall 102
	syscall 103
	syscall 104
	syscall 105
	syscall 106
	syscall 107
	syscall 108
	syscall 109
	syscall 110
	syscall 111
	syscall 112
	syscall 113
	syscall 114
	syscall 115
	syscall 116
	syscall 117
	syscall 118
	syscall 119
	syscall 120
	syscall 121
	syscall 122
	syscall 123
	syscall 124
	syscall 125
	syscall 126
	syscall 127
	syscall 128
	syscall 129
	syscall 130
	syscall 131
	syscall 132
	syscall 133
	syscall 134
	syscall 135
	syscall 136
	syscall 137
	syscall 138
	syscall 139
	syscall 140
	syscall 141
	syscall 142
	syscall 143
	syscall 144
	syscall 145
	syscall 146
	syscall 147
	syscall 148
	syscall 149
	syscall 150
	syscall 151
	syscall 152
	syscall 153
	syscall 154
	syscall 155
	syscall 156
	syscall 157
	syscall 158
	syscall 159
	syscall 160
	syscall 161
	syscall 162
	syscall 163
	syscall 164
	syscall 165
	syscall 166
	syscall 167
	syscall 168
	syscall 169
	syscall 170
	syscall 171
	syscall 172
	syscall 173
	syscall 174
	syscall 175
	syscall 176
	syscall 177
	syscall 178
	syscall 179
	syscall 180
	syscall 181
	syscall 182
	syscall 183
	syscall 184
	syscall 185
	syscall 186
	syscall 187
	syscall 188
	syscall 189
	syscall 190
	syscall 191
	syscall 192
	syscall 193
	syscall 194
	syscall 195
	syscall 196
	syscall 197
	syscall 198
	syscall 199
	syscall 200
	syscall 201
	syscall 202
	syscall 203
	syscall 204
	syscall 205
	syscall 206
	syscall 207
