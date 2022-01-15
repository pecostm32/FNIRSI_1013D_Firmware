.text
	.arm

	.global	_start
_start:
	/* Boot head information for BROM */
	.long 0xea000006
	.byte 'e', 'G', 'O', 'N', '.', 'B', 'T', '0'
	.long 0, __bootloader_size
	.byte 'S', 'P', 'L', 2
	.long 0, 0

_reset:
	/* Initialize stacks. Interrupts disabled */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xdb
	msr cpsr, r0
	ldr sp, _stack_und_end

	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd7
	msr cpsr, r0
	ldr sp, _stack_abt_end

	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd2
	msr cpsr, r0
	ldr sp, _stack_irq_end

	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd1
	msr cpsr, r0
	ldr sp, _stack_fiq_end

	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xdf
	msr cpsr, r0
	ldr sp, _stack_sys_end

	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0
	ldr sp, _stack_srv_end

	/* Set vector to the low address */
	//mrc p15, 0, r0, c1, c0, 0
	//bic r0, #0x2000
//	mcr p15, 0, r0, c1, c0, 0

//
	/* Call main */
	ldr r1, =main
	mov pc, r1

/*
 * The location of section
 */
 	.align 4
_stack_und_end:
	.long 0x00006C00
_stack_abt_end:
	.long 0x00007000
_stack_irq_end:
	.long 0x00007400
_stack_fiq_end:
	.long 0x00007800
_stack_sys_end:
	.long 0x00008000
_stack_srv_end:
	.long 0x00007C00
