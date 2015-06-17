BITS 32
SEGMENT .text

_s:
    mov eax, ecx
    add eax, 1
    jmp eax
