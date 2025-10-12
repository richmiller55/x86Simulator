@ arm_one.asm
@ A simple ARM program for the simulator.
@ It performs basic arithmetic and stores the result in memory.

.section .data
    result: .word 0 @ Reserve a 32-bit word in memory for the result

.section .text
.global _start

_start:
    @ Load initial values into registers
    mov r0, #10         @ r0 = 10
    mov r1, #20         @ r1 = 20

    @ Perform addition
    add r2, r0, r1      @ r2 = r0 + r1 (r2 should be 30)

    @ Perform subtraction
    sub r2, r2, #5      @ r2 = r2 - 5 (r2 should be 25)

    @ Store the final result into memory
    ldr r3, =result     @ Load the address of 'result' into r3
    str r2, [r3]        @ Store the value of r2 into the memory at the address in r3

end_loop:
    b end_loop          @ Infinite loop to halt the processor
