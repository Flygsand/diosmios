#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	off	
#	r5	cb
#   r6  prio

DVDSeekAbsAsyncPrio:

	stwu    %sp,	-0x10(%sp)
	mflr    %r0
	stw     %r0,	8(%sp)

#update dvdstruct

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
	stw     %r0,	0x1C(%r3)
	li		%r0,	2
	stw     %r0,	0x08(%r3)

	li		%r0,	0
	stw     %r0,	0x0C(%r3)

#off
	stw     %r4,	0x10(%r3)
#cb
	stw     %r5,	0x28(%r3)

	cmpwi	%r5,	0
	beq		skip_cb
	mtctr	%r5
	mr		%r4,	%r3
	li		%r3,	0
	bctrl	

skip_cb:

	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
