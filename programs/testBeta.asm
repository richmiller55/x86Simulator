; testBeta.asm
; A program to test the new arithmetic and logical instructions.

section .text
global _start

_start:
    ; --- Test MUL ---
    ; Multiply EAX (5) by EBX (10). Result should be 50.
    ; EAX should be 0x32, EDX should be 0.
    mov eax, 5
    mov ebx, 10
    mul ebx

    ; --- Test DEC ---
    ; Decrement ECX.
    ; ECX starts at 50, becomes 49.
    mov ecx, 50
    dec ecx

    ; --- Test DIV ---
    ; Divide EDX:EAX (50) by EBX (5).
    ; EAX (quotient) should be 10, EDX (remainder) should be 0.
    mov edx, 0
    mov eax, 50
    mov ebx, 5
    div ebx

    ; --- Test AND ---
    ; EAX = 0b1100, EBX = 0b1010. Result in EAX should be 0b1000 (8).
    mov eax, 12
    mov ebx, 10
    and eax, ebx

    ; --- Test OR ---
    ; EAX = 0b1000, EBX = 0b0101. Result in EAX should be 0b1101 (13).
    mov eax, 8
    mov ebx, 5
    or eax, ebx

    ; --- Test XOR ---
    ; XOR EAX with itself to clear it. Result should be 0.
    mov eax, 1337
    xor eax, eax

    ; --- Test NOT ---
    ; Invert all bits of EAX.
    ; EAX = 0. NOT EAX should result in 0xFFFFFFFF.
    not eax

    ; --- Test XOR with immediate ---
    ; EBX = 0b1111. XOR with 0b0101. Result should be 0b1010 (10).
    mov ebx, 15
    xor ebx, 5

    ; --- End of Program ---
    ; Use the standard exit syscall.
    ; EAX = 1 (sys_exit)
    ; EBX = 0 (exit code)
    mov eax, 1
    mov ebx, 0
    int 0x80

exit_loop:
    jmp exit_loop
