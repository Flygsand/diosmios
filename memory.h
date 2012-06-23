#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "string.h"
#include "global.h"
#include "ipc.h"
#include "utils.h"
#include "hollywood.h"
#include "vsprintf.h"

#define ALIGN_FORWARD(x,align) \
	((typeof(x))((((u32)(x)) + (align) - 1) & (~(align-1))))

#define ALIGN_BACKWARD(x,align) \
	((typeof(x))(((u32)(x)) & (~(align-1))))

enum AHBDEV {
	AHB_STARLET = 0, //or MEM2 or some controller or bus or ??
	AHB_PPC = 1, //ppc or something else???
	AHB_NAND = 3,
	AHB_AES = 4,
	AHB_SHA1 = 5,
	AHB_EHCI = 6,
	AHB_SDHC = 9,
};

void dc_flushrange(const void *start, u32 size);
void dc_invalidaterange(void *start, u32 size);
void dc_flushall(void);
void ic_invalidateall(void);
void ahb_flush_from(enum AHBDEV dev);
void ahb_flush_to(enum AHBDEV dev);
u32 dma_addr(void *p);

static inline u32 get_cr(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c1, c0, 0" : "=r" (data) );
	return data;
}

static inline u32 get_ttbr(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c2, c0, 0" : "=r" (data) );
	return data;
}

static inline u32 get_dacr(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c3, c0, 0" : "=r" (data) );
	return data;
}

static inline void set_cr(u32 data)
{
	__asm__ volatile ( "mcr\tp15, 0, %0, c1, c0, 0" :: "r" (data) );
}

static inline void set_ttbr(u32 data)
{
	__asm__ volatile ( "mcr\tp15, 0, %0, c2, c0, 0" :: "r" (data) );
}

static inline void set_dacr(u32 data)
{
	__asm__ volatile ( "mcr\tp15, 0, %0, c3, c0, 0" :: "r" (data) );
}

static inline u32 get_dfsr(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c5, c0, 0" : "=r" (data) );
	return data;
}

static inline u32 get_ifsr(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c5, c0, 1" : "=r" (data) );
	return data;
}

static inline u32 get_far(void)
{
	u32 data;
	__asm__ volatile ( "mrc\tp15, 0, %0, c6, c0, 0" : "=r" (data) );
	return data;
}

void _ahb_flush_to(enum AHBDEV dev);

static inline void dc_inval_block_fast(void *block)
{
	__asm__ volatile ( "mcr\tp15, 0, %0, c7, c6, 1" :: "r" (block) );
	_ahb_flush_to(AHB_STARLET); //TODO: check if really needed and if not, remove
}

static inline void dc_flush_block_fast(void *block)
{
	__asm__ volatile ( "mcr\tp15, 0, %0, c7, c10, 1" :: "r" (block) );
	__asm__ volatile ( "mcr\tp15, 0, %0, c7, c10, 4" :: "r" (0) );
	ahb_flush_from(AHB_PPC); //TODO: check if really needed and if not, remove
}

#endif

