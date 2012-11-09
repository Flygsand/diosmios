#ifndef _CFG_
#define _CFG_

#include "string.h"
#include "global.h"
#include "alloc.h"
#include "vsprintf.h"
#include "HW.h"

#define	EXI_BASE	0x0D806800

typedef struct GC_SRAM 
{
/* 0x00 */	u16 CheckSum1;
/* 0x02 */	u16 CheckSum2;
/* 0x04 */	u32 ead0;
/* 0x08 */	u32 ead1;
/* 0x0C */	u32 CounterBias;
/* 0x10 */	u8	DisplayOffsetH;
/* 0x11 */	u8	BootMode;	// Bit 6 PAL60 flag
/* 0x12 */	u8	Language;
/* 0x13 */	u8	Flags;
		/*
			bit			desc			0		1
			0			-\_ Video mode
			1			-/
			2			Sound mode		Mono	Stereo
			3			always 1
			4			always 0
			5			always 1
			6			?
			7			Prog mode		off		on
		*/
/* 0x14 */	u8	FlashID[2*12];
/* 0x2C */	u32	WirelessKBID;
/* 0x30 */	u16	WirlessPADID[4];
/* 0x38 */	u8	LastDVDError;
/* 0x39 */	u8	Reserved;
/* 0x3A */	u16	FlashIDChecksum[2];
/* 0x3C */	u16	Unused;
} GC_SRAM;

typedef struct DML_CFG 
{
	u32		Magicbytes;		// 0xD1050CF6
	u32		Version;		// 0x00000002
	u32		VideoMode;
	u32		Config;
	char	GamePath[255];
	char	CheatPath[255];
} DML_CFG;

enum dmlconfig
{
	DML_CFG_CHEATS		= (1<<0),
	DML_CFG_DEBUGGER	= (1<<1),
	DML_CFG_DEBUGWAIT	= (1<<2),
	DML_CFG_NMM			= (1<<3),
	DML_CFG_NMM_DEBUG	= (1<<4),
	DML_CFG_GAME_PATH	= (1<<5),
	DML_CFG_CHEAT_PATH	= (1<<6),
	DML_CFG_ACTIVITY_LED= (1<<7),
	DML_CFG_PADHOOK		= (1<<8),
	DML_CFG_FORCE_WIDE	= (1<<9),
	DML_CFG_BOOT_DISC	= (1<<10),
	DML_CFG_BOOT_DISC2	= (1<<11),
	DML_CFG_NODISC		= (1<<12),
	DML_CFG_SCREENSHOT	= (1<<13),
};

enum dmlvideomode
{
	DML_VID_DML_AUTO	= (0<<16),
	DML_VID_FORCE		= (1<<16),
	DML_VID_NONE		= (2<<16),

	DML_VID_FORCE_PAL50	= (1<<0),
	DML_VID_FORCE_PAL60	= (1<<1),
	DML_VID_FORCE_NTSC	= (1<<2),
	DML_VID_FORCE_PROG	= (1<<3),
};

enum VideoModes
{
	GCVideoModeNone		= 0,
	GCVideoModePAL60	= 1,
	GCVideoModeNTSC		= 2,
	GCVideoModePROG		= 3,
};

void ConfigInit( DML_CFG *Cfg );
u32 ConfigGetConfig( u32 Config );
u32 ConfigGetVideMode( void );

char *ConfigGetGamePath( void );
char *ConfigGetCheatPath( void );

#endif