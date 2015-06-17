BITS 32
SEGMENT .text

_s:
    mov eax, ecx
    mov ebx, 2
    mul ebx
    imul ebx
    ret
