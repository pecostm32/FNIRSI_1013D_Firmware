/*
 * strcpy.s
 */
	.text

    .global strcpy
    .type strcpy, %function
    .align 4

strcpy:
1:  ldrb  r3, [r1], #1
    strb  r3, [r0], #1
    cmp   r3, #0
    bne   1b
    mov   pc, lr
