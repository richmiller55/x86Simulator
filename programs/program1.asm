; Program to calculate sum of numbers from 1 to 5
; Uses MOV, ADD,  CMP, JNE, INC

START:
    MOV RAX, 0          ; Initialize sum (RAX) to 0
    MOV RCX, 1          ; Initialize counter (RCX) to 1

LOOP_START:
    ADD RAX, RCX        ; Add current counter value to sum
    INC RCX             ; Increment counter
    CMP RCX, 6          ; Compare counter with 6 (we want to sum up to 5)
    JNE LOOP_START      ; If not equal, jump back to LOOP_START

    ; Program finishes, RAX should contain 1+2+3+4+5 = 15
    ; (In a real program, you'd typically have an exit instruction here)

END_PROGRAM:
    ; Placeholder for simulator termination or further operations
    MOV RBX, RAX        ; Copy result to RBX for observation
    JMP EXIT            ; Jump to a simulated exit point

EXIT:
    ; Simulated exit - program execution ends here
