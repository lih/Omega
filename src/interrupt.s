%include "constants.S"

extern idtp,handleISR,handleIRQ
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
	dummyISR 48
	dummyISR 49
	dummyISR 50
	dummyISR 51
	dummyISR 52
	dummyISR 53
	dummyISR 54
	dummyISR 55
	dummyISR 56
	dummyISR 57
	dummyISR 58
	dummyISR 59
	dummyISR 60
	dummyISR 61
	dummyISR 62
	dummyISR 63
	dummyISR 64
	dummyISR 65
	dummyISR 66
	dummyISR 67
	dummyISR 68
	dummyISR 69
	dummyISR 70
	dummyISR 71
	dummyISR 72
	dummyISR 73
	dummyISR 74
	dummyISR 75
	dummyISR 76
	dummyISR 77
	dummyISR 78
	dummyISR 79
	dummyISR 80
	dummyISR 81
	dummyISR 82
	dummyISR 83
	dummyISR 84
	dummyISR 85
	dummyISR 86
	dummyISR 87
	dummyISR 88
	dummyISR 89
	dummyISR 90
	dummyISR 91
	dummyISR 92
	dummyISR 93
	dummyISR 94
	dummyISR 95
	dummyISR 96
	dummyISR 97
	dummyISR 98
	dummyISR 99
	dummyISR 100
	dummyISR 101
	dummyISR 102
	dummyISR 103
	dummyISR 104
	dummyISR 105
	dummyISR 106
	dummyISR 107
	dummyISR 108
	dummyISR 109
	dummyISR 110
	dummyISR 111
	dummyISR 112
	dummyISR 113
	dummyISR 114
	dummyISR 115
	dummyISR 116
	dummyISR 117
	dummyISR 118
	dummyISR 119
	dummyISR 120
	dummyISR 121
	dummyISR 122
	dummyISR 123
	dummyISR 124
	dummyISR 125
	dummyISR 126
	dummyISR 127
[warning -number-overflow]
	dummyISR 128
	dummyISR 129
	dummyISR 130
	dummyISR 131
	dummyISR 132
	dummyISR 133
	dummyISR 134
	dummyISR 135
	dummyISR 136
	dummyISR 137
	dummyISR 138
	dummyISR 139
	dummyISR 140
	dummyISR 141
	dummyISR 142
	dummyISR 143
	dummyISR 144
	dummyISR 145
	dummyISR 146
	dummyISR 147
	dummyISR 148
	dummyISR 149
	dummyISR 150
	dummyISR 151
	dummyISR 152
	dummyISR 153
	dummyISR 154
	dummyISR 155
	dummyISR 156
	dummyISR 157
	dummyISR 158
	dummyISR 159
	dummyISR 160
	dummyISR 161
	dummyISR 162
	dummyISR 163
	dummyISR 164
	dummyISR 165
	dummyISR 166
	dummyISR 167
	dummyISR 168
	dummyISR 169
	dummyISR 170
	dummyISR 171
	dummyISR 172
	dummyISR 173
	dummyISR 174
	dummyISR 175
	dummyISR 176
	dummyISR 177
	dummyISR 178
	dummyISR 179
	dummyISR 180
	dummyISR 181
	dummyISR 182
	dummyISR 183
	dummyISR 184
	dummyISR 185
	dummyISR 186
	dummyISR 187
	dummyISR 188
	dummyISR 189
	dummyISR 190
	dummyISR 191
	dummyISR 192
	dummyISR 193
	dummyISR 194
	dummyISR 195
	dummyISR 196
	dummyISR 197
	dummyISR 198
	dummyISR 199
	dummyISR 200
	dummyISR 201
	dummyISR 202
	dummyISR 203
	dummyISR 204
	dummyISR 205
	dummyISR 206
	dummyISR 207
	dummyISR 208
	dummyISR 209
	dummyISR 210
	dummyISR 211
	dummyISR 212
	dummyISR 213
	dummyISR 214
	dummyISR 215
	dummyISR 216
	dummyISR 217
	dummyISR 218
	dummyISR 219
	dummyISR 220
	dummyISR 221
	dummyISR 222
	dummyISR 223
	dummyISR 224
	dummyISR 225
	dummyISR 226
	dummyISR 227
	dummyISR 228
	dummyISR 229
	dummyISR 230
	dummyISR 231
	dummyISR 232
	dummyISR 233
	dummyISR 234
	dummyISR 235
	dummyISR 236
	dummyISR 237
	dummyISR 238
	dummyISR 239
	dummyISR 240
	dummyISR 241
	dummyISR 242
	dummyISR 243
	dummyISR 244
	dummyISR 245
	dummyISR 246
	dummyISR 247
	dummyISR 248
	dummyISR 249
	dummyISR 250
	dummyISR 251
	dummyISR 252
	dummyISR 253
	dummyISR 254
	dummyISR 255

