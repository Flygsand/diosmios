#include <asm.h>

#
#	r3	chan
#	r4	fileName
#	r5	size
#	r6	fileInfo
#	r7	cb

CARDCreate:

	cmpwi	%r3, 0
	beq		DoCode
	li		%r3,	-3
	li		%r4,	-3
	blr

DoCode:

	mflr    %r0
	stw     %r0, 4(%sp)
	stwu    %sp, -0x28(%sp)
	stmw    %r27, 0x14(%sp)
				
#send cmd to DM

	mr		%r12,	%r7

	li		%r0,	0
	lis		%r7,	0xC000
	addi	%r7,	%r7,	0x2F60
#IPC area
	dcbi	%r0,	%r7
	addi	%r7,	%r7,	0x20
	dcbi	%r0,	%r7

	lis     %r7,	0xC000
	lis     %r0,	0xC200
	
	stw     %r0,	0x2F60(%r7)
	stw     %r4,	0x2F64(%r7)
	stw     %r5,	0x2F68(%r7)	
	stw     %r6,	0x2F6C(%r7)	

#cache workaround for the filename
	li		%r5,	8
	mtctr	%r5
	lis     %r5,	0xC000
invalidloop:
	lwz		%r0,	0(%r4)
	stw		%r0,	0x17E0(%r5)
	addi	%r4,	%r4,	4
	addi	%r5,	%r5,	4
	bdnz	invalidloop

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
	
	lwz     %r0,	0x2F90(%r7)
	stw     %r0,	0x04(%r6)
	
	cmpwi	%r12,	0
	beq		skip_cb
	mtlr	%r12
	li      %r3,	0
	li      %r4,	0
	blrl

skip_cb:	
	lis		%r7,	0xC000
	lwz     %r3,	0x2F94(%r7)
	mr		%r4,	%r3
	
	lmw		%r27,	0x14(%sp)
	lwz		%r0,	0x2C(%sp)
	addi	%sp,	%sp,	0x28
	mtlr	%r0
	blr

