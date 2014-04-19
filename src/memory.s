%include "constants.s"

section .text
[BITS 32]

global init_memory
global page_count
global page_head
	
page_head: dd 0
page_count: dd 0

init_memory:
        mov ecx,[MEM_MAP_SIZE]
        mov ebp,MEM_MAP
        mov edi,0

register_entry:     
        cmp ecx,0
        je done_entry
        
        test dword [ebp+0x14],1
        jz skip_entry
        cmp dword [ebp+0x10],1
        jne skip_entry
                
        mov ebx,[ebp]
        add ebx,0xfff
        and ebx,0xfffff000

        mov edx,ebx
        add edx,dword [ebp+0x8]
        add edx,1
        and edx,0xfffff000

        mov eax,[page_head]

        cmp ebx,0x100000
        jge register_page
        mov ebx,0x100000

register_page:
        cmp ebx,edx
        jg done_page
        mov [ebx],eax
        mov eax,ebx
        add ebx,PAGE_SIZE
        inc edi
        jmp register_page
done_page:
        mov [page_head],eax

skip_entry:      
        add ebp,24
        dec ecx
        jmp register_entry
        
done_entry:
        mov [page_count],edi
        ret

