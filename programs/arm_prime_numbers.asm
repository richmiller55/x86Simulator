@ arm_prime_numbers.asm
@ An ARM program to find prime numbers from 1 to 100.

.section .text
.global _start

_start:
    mov r0, #2          @ r0 is the number to test (n)

prime_loop:
    cmp r0, #100        @ Check if n > 100
    bgt end_program     @ If so, exit

    mov r1, #2          @ r1 is the divisor (i)
    mov r2, r0          @ Copy n to r2 for the division

test_divisor_loop:
    cmp r1, r2          @ Check if i >= n
    bge is_prime        @ If so, n is prime

    udiv r3, r2, r1     @ r3 = n / i
    mul r3, r3, r1      @ r3 = (n / i) * i
    sub r3, r2, r3      @ r3 = n - ((n / i) * i) (i.e., the remainder)

    cmp r3, #0          @ Is the remainder 0?
    beq not_prime       @ If so, n is not prime

    add r1, r1, #1      @ i = i + 1
    b test_divisor_loop

is_prime:
    @ This is where you would print the prime number (r0).
    @ For now, we just continue to the next number.

not_prime:
    add r0, r0, #1      @ n = n + 1
    b prime_loop

end_program:
    @ Exit syscall for ARM
    mov r7, #1          @ syscall number for exit
    mov r0, #0          @ exit code 0
    swi 0               @ Software interrupt to call the kernel
