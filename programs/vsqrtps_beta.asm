section .data
    ; Data must be 32-byte aligned for AVX
    align 32

    ; Test case 1: Standard positive floating-point values
    positive_vals: dd 1.0, 4.0, 9.0, 16.0, 25.0, 36.0, 49.0, 64.0
    results_positive: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

    ; Test case 2: Special floating-point values
    ; -1.0 (sqrt -> NaN)
    ; +0.0 (sqrt -> +0.0)
    ; -0.0 (sqrt -> -0.0)
    ; +Infinity (sqrt -> +Infinity)
    ; -Infinity (sqrt -> NaN)
    ; NaN (sqrt -> NaN)
    ; 1.0, 1.0 (placeholders)
    special_vals: dd -1.0, 0.0, 0x80000000, 0x7f800000, 0xff800000, 0x7fc00000, 1.0, 1.0
    results_special: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

    ; Test case 3: Memory source operand
    results_mem_src: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

section .text
    global _start

_start:
    ; --- Test 1: VSQRTPS with register source (positive values) ---
    vmovups ymm0, [positive_vals]
    vsqrtps ymm1, ymm0
    vmovups [results_positive], ymm1
    ; Expected result in results_positive: {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}

    ; --- Test 2: VSQRTPS with register source (special values) ---
    vmovups ymm0, [special_vals]
    vsqrtps ymm1, ymm0
    vmovups [results_special], ymm1
    ; Expected result in results_special: {NaN, +0.0, -0.0, +Inf, NaN, NaN, 1.0, 1.0}

    ; --- Test 3: VSQRTPS with memory source ---
    vsqrtps ymm2, [positive_vals]
    vmovups [results_mem_src], ymm2
    ; Expected result in results_mem_src: {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}

    ; --- Exit the program (Linux syscall) ---
    mov rax, 60         ; syscall number for 'exit'
    xor rdi, rdi        ; exit code 0
    syscall
