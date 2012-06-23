#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	dst	
#	r5	cb

DVDInquiryAsync:

	stwu    %sp,	-0x10(%sp)
	mflr    %r0
	stw     %r0,	8(%sp)
	
#update dvdstruct

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
	stw     %r0,	0x0C(%r3)
#offset
	stw     %r0,	0x10(%r3)

	li		%r0,	2
	stw     %r0,	0x08(%r3)
	
	li		%r0,	0x20
#TransferSize
	stw     %r0,	0x1C(%r3)
	stw     %r0,	0x20(%r3)
#size
	stw     %r0,	0x14(%r3)

#ptr
	stw     %r4,	0x18(%r3)
#cb
	stw     %r5,	0x28(%r3)

#Inquiry reply

	li		%r0,	0
	dcbi	%r0,	%r4

	li		%r0,	0x0000
	sth		%r0,	0x02(%r4)
	
	cmpwi	%r5,	0
	beq		skip_cb
	mtctr	%r5
	mr		%r4,	%r3
	li      %r3,	0x20
	bctrl	

skip_cb:
	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
