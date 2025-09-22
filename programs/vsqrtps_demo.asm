section .data
    ; Data arrays must be 32-byte aligned for optimal AVX performance.
    align 32
    ; Define an array of 8 single-precision floats (4 bytes each).
    ; We'll use values with easy-to-verify square roots.
    input_array: dd 1.0, 4.0, 9.0, 16.0, 25.0, 36.0, 49.0, 64.0
    
    ; Define a result array to hold the output.
    result_array: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

section .text
    global _start

_start:
    ; Load the 8 single-precision floating-point values from 'input_array'
    ; into the 256-bit YMM0 register.
    vmovups ymm0, [input_array]

    ; Compute the square root of each packed single-precision floating-point value.
    ; VSQRTPS is a VEX-coded instruction.
    ; It takes the source operand (ymm0) and stores the result in the destination register (ymm1).
    vsqrtps ymm1, ymm0

    ; Store the results from YMM1 back into the 'result_array' in memory.
    vmovups [result_array], ymm1

    ; --- Print the result (Linux specific) ---
    ; For a simple demonstration, we will exit the program and inspect the
    ; memory with a debugger to verify the results.
    ; The 'result_array' should contain:
    ; {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}

    ; Exit the program (Linux syscall)
    mov rax, 60         ; syscall number for 'exit'
    xor rdi, rdi        ; exit code 0
    syscall
