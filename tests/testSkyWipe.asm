BITS 32
SEGMENT .text

_s:
        lea     ecx, [eax+11h]
        add     eax, 0Bh
        imul    ecx, eax
        mov     edx, ecx
        shr     edx, 8
        mov     eax, edx
        xor     eax, ecx
        shr     eax, 10h
        xor     eax, edx
        xor     eax, ecx
        ret
