.text
	.arm

	.global	_start
_start:
	//Boot head information for BROM
	.long 0xea000006
	.byte 'e', 'G', 'O', 'N', '.', 'E', 'X', 'E'
	.long 0, __program_size
	.byte 'E', 'X', 'E', 'C'
	.long 0, 0

_reset:
	//Initialize stacks. Interrupts disabled
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

	//Set vector to the low address
	mrc p15, 0, r0, c1, c0, 0
	bic r0, #0x2000
	mcr p15, 0, r0, c1, c0, 0

  //Copy vector to the correct address
  adr   r0, _vector
	mrc   p15, 0, r2, c1, c0, 0
	ands  r2, r2, #(1 << 13)
	ldreq r1, =0x00000000
	ldrne r1, =0xffff0000
  ldmia r0!, {r2-r8, r10}
  stmia r1!, {r2-r8, r10}
  ldmia r0!, {r2-r8, r10}
  stmia r1!, {r2-r8, r10}

  //Clear the used variables


	//Call main
	ldr r1, = main
	mov pc, r1

_vector:
	ldr pc, _jump_reset
	ldr pc, _undefined_instruction
	ldr pc, _software_interrupt
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	ldr pc, _not_used
	ldr pc, _irq
	ldr pc, _fiq

_jump_reset:
  .word _reset
_undefined_instruction:
	.word undefined_instruction
_software_interrupt:
	.word software_interrupt
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_not_used:
	.word not_used
_irq:
	.word irq
_fiq:
	.word fiq

//Not used exception handlers
undefined_instruction:
	b .

software_interrupt:
	b .

prefetch_abort:
	b .

data_abort:
	b .

not_used:
	b .

irq:
	ldr sp, _stack_irq_end
	sub sp, sp, #72
	stmia sp, {r0 - r12}
	add r8, sp, #60
	stmdb r8, {sp, lr}^
	str lr, [r8, #0]
	mrs r6, spsr
	str r6, [r8, #4]
	str r0, [r8, #8]
	mov r0, sp
	bl irq_handler
	ldmia sp, {r0 - lr}^
	mov r0, r0
	ldr lr, [sp, #60]
	add sp, sp, #72
	subs pc, lr, #4

fiq:
  b .

//The location of the stacks section
_stack_und_end:
	.long 0x81FB5000
_stack_abt_end:
	.long 0x81FC1800
_stack_irq_end:
	.long 0x81FDA800
_stack_fiq_end:
	.long 0x81FE7000
_stack_sys_end:
	.long 0x82000000
_stack_srv_end:
	.long 0x81FF3800
