.global _start
.global start

.global udelay

.global RegWrite
.global RegRead

.global DRAMWrite
.global DRAMRead

.global DRAMCTRLRead
.global DRAMCTRLWrite

.extern main
.extern Syscall
.extern SWI
.extern PrefetchAbort
.extern DataAbort
.extern IRQHandler
.extern FIQHandler
.arm

.section ".init"
_start:
	ldr pc, =start
	ldr pc, =Syscall
	ldr pc, =SWI
	ldr pc, =PrefetchAbort
	ldr pc, =DataAbort
	ldr pc, =0
	ldr pc, =IRQHandler
	movs pc, lr

start:
	mov     r0, #0
	ldr     r1, =0xd80003c
	str     r0, [r1]

	mov     r0, #0
	mcr     p15, 0, r0,c7,c5
	mcr     p15, 0, r0,c7,c6

	mrc     p15, 0, r0,c1,c0
	bic     r0, r0, #4
	bic     r0, r0, #0x1000
	mcr     p15, 0, r0,c1,c0
	
	ldr		r0,=__bss_start
	ldr		r1,=__bss_end
	mov		r2,#0
	mov		r3,#4
clearbss:
	cmp r0, r1
	bcs clearbss_end
	str r2, [r0], r3
	b clearbss

clearbss_end:
	
	msr     CPSR_c, #211
	ldr     sp, =0xFFFF7E60
	msr     CPSR_c, #210
	ldr     sp, =0xFFFF7E60
	msr     CPSR_c, #209
	ldr     sp, =0xFFFF7E60
	msr     CPSR_c, #215
	ldr     sp, =0xFFFF7E60
	msr     CPSR_c, #219
	ldr     sp, =0xFFFF7E60
	msr     CPSR_c, #31
	ldr     sp, =0xFFFE4000
	
#enable IRQs

	mrs		r1,		cpsr
	bic		r1,		r1,		#0xC0
	msr		cpsr_c,	r1

	mov lr, pc
	ldr pc, =main
	
RegWrite:
	ldr r3,=0xd8b4000
	lsl r0, r0, #1
	add r0, r0, r3
	strh r1, [r0]
	bx lr

RegRead:
	ldr r3,=0xd8b4000
	lsl r0, r0, #1
	add r0, r0, r3
	ldrh r0, [r0]
	lsl r0, r0, #0x10
	lsr r0, r0, #0x10
	bx  lr
	
DRAMWrite:
	push {r4,lr}
	add r3, r0, #0
	add r4, r1, #0
	mov r0, #0x3a
	add r1, r3, #0
	bl	RegWrite

	mov r0, #0x3a
	bl	RegRead

	mov r0, #0x3b
	add r1, r4, #0
	bl	RegWrite

	pop {r4}
	pop {r0}
	bx r0

DRAMRead:
	push {lr}
	add r1, r0, #0
	mov r0, #0x3a
	bl	RegWrite
	
	mov r0, #0x3a
	bl	RegRead

	mov r0, #0x3b
	bl	RegRead

	pop {r1}
	bx r1

DRAMCTRLWrite:
	push {r4,r5,lr}
	ldr r4, =0x163
	add r3, r0, #0
	add r5, r1, #0
	add r0, r4, #0
	add r1, r3, #0
	bl DRAMWrite

	add r0, r4, #0
	bl DRAMRead

	mov r0, #0xb1
	add r1, r5, #0
	lsl r0, r0, #1
	bl DRAMWrite

	pop {r4,r5}
	pop {r0}
	bx r0

DRAMCTRLRead:
	push {r4,lr}
	ldr r4, =0x163
	add r1, r0, #0
	add r0, r4, #0
	bl DRAMWrite

	add r0, r4, #0
	bl DRAMRead
	
	ldr r0, =0x162
	bl DRAMRead

	pop {r4}
	pop {r1}
	bx  r1

udelay:

	push    {lr}

	lsr     r3, r0, #2
	add     r3, r3, r0
	lsr     r2, r0, #6

	add     r0, r3, r2
	cmp     r0, #1
	bhi     loc_ffff20e8

	mov     r0, #2

loc_ffff20e8:
	ldr     r1, =0xd800010
	ldr     r3, [r1]
	add     r2, r3, r0
	cmp     r2, r3
	bls     loc_ffff211c

loc_ffff20f2:
	ldr     r3, [r1]
	cmp     r3, r2
	bcc     loc_ffff20f2

loc_ffff20f8:
	pop     {r0}
	bx      r0

loc_ffff211c:
	add     r3, r1, #0

loc_ffff211e:
	ldr     r0, [r3]
	cmp     r0, #0
	blt     loc_ffff211e

	cmp     r0, r2
	bcc     loc_ffff211e
	b       loc_ffff20f8

.end