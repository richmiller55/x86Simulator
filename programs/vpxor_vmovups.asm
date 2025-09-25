section .data
    ; Data must be 32-byte aligned for AVX
    align 32
    array1:  dd 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0
    array2:  dd 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
    
    ; Define a mask for the XOR operation.
    ; This mask is a pattern of all 1s (0xFFFFFFFF in hex) repeated 8 times.
    ; When you XOR with a mask of all 1s, it inverts all the bits.
    ; This is a common technique to achieve a bitwise NOT operation.
    mask:    dd -1, -1, -1, -1, -1, -1, -1, -1
    
    result1: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    result2: dd 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

section .text
    global _start

_start:
    ; --- Part 1: Perform Packed XOR using VPXOR ---
    
    ; Load the mask into ymm0.
    vmovups ymm0, [mask]

    ; Load array1 into ymm1.
    vmovups ymm1, [array1]
    
    ; Perform a packed bitwise XOR operation between ymm1 and ymm0.
    ; VPXOR is a VEX-coded instruction. The result (a bitwise NOT) is stored in ymm0.
    ; For floating-point numbers, this operation flips all the bits, including the sign bit.
    vpxor ymm0, ymm1, ymm0
    
    ; Store the result of the XOR operation in result1.
    vmovups [result1], ymm0
    
    ; --- Part 2: Perform Packed Subtract using VPSUBPS ---
    
    ; Load array1 into ymm0 (overwriting the previous value).
    vmovups ymm0, [array1]
    
    ; Load array2 into ymm1.
    vmovups ymm1, [array2]
    
    ; Perform a packed single-precision floating-point subtract.
    ; VPSUBPS is a VEX-coded instruction. It subtracts ymm1 from ymm0.
    vpsubps ymm0, ymm0, ymm1
    
    ; Store the result of the subtraction in result2.
    vmovups [result2], ymm0
    
    ; --- Exit the program (Linux specific) ---
    mov rax, 60         ; syscall number for 'exit'
    xor rdi, rdi        ; exit code 0
    syscall
