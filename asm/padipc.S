#include <asm.h>


PADRead:

	lis		%r4,	0xCC00
	
	lwz		%r5,	0x6428(%r4)
	lwz		%r0,	0x642C(%r4)

	lwz		%r0,	0x6404(%r4)
	lwz		%r4,	0x6408(%r4)
	
	lis		%r4,	0xC000
	stw		%r0,	0x12FC(%r4)
	stw		%r5,	0x12F8(%r4)

	blr
