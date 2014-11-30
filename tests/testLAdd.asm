BITS 32
SEGMENT .text

_s:
    mov eax, 5
    mov edx, 1
    lock xadd [EAX], EDX
    ret
