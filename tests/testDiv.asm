BITS 32
SEGMENT .text

_s:
    mov eax, ecx
    mov ebx, 4
    div ebx
    ret
