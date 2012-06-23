#include <asm.h>


PADRead:

	lis		%r4,	0xCC00
	lwz		%r0,	0x6404(%r4)
	lwz		%r4,	0x6408(%r4)
	lis		%r4,	0xC000
	stw		%r0,	0x12FC(%r4)
	blr
