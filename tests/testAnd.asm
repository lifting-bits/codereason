BITS 32
SEGMENT .text

_s:
    mov eax, ecx
    and eax, 8
    ret
