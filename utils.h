#ifndef __UTILS_H__
#define __UTILS_H__

#include "global.h"

static inline u32 read32(u32 addr)
{
	u32 data;
	__asm__ volatile ("ldr\t%0, [%1]" : "=l" (data) : "l" (addr));
	return data;
}

static inline void write32(u32 addr, u32 data)
{
	__asm__ volatile ("str\t%0, [%1]" : : "l" (data), "l" (addr));
}

static inline u32 set32(u32 addr, u32 set)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\torr\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set)
	);
	return data;
}

static inline u32 clear32(u32 addr, u32 clear)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\tbic\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (clear)
	);
	return data;
}


static inline u32 mask32(u32 addr, u32 clear, u32 set)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\tbic\t%0, %3\n"
		"\torr\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set), "l" (clear)
	);
	return data;
}

static inline u16 read16(u32 addr)
{
	u32 data;
	__asm__ volatile ("ldrh\t%0, [%1]" : "=l" (data) : "l" (addr));
	return data;
}

static inline void write16(u32 addr, u16 data)
{
	__asm__ volatile ("strh\t%0, [%1]" : : "l" (data), "l" (addr));
}

static inline u16 set16(u32 addr, u16 set)
{
	u16 data;
	__asm__ volatile (
		"ldrh\t%0, [%1]\n"
		"\torr\t%0, %2\n"
		"\tstrh\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set)

	);
	return data;
}

static inline u16 clear16(u32 addr, u16 clear)
{
	u16 data;
	__asm__ volatile (
		"ldrh\t%0, [%1]\n"
		"\tbic\t%0, %2\n"
		"\tstrh\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (clear)
	);
	return data;
}


static inline u16 mask16(u32 addr, u16 clear, u16 set)
{
	u16 data;
	__asm__ volatile (
		"ldrh\t%0, [%1]\n"
		"\tbic\t%0, %3\n"
		"\torr\t%0, %2\n"
		"\tstrh\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set), "l" (clear)
	);
	return data;
}

static inline u8 read8(u32 addr)
{
	u32 data;
	__asm__ volatile ("ldrb\t%0, [%1]" : "=l" (data) : "l" (addr));
	return data;
}

static inline void write8(u32 addr, u8 data)
{
	__asm__ volatile ("strb\t%0, [%1]" : : "l" (data), "l" (addr));
}

static inline u8 set8(u32 addr, u8 set)
{
	u8 data;
	__asm__ volatile (
		"ldrb\t%0, [%1]\n"
		"\torr\t%0, %2\n"
		"\tstrb\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set)
	);
	return data;
}

static inline u8 clear8(u32 addr, u8 clear)
{
	u8 data;
	__asm__ volatile (
		"ldrb\t%0, [%1]\n"
		"\tbic\t%0, %2\n"
		"\tstrb\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (clear)
	);
	return data;
}

static inline u8 mask8(u32 addr, u8 clear, u8 set)
{
	u8 data;
	__asm__ volatile (
		"ldrb\t%0, [%1]\n"
		"\tbic\t%0, %3\n"
		"\torr\t%0, %2\n"
		"\tstrb\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set), "l" (clear)
	);
	return data;
}

/*
 * These functions are guaranteed to copy by reading from src and writing to dst in <n>-bit units
 * If size is not aligned, the remaining bytes are not copied
 */
extern void memset32(void *dst, u32 value, u32 size);
extern void memset16(void *dst, u16 value, u32 size);
extern void memset8(void *dst, u8 value, u32 size);

extern void memcpy(void *dst, void *src, u32 size);

//void udelay(u32 d);
void panic(u8 v);

static inline u32 get_cpsr(void)
{
	u32 data;
	__asm__ volatile ( "mrs\t%0, cpsr" : "=r" (data) );
	return data;
}

#define STACK_ALIGN(type, name, cnt, alignment)         \
u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
(((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
((sizeof(type)*(cnt))%(alignment))) : 0))]; \
type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
(u32)(_al__##name))&((alignment)-1))))

#define swab32(x) ((u32)(                                     \
                                   (((u32)(x) & (u32)0x000000ffUL) << 24) | \
                                   (((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
                                   (((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
                                   (((u32)(x) & (u32)0xff000000UL) >> 24)))
#define swab16(x) ((u16)(                                   \
                (((u16)(x) & (u16)0x00ffU) << 8) |          \
                (((u16)(x) & (u16)0xff00U) >> 8)))



#endif

