global _start

section .text
_start:
    mov eax, 0x11223344
    mov ecx, 0x55667788

    ; Push values onto the stack
    push eax
    push ecx

    ; The stack is LIFO (Last-In, First-Out)
    ; pop ebx will get the value of ecx
    ; and pop edx will get the value of eax
    pop ebx  ; ebx should now be 0x55667788
    pop edx  ; edx should now be 0x11223344

    ; Exit the program
    mov eax, 1      ; sys_exit syscall
    mov ebx, 0      ; exit code 0
    int 0x80
