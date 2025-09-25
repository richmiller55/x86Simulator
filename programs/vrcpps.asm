section .data
    ; Data must be 32-byte aligned for AVX
    align 32

    ; Test case 1: Standard floating-point values
    input_vals: dd 1.0, 2.0, 4.0, 8.0, 0.5, 0.25, -2.0, -4.0
    results_reg: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

    ; Test case 2: Special floating-point values
    ; +0.0 (rcp -> +inf)
    ; -0.0 (rcp -> -inf)
    ; +Infinity (rcp -> +0.0)
    ; -Infinity (rcp -> -0.0)
    ; NaN (rcp -> NaN)
    special_vals: dd 0.0, 0x80000000, 0x7f800000, 0xff800000, 0x7fc00000, 1.0, -1.0, 2.0
    results_special: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

    ; Test case 3: Memory source operand
    results_mem: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

section .text
    global _start

_start:
    ; --- Test 1: VRCPPS with register source ---
    vmovups ymm0, [input_vals]
    vrcpps ymm1, ymm0
    vmovups [results_reg], ymm1
    ; Expected result in results_reg: {~1.0, ~0.5, ~0.25, ~0.125, ~2.0, ~4.0, ~-0.5, ~-0.25}

    ; --- Test 2: VRCPPS with memory source ---
    vrcpps ymm2, [input_vals]
    vmovups [results_mem], ymm2
    ; Expected result in results_mem: {~1.0, ~0.5, ~0.25, ~0.125, ~2.0, ~4.0, ~-0.5, ~-0.25}

    ; --- Test 3: VRCPPS with special values ---
    vmovups ymm0, [special_vals]
    vrcpps ymm1, ymm0
    vmovups [results_special], ymm1
    ; Expected result in results_special: {+inf, -inf, +0.0, -0.0, NaN, ~1.0, ~-1.0, ~0.5}

    ; --- Exit the program (Linux syscall) ---
    mov rax, 60         ; syscall number for 'exit'
    xor rdi, rdi        ; exit code 0
    syscall
