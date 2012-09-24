#ifndef _HW_
#define _HW_

#include "string.h"
#include "global.h"
#include "memory.h"

#include "alloc.h"
#include "dip.h"
#include "GCPad.h"

#define P2C(x)			((x)&0x7FFFFFFF)

#define HW_BASE			0x0d800000
#define HW_MEMIRR		(HW_BASE+0x60)
#define HW_AHBPROT		(HW_BASE+0x64)

#define HW_DDRCTRL_ADDR	(HW_BASE+0x74)
#define HW_DDRCTRL_VAL	(HW_BASE+0x76)

#define HW_GPIO_ENABLE	(HW_BASE+0xDC)
#define HW_GPIO_OUT		(HW_BASE+0xE0)
#define HW_GPIO_DIR		(HW_BASE+0xE4)
#define HW_GPIO_IN		(HW_BASE+0xE8)
#define HW_GPIO_INTLVL	(HW_BASE+0xEC)
#define HW_GPIO_INTFLAG	(HW_BASE+0xF0)
#define HW_GPIO_INTMASK	(HW_BASE+0xF4)
#define HW_GPIO_INMIR	(HW_BASE+0xF8)
#define HW_GPIO_OWNER	(HW_BASE+0xFC)

#define HW_ACRPLLSYS	(HW_BASE+0x1B0)
#define HW_ACRPLLSYSEXT	(HW_BASE+0x1B4)

#define DIFLAGS_PPCBOOT	(1<<20)


#define IRQ_TIMER	(1<<0)
#define IRQ_NAND	(1<<1)
#define IRQ_AES		(1<<2)
#define IRQ_SHA1	(1<<3)
#define IRQ_EHCI	(1<<4)
#define IRQ_OHCI0	(1<<5)
#define IRQ_OHCI1	(1<<6)
#define IRQ_SDHC	(1<<7)
#define IRQ_WIFI	(1<<8)
#define IRQ_GPIO1B	(1<<10)
#define IRQ_GPIO1	(1<<11)
#define IRQ_RESET	(1<<17)
#define IRQ_PPCIPC	(1<<30)
#define IRQ_IPC		(1<<31)

#define GPIO_POWER	(1<<1)
#define GPIO_EJECT	(1<<9)

extern void DRAMCTRLWrite( u32 Register, u32 Value );
extern u32 DRAMCTRLRead( u32 Register );

void EHCIInit( void );

void EXIControl( u32 value );

void MIOSHWInit( u32 A, u32 B );
void MIOSInit( void );
void MEMInitLow( void );
void BootPPC( void );
void UNKInit( u32 A, u32 B );
void DRAMInit( u32 A, u32 B );
void ChangeClock( void );
void PPCReset( void );
void HWResetDisable( void );
void HWResetEnable( void );
void HW_184( void );
void HW_184_2( void );

void GetRevision( u32 *Version, u32 *Revision );

void HWMAgic( u32 R0, u32 R1, u32 R2, u32 R3 );

u32 DRAMRead( u32 ValueA );
void DRAMWrite( u32 ValueA, u32 ValueB );

u32 RegRead( u32 Register );
void RegWrite( u32 Register, u32 Value );
void HWRegWriteBatch( u32 A, u32 B, u32 C, u32 D, u32 delay );

void Shutdown( void );

#endif
