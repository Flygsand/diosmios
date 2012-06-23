#ifndef _CARD_
#define _CARD_

#include "string.h"
#include "global.h"
#include "alloc.h"
#include "ff.h"
#include "vsprintf.h"
#include "HW.h"
#include "vsprintf.h"
#ifdef TRIFORCE
#include "JVSIOMessage.h"
#endif
#define		CARD_MAX_FILES	128

#define		CARD_BASE		0x00002F60

#define		CARD_CMD		(CARD_BASE+0x00)
#define		CARD_CMD_1		(CARD_BASE+0x04)
#define		CARD_CMD_2		(CARD_BASE+0x08)
#define		CARD_CMD_3		(CARD_BASE+0x0C)
#define		CARD_CMD_4		(CARD_BASE+0x10)
#define		CARD_RETURN		(CARD_BASE+0x14)
#define		CARD_CONTROL	(CARD_BASE+0x18)
#define		CARD_STATUS		(CARD_BASE+0x1C)

#define		CARD_SHADOW		(CARD_BASE + 0x20)

#define		CARD_SCMD		(CARD_SHADOW+0x00)
#define		CARD_SCMD_1		(CARD_SHADOW+0x04)
#define		CARD_SCMD_2		(CARD_SHADOW+0x08)
#define		CARD_SCMD_3		(CARD_SHADOW+0x0C)
#define		CARD_SCMD_4		(CARD_SHADOW+0x10)
#define		CARD_SRETURN	(CARD_SHADOW+0x14)
#define		CARD_SCONTROL	(CARD_SHADOW+0x18)
#define		CARD_SSTATUS	(CARD_SHADOW+0x1C)

// internal API command xfer bytes
#define CARD_XFER_CREATE        (2 * 8 * 1024)  // CARDCreate[Async]
#define CARD_XFER_DELETE        (2 * 8 * 1024)  // CARD[Fast]Delete[Async]
#define CARD_XFER_MOUNT         (5 * 8 * 1024)  // CARDMount[Async]
#define CARD_XFER_FORMAT        (5 * 8 * 1024)  // CARDFormat[Async]
#define CARD_XFER_RENAME        (1 * 8 * 1024)  // CARDRename[Async]
#define CARD_XFER_SETSTATUS     (1 * 8 * 1024)  // CARDSetStatus[Async]
#define CARD_XFER_SETATTRIBUTES (1 * 8 * 1024)  // CARDSetAttributes[Async]
#define CARD_XFER_WRITE         (1 * 8 * 1024)  // CARDWrite[Async]


#define		CARD_FILENAME_MAX		32

#define		CARD_ICON_MAX			8
#define		CARD_ICON_WIDTH			32
#define		CARD_ICON_HEIGHT		32
#define		CARD_BANNER_WIDTH		96
#define		CARD_BANNER_HEIGHT		32

#define		CARD_STAT_ICON_NONE		0
#define		CARD_STAT_ICON_C8		1
#define		CARD_STAT_ICON_RGB5A3	2

#define		CARD_STAT_BANNER_NONE	0
#define		CARD_STAT_BANNER_C8		1
#define		CARD_STAT_BANNER_RGB5A3	2

enum CardStatus
{
	CARD_SUCCESS		=   0,
	CARD_NO_FILE		=  -4,
	CARD_FILE_EXISTS	=  -7,
	CARD_FATAL_ERROR	=-128,
};

typedef struct CARDFileInfo
{
/* 0x00 */    s32 chan;
/* 0x04 */    s32 fileNo;

/* 0x08 */    s32 offset;
/* 0x0C */    s32 length;
/* 0x10 */    u16 iBlock;
} CARDFileInfo;

typedef struct CARDStat
{
    // read-only (Set by CARDGetStatus)
/* 0x00 */    char fileName[32];	
/* 0x20 */    u32  length;
/* 0x24 */    u32  time;           // seconds since 01/01/2000 midnight
/* 0x28 */    u8   gameName[4];
/* 0x2C */    u8   company[2];

    // read/write (Set by CARDGetStatus/CARDSetStatus)
/* 0x2E */    u8	bannerFormat;
/* 0x30 */    u32	iconAddr;      // offset to the banner, bannerTlut, icon, iconTlut data set.
/* 0x34 */    u16	iconFormat;
/* 0x36 */    u16	iconSpeed;
/* 0x38 */    u32	commentAddr;   // offset to the pair of 32 byte character strings.

    // read-only (Set by CARDGetStatus)
/* 0x3C */    u32  offsetBanner;
/* 0x40 */    u32  offsetBannerTlut;
/* 0x44 */    u32  offsetIcon[8];
/* 0x64 */    u32  offsetIconTlut;
/* 0x68 */    u32  offsetData;
} CARDStat;


void CardInit( void );
void CARDUpdateRegisters( void );
s32 CardFindFreeEntry( void );
s32 CardOpenFile( char *Filename, CARDFileInfo *CFInfo );
void CardCreateFile( char *Filename, u32 Size, CARDFileInfo *CFInfo );

#endif
