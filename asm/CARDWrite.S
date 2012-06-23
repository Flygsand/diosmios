#include <asm.h>

#
#	r3	file *
#	r4	buffer
#	r5	length
#	r6	offset
#	r7	cb

CardWrite:

	mflr    %r0
	stw     %r0, 4(%sp)
	stwu    %sp, -0x10(%sp)	
	
#Update fileinfo
	stw     %r5,	0x0C(%r3)
	stw     %r6,	0x08(%r3)

#send cmd to DM

	mr		%r12,	%r7

	li		%r0,	0
	lis		%r7,	0xC000
	addi	%r7,	%r7,	0x2F60
#IPC area
	dcbi	%r0,	%r7

	srwi	%r7,	%r5,	5
	mtctr	%r7
	mr		%r7,	%r4
invalidloop:
	dcbst	%r0,	%r7
	addi	%r7,	%r7,	0x20
	bdnz	invalidloop
	sc

	lis     %r7,	0xC000
	lis     %r0,	0xC800
	
	stw     %r0,	0x2F60(%r7)
	
	stw     %r4,	0x2F64(%r7)
	stw     %r5,	0x2F68(%r7)	

	stw     %r6,	0x2F6C(%r7)	

	lwz		%r0,	0x04(%r3)
	stw     %r0,	0x2F70(%r7)	

	li      %r0,	3
	stw     %r0,	0x2F78(%r7)
	
ready_loop:
	lwz		%r0,	0x2F78(%r7)
	cmpwi	%r0,	3
	beq		ready_loop

wait_loop:
	lwz		%r0,	0x2F9C(%r7)
	andi.	%r0,	%r0,	0x14
	cmpwi	%r0,	0
	beq		wait_loop
	
	cmpwi	%r12,	0
	beq		skip_cb
	mtctr	%r12
	li      %r3,	0
	li      %r4,	0
	bctrl

skip_cb:	
	li      %r3,	0
	mr		%r4,	%r3
	
	lwz     %r0,	0x14(%sp)
	addi    %sp,	%sp,	0x10
	mtlr    %r0
	blr
