/*
 * strlen.s
 */
	.text

    .global strlen
    .type strlen, %function
    .align 4

strlen:
1:  mov   r1, r0
    ldrb  r3, [r1], #1
    cmp   r3, #0
    addne r0, #1
    bne   1b
    mov   pc, lr



