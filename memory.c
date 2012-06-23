#include "memory.h"

void _dc_inval_entries(void *start, int count);
void _dc_flush_entries(const void *start, int count);
void _dc_flush(void);
void _ic_inval(void);
void _drain_write_buffer(void);
u32 irq_kill(void);
void irq_restore(u32 cookie);

#define LINESIZE 0x20
#define CACHESIZE 0x4000


#define CR_MMU		(1 << 0)
#define CR_DCACHE	(1 << 2)
#define CR_ICACHE	(1 << 12)

// TODO: move to hollywood.h once we figure out WTF
#define		HW_100	(HW_REG_BASE + 0x100)
#define		HW_104	(HW_REG_BASE + 0x104)
#define		HW_108	(HW_REG_BASE + 0x108)
#define		HW_10c	(HW_REG_BASE + 0x10c)
#define		HW_110	(HW_REG_BASE + 0x110)
#define		HW_114	(HW_REG_BASE + 0x114)
#define		HW_118	(HW_REG_BASE + 0x118)
#define		HW_11c	(HW_REG_BASE + 0x11c)
#define		HW_120	(HW_REG_BASE + 0x120)
#define		HW_124	(HW_REG_BASE + 0x124)
#define		HW_130	(HW_REG_BASE + 0x130)
#define		HW_134	(HW_REG_BASE + 0x134)
#define		HW_138	(HW_REG_BASE + 0x138)
#define		HW_188	(HW_REG_BASE + 0x188)
#define		HW_18C	(HW_REG_BASE + 0x18c)

// what is this thing doing anyway?
// and why only on reads?
u32 _mc_read32(u32 addr)
{
	u32 data;
	u32 tmp130 = 0;
	// this seems to be a bug workaround
	if(!(read32(HW_VERSION) & 0xF0))
	{
		tmp130 = read32(HW_130);
		write32(HW_130, tmp130 | 0x400);
		// Dummy reads?
		read32(HW_138);
		read32(HW_138);
		read32(HW_138);
		read32(HW_138);
	}
	data = read32(addr);
	read32(HW_VERSION); //???

	if(!(read32(HW_VERSION) & 0xF0))
		write32(HW_130, tmp130);

	return data;
}

// this is ripped from IOS, because no one can figure out just WTF this thing is doing
void _ahb_flush_to(enum AHBDEV dev) {
	u32 mask = 10;
	switch(dev) {
		case AHB_STARLET: mask = 0x8000; break;
		case AHB_PPC: mask = 0x4000; break;
		//case 2: mask = 0x0001; break;
		case AHB_NAND: mask = 0x0002; break;
		case AHB_AES: mask = 0x0004; break;
		case AHB_SHA1: mask = 0x0008; break;
		case AHB_EHCI: mask = 0x0010; break;
		//case 7: mask = 0x0020; break;
		//case 8: mask = 0x0040; break;
		case AHB_SDHC: mask = 0x0080; break;
		//case 10: mask = 0x0100; break;
		//case 11: mask = 0x1000; break;
		//case 12: mask = 0x0000; break;
		default:
			dbgprintf("ahb_invalidate(%d): Invalid device\n", dev);
			return;
	}
	//NOTE: 0xd8b000x, not 0xd8b400x!
	u32 val = _mc_read32(0xd8b0008);
	if(!(val & mask)) {
		switch(dev) {
			// 2 to 10 in IOS, add more
			case AHB_NAND:
			case AHB_AES:
			case AHB_SHA1:
			case AHB_EHCI:
			case AHB_SDHC:
				while((read32(HW_18C) & 0xF) == 9)
					set32(HW_188, 0x10000);
				clear32(HW_188, 0x10000);
				set32(HW_188, 0x2000000);
				mask32(HW_124, 0x7c0, 0x280);
				set32(HW_134, 0x400);
				while((read32(HW_18C) & 0xF) != 9);
				set32(HW_100, 0x400);
				set32(HW_104, 0x400);
				set32(HW_108, 0x400);
				set32(HW_10c, 0x400);
				set32(HW_110, 0x400);
				set32(HW_114, 0x400);
				set32(HW_118, 0x400);
				set32(HW_11c, 0x400);
				set32(HW_120, 0x400);
				write32(0xd8b0008, _mc_read32(0xd8b0008) & (~mask));
				write32(0xd8b0008, _mc_read32(0xd8b0008) | mask);
				clear32(HW_134, 0x400);
				clear32(HW_100, 0x400);
				clear32(HW_104, 0x400);
				clear32(HW_108, 0x400);
				clear32(HW_10c, 0x400);
				clear32(HW_110, 0x400);
				clear32(HW_114, 0x400);
				clear32(HW_118, 0x400);
				clear32(HW_11c, 0x400);
				clear32(HW_120, 0x400);
				clear32(HW_188, 0x2000000);
				mask32(HW_124, 0x7c0, 0xc0);
			//0, 1, 11 in IOS, add more
			case AHB_STARLET:
			case AHB_PPC:
				write32(0xd8b0008, val & (~mask));
				// wtfux
				write32(0xd8b0008, val | mask);
				write32(0xd8b0008, val | mask);
				write32(0xd8b0008, val | mask);
		}
	}
}

// invalidate device and then starlet
void ahb_flush_to(enum AHBDEV type)
{
	u32 cookie = irq_kill();
	_ahb_flush_to(type);
	if(type != AHB_STARLET)
		_ahb_flush_to(AHB_STARLET);

	irq_restore(cookie);
}

// flush device and also invalidate memory
void ahb_flush_from(enum AHBDEV dev)
{
	u32 cookie = irq_kill();
	u16 req = 0;
	u16 ack;
	int i;

	switch(dev)
	{
		case AHB_STARLET:
		case AHB_PPC:
			req = 1;
			break;
		case AHB_AES:
		case AHB_SHA1:
			req = 2;
			break;
		case AHB_NAND:
		case AHB_SDHC:
			req = 8;
			break;
		case AHB_EHCI:
			req = 4;
			break;
		default:
			dbgprintf("ahb_flush(%d): Invalid device\n", dev);
			goto done;
	}

	write16(MEM_FLUSHREQ, req);

	for(i=0;i<1000000;i++) {
		ack = read16(MEM_FLUSHACK);
		_ahb_flush_to(AHB_STARLET);
		if(ack == req)
			break;
	}
	write16(MEM_FLUSHREQ, 0);
	if(i>=1000000) {
		dbgprintf("ahb_flush(%d): Flush (0x%x) did not ack!\n", dev, req);
	}
done:
	irq_restore(cookie);
}

void dc_flushrange(const void *start, u32 size)
{
	u32 cookie = irq_kill();
	if(size > 0x4000) {
		_dc_flush();
	} else {
		void *end = ALIGN_FORWARD(((u8*)start) + size, LINESIZE);
		start = ALIGN_BACKWARD(start, LINESIZE);
		_dc_flush_entries(start, (end - start) / LINESIZE);
	}
	_drain_write_buffer();
	ahb_flush_from(AHB_PPC);
	irq_restore(cookie);
}

void dc_invalidaterange(void *start, u32 size)
{
	u32 cookie = irq_kill();
	void *end = ALIGN_FORWARD(((u8*)start) + size, LINESIZE);
	start = ALIGN_BACKWARD(start, LINESIZE);
	_dc_inval_entries(start, (end - start) / LINESIZE);
	ahb_flush_to(AHB_STARLET);
	irq_restore(cookie);
}

void dc_flushall(void)
{
	u32 cookie = irq_kill();
	_dc_flush();
	_drain_write_buffer();
	ahb_flush_from(AHB_PPC);
	irq_restore(cookie);
}

void ic_invalidateall(void)
{
	u32 cookie = irq_kill();
	_ic_inval();
	ahb_flush_to(AHB_STARLET);
	irq_restore(cookie);
}

u32 dma_addr(void *p)
{
	u32 addr = (u32)p;

	switch(addr>>20) {
		case 0xfff:
		case 0x0d4:
		case 0x0dc:
			if(read32(HW_MEMMIRR) & 0x20) {
				addr ^= 0x10000;
			}
			addr &= 0x0001FFFF;
			addr |= 0x0d400000;
			break;
	}
	dbgprintf("DMA to %p: address %08x\n", p, addr);
	return addr;
}
