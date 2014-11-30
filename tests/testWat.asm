BITS 32

segment .text

_test:
xor     eax, eax
push    eax
sub     eax, 93939BD2h
push    eax
sub     eax, 3A02EEC0h
push    eax
sub     eax, 0D0FD0100h
push    eax
mov     eax, esp
push    eax
ret
