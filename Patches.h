#ifndef _PATCHES_
#define _PATCHES_

#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "vsprintf.h"
#include "HW.h"
#include "dol.h"


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

void PatchGCIPL( void );
void DoPatches( char *ptr, u32 size, u32 SectionOffset );
void DoCardPatches( char *ptr, u32 size, u32 SectionOffset );
void DoPatchesLoader( char *ptr, u32 size );

#endif