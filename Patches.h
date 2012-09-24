#ifndef _PATCHES_
#define _PATCHES_

#include "string.h"
#include "global.h"
#include "alloc.h"
#include "ff.h"
#include "vsprintf.h"
#include "HW.h"
#include "dol.h"
#include "Config.h"


typedef struct PatchInfo
{
	u8 *Signature;
	u8 *Mask;
	u32 Length;
	u32 FunctionLength;
	u8 *Patch;
	u32 PatchLength;
	char *Name;
} PatchInfo;

typedef struct FuncPattern
{
	u32 Length;
	u32 Loads;
	u32 Stores;
	u32 FCalls;
	u32 Branch;
	u32 Moves;
	u8 *Patch;
	u32 PatchLength;
	char *Name;
	u32 Group;
	u32 Found;
} FuncPattern;

typedef struct PatchCache
{
	u32 Offset;
	u32 PatchID;
	
} PatchCache;

typedef struct _gx_rmodeobj {
	u32 viTVMode;
	u16 fbWidth;
	u16 efbHeight;
	u16 xfbHeight;
	u16 viXOrigin;	
	u16 viYOrigin;	
	u16 viWidth;
	u16 viHeight;
	u32  xfbMode;
	u8  field_rendering;
	u8  aa;
	u8  sample_pattern[12][2];
	u8  vfilter[7];
} GXRModeObj;

#define GXPal528IntDf		0
#define GXEurgb60Hz480IntDf	1
#define GXMpal480IntDf		2
#define GXNtsc480IntDf		3
#define GXNtsc480Int		4


void PatchGCIPL( void );
void DoPatches( char *ptr, u32 size, u32 SectionOffset );
void DoCardPatches( char *ptr, u32 size, u32 SectionOffset );
void DoPatchesLoader( char *ptr, u32 size );

#endif