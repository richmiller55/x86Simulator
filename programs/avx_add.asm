section .data
    ; Define and align data for AVX operations.
    ; YMM registers operate on 256-bit data.
    ; An array of 8 single-precision floats is 8 * 4 = 32 bytes.
    ; A 256-bit register is 32 bytes, so we need 32-byte alignment.
    align 32
    array1: dd 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
    array2: dd 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0
    result: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

section .text
    global _start

_start:
    ; Load the first array into the 256-bit YMM0 register.
    ; The 'vmovups' instruction moves 8 packed single-precision floats.
    vmovups ymm0, [array1]

    ; Load the second array into the 256-bit YMM1 register.
    vmovups ymm1, [array2]

    ; Add the packed single-precision floating-point values from YMM1 to YMM0.
    ; The result is stored in YMM0. The 'vaddps' instruction is a VEX-coded AVX instruction.
    vaddps ymm0, ymm0, ymm1

    ; Store the result from YMM0 into the 'result' array in memory.
    vmovups [result], ymm0

    ; --- Print the result (Linux specific) ---
    ; In a real application, you would link this with a C function for printing.
    ; This part is for demonstrating the final result by halting execution.
    ; The final `result` array should contain the sums:
    ; {11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0}

    ; Exit the program (syscall in Linux)
    mov rax, 60         ; syscall number for 'exit'
    xor rdi, rdi        ; exit code 0
    syscall
