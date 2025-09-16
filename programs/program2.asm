; A simple program to test basic MOV and JMP.
; It initializes two registers and then enters an infinite loop.

mov eax, 0xDEADBEEF  ; Load a distinct value into EAX
mov ebx, eax        ; Copy the value from EAX to EBX

endless_loop:
    jmp endless_loop    ; Infinite loop to halt execution