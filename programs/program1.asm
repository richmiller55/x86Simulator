; Program to calculate sum of numbers from 1 to 5
; Uses MOV, ADD,  CMP, JNE, INC

START:
    MOV EAX, 0          ; Initialize sum (EAX) to 0
    MOV ECX, 1          ; Initialize counter (ECX) to 1

LOOP_START:
    ADD EAX, ECX        ; Add current counter value to sum
    INC ECX             ; Increment counter
    CMP ECX, 6          ; Compare counter with 6 (we want to sum up to 5)
    JNE LOOP_START      ; If not equal, jump back to LOOP_START

    ; Program finishes, EAX should contain 1+2+3+4+5 = 15
    ; (In a real program, you'd typically have an exit instruction here)

END_PROGRAM:
    ; Placeholder for simulator termination or further operations
    MOV EBX, EAX        ; Copy result to EBX for observation
    JMP EXIT            ; Jump to a simulated exit point

EXIT:
    ; Simulated exit - program execution ends here
