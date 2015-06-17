BITS 32
SEGMENT .text

_s:
    mov eax, DWORD [ ecx ]
    ret
