BITS 32
SEGMENT .text

_s:
    mov eax, ecx
    mov ebx, edx
    xor eax, ebx
    ret
