section .text
global _start

_start:
    ; Set up registers with values to be added.
    mov eax, 10           ; Move the value 10 into the EAX register.
    mov ebx, 20           ; Move the value 20 into the EBX register.
    
    ; Add the two numbers, result goes into EAX.
    add eax, ebx          ; EAX = EAX + EBX.
    
    ; Check if the result is correct.
    mov ecx, 30           ; Set a known-good value of 30 in ECX.
    cmp eax, ecx          ; Compare the result in EAX with the value in ECX.
    jne exit_fail         ; If EAX is not equal to ECX, jump to exit_fail.
    
    ; If the comparison passed, exit with a success code.
exit_success:
    mov ebx, 0            ; Set exit code to 0 (success).
    mov eax, 1            ; System call number for exit (1).
    int 0x80              ; Execute the system call.
    
    ; This block is executed only if the comparison fails.
exit_fail:
    mov ebx, 1            ; Set exit code to 1 (failure).
    mov eax, 1            ; System call number for exit (1).
    int 0x80              ; Execute the system call.
