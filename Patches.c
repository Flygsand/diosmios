#include "Patches.h"

#include "CardPatches.c"
#include "DVDPatches.c"
#include "FwritePatches.c"
#include "CheatCode.c"

extern u32 DOLSize;
u32 FrameBuffer	= 0;
u32 FBOffset	= 0;
u32 FBEnable	= 0;
u32	FBSize		= 0;

unsigned char VISetFB[] =
{
    0x38, 0x7F, 0x00, 0xF0,		//	mr      %r3, %r7
	0x38, 0x9F, 0x01, 0x24,
	0x38, 0xBF, 0x01, 0x28,
	0x38, 0xDF, 0x01, 0x3C, 
};

unsigned char OSReportDM[] =
{
	0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x04, 0x90, 0xE1, 0x00, 0x08, 0x3C, 0xE0, 0xC0, 0x00, 
	0x90, 0x67, 0x18, 0x60, 0x90, 0x87, 0x18, 0x64, 0x90, 0xA7, 0x18, 0x68, 0x90, 0xC7, 0x18, 0x6C, 
	0x90, 0xE7, 0x18, 0x70, 0x91, 0x07, 0x18, 0x74, 0x80, 0x07, 0x18, 0x60, 0x7C, 0x00, 0x18, 0x00, 
	0x41, 0x82, 0xFF, 0xF8, 0x80, 0xE1, 0x00, 0x08, 0x80, 0x01, 0x00, 0x04, 0x7C, 0x08, 0x03, 0xA6, 
	0x4E, 0x80, 0x00, 0x20, 
} ;
// Audio streaming replacement functions copied from Swiss r92
u32 __dvdLowAudioStatusNULL[17] = {
        // execute function(1); passed in on r4
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7C8903A6,     //  mtctr       r4
        0x3C80CC00,     //  lis         r4, 0xCC00
        0x2E830000,     //  cmpwi       cr5, r3, 0
        0x4196000C,     //  beq-        cr5, +0xC ?
        0x38600001,     //  li          r3, 1
        0x48000008,     //  b           +0x8 ?
        0x38600000,     //  li          r3, 0
        0x90646020,     //  stw         r3, 0x6020 (r4)
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

u32 __dvdLowAudioConfigNULL[10] = {
        // execute callback(1); passed in on r5 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CA903A6,     //  mtctr       r5
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

u32 __dvdLowReadAudioNULL[] = {
        // execute callback(1); passed in on r6 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CC903A6,     //  mtctr       r6
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctr;
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020
};

u32 __GXSetVAT_patch[31] = {
	/*0x8122ce00,*/ 0x39400000, 0x896904f2, 0x7d284b78,
	0x556007ff, 0x41820050, 0x38e00008, 0x3cc0cc01,
	0x98e68000, 0x61400070, 0x61440080, 0x61430090,
	0x98068000, 0x38000000, 0x80a8001c, 0x90a68000,
	0x98e68000, 0x98868000, 0x8088003c, 0x90868000,
	0x98e68000, 0x98668000, 0x8068005c, 0x90668000,
	0x98068000, 0x556bf87f, 0x394a0001, 0x39080004,
	0x4082ffa0, 0x38000000, 0x980904f2, 0x4e800020
};

u8 GXMObjects[][0x3C] =
{
	{	// GXPal528IntDf
		0x00, 0x00, 0x00, 0x04, 0x02, 0x80, 0x02, 0x10, 0x02, 0x10, 0x00, 0x28, 0x00, 0x17, 0x02, 0x80, 
		0x02, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x08, 0x08, 0x0A, 0x0C, 0x0A, 0x08, 0x08, 0x00, 0x00, 0x00, 
	},

	{	// GXEurgb60Hz480IntDf
		0x00, 0x00, 0x00, 0x14, 0x02, 0x80, 0x01, 0xE0, 0x01, 0xE0, 0x00, 0x28, 0x00, 0x00, 0x02, 0x80, 
		0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x08, 0x08, 0x0A, 0x0C, 0x0A, 0x08, 0x08, 0x00, 0x00, 0x00, 
	},

	{	// GXMpal480IntDf
		0x00, 0x00, 0x00, 0x08, 0x02, 0x80, 0x01, 0xE0, 0x01, 0xE0, 0x00, 0x28, 0x00, 0x00, 0x02, 0x80, 
		0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x08, 0x08, 0x0A, 0x0C, 0x0A, 0x08, 0x08, 0x00, 0x00, 0x00, 
	},

	{	// GXNtsc480IntDf
		0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x01, 0xE0, 0x01, 0xE0, 0x00, 0x28, 0x00, 0x00, 0x02, 0x80, 
		0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x08, 0x08, 0x0A, 0x0C, 0x0A, 0x08, 0x08, 0x00, 0x00, 0x00, 
	},

	{	// GXNtsc480Int
		0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x01, 0xE0, 0x01, 0xE0, 0x00, 0x28, 0x00, 0x00, 0x02, 0x80, 
		0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
		0x06, 0x06, 0x00, 0x00, 0x15, 0x16, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
};

u32 DVDGetDriveStatus[] = {
        0x38600000,     //  li		r3, 0
        0x4E800020
};

FuncPattern FPatterns[] =
{
	{ 0xCC,			17,     10,     5,      3,      2,	DVDInquiryAsync,			sizeof(DVDInquiryAsync),		"DVDInquiryAsync",				0,		0 },
	{ 0xC8,			16,     9,      5,      3,      3,	DVDSeekAbsAsyncPrio,		sizeof(DVDSeekAbsAsyncPrio),	"DVDSeekAbsAsyncPrio",			0,		0 },
	{ 0xA8,			10,     4,      4,      6,      3,	(u8*)DVDGetDriveStatus,		sizeof(DVDGetDriveStatus),		"DVDGetDriveStatus",			0,		0 },
	{ 0xD4,			13,     8,      11,     2,      7,	(u8*)NULL,					0xdead0004,						"AIResetStreamSampleCount",		0,		0 },

	{ 0x10C,        30,     18,     5,      2,      3,	(u8*)NULL,					0xdead0002,						"DVDLowRead A",					0,		0 },
	{ 0xDC,			23,     18,     3,      2,      4,	(u8*)NULL,					0xdead0002,						"DVDLowRead B",					0,		0 },
	{ 0x104,        29,     17,     5,      2,      3,	(u8*)NULL,					0xdead0002,						"DVDLowRead C",					0,		0 },

	
	{ 0xCC,			3,		3,		1,		0,		3,	(u8*)NULL,					0xdead000C,						"C_MTXPerspective",				0,		0 },

	{ 0x94, 		18, 	10, 	2, 		0, 		2,	(u8*)__dvdLowReadAudioNULL, sizeof(__dvdLowReadAudioNULL), "DVDLowReadAudio", 				0, 		0 },
	{ 0x88, 		18, 	8, 		2, 		0, 		2,	(u8*)__dvdLowAudioStatusNULL, sizeof(__dvdLowAudioStatusNULL), "DVDLowAudioStatus", 		0, 		0 },
	{ 0x98, 		19, 	8, 		2, 		1, 		3,	(u8*)__dvdLowAudioConfigNULL, sizeof(__dvdLowAudioConfigNULL), "DVDLowAudioConfig", 		0, 		0 },

	{ 0x308,        40,     18,     10,		23,		17,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite A",					1,		0 },
	{ 0x338,        48,     20,     10,		24,		16,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite B",					1,		0 },
	{ 0x2D8,        41,     17,     8,		21,		13,	patch_fwrite_GC,			sizeof(patch_fwrite_GC),		"__fwrite C",					1,		0 },
	
	{ 0x98, 		8, 		3, 		0, 		3,		5,	(u8*)NULL,					0xdead0001,						"__GXSetVAT",					0,		0 },

	{ 0x3A8,        86,     13,     27,     17,     24,	(u8*)NULL,					0xdead000B,						"PADRead A",					2,		0 },
	{ 0x2FC,        73,     8,      23,     16,     15,	(u8*)NULL,					0xdead000B,						"PADRead B",					2,		0 },
	{ 0x3B0,        87,     13,     27,     17,     25,	(u8*)NULL,					0xdead000B,						"PADRead C",					2,		0 },
	{ 0x334,        78,     7,      20,     17,     19,	(u8*)NULL,					0xdead000B,						"PADRead D",					2,		0 },
	{ 0x2A8,        66,     4,      20,     17,     14,	(u8*)NULL,					0xdead000B,						"PADRead E",					2,		0 },
};	

FuncPattern CPatterns[] =
{
	{ 0x14C,        28,     12,     7,      12,     4,	CARDFreeBlocks,		sizeof(CARDFreeBlocks),	"CARDFreeBlocks A",		1,		0 },
	{ 0x11C,        24,     10,     7,      10,     4,	CARDFreeBlocks,		sizeof(CARDFreeBlocks),	"CARDFreeBlocks B",		1,		0 },
	{ 0xC0,			22,     5,      2,      5,      10,	CARDGetSerialNo,	sizeof(CARDGetSerialNo),"CARDGetSerialNo",		0,		0 },
	{ 0x84,			12,     5,      3,      4,      2,	CARDGetEncoding,	sizeof(CARDGetEncoding),"CARDGetEncoding",		0,		0 },
	{ 0x80,			11,     5,      3,      4,      2,	CARDGetMemSize,		sizeof(CARDGetMemSize),	"CARDGetMemSize",		0,		0 },

	{ 0x94,			11,     6,      3,      5,      4,	__CARDSync,			sizeof(__CARDSync),		"__CARDSync",			0,		0 },
	{ 0x50,			6,      3,      2,      2,      2,	CARDCheck,			sizeof(CARDCheck),		"CARDCheck",			0,		0 },
	//{ 0x24,			4,      2,      1,      0,      2,	CARDCheckAsync,		sizeof(CARDCheckAsync),	"CARDCheckAsync",	0,		0 },
	{ 0x58C,        82,     11,     18,     41,     57,	CARDCheckEX,		sizeof(CARDCheckEX),	"CARDCheckExAsync",		0,		0 },
	{ 0x34,			4,      2,      1,      2,      2,	CARDProbe,			sizeof(CARDProbe),		"CARDProbe",			2,		0 },
	//{ 0x1C,			2,      2,      1,      0,      2,	CARDProbe,			sizeof(CARDProbe),		"CARDProbe B",			2,		0 },	//This is causing more trouble than a hack...
	{ 0x178,        20,     6,      6,      20,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx A",		3,		0 },
	{ 0x198,        22,     6,      5,      19,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx B",		3,		0 },
	{ 0x160,        17,     6,      5,      18,     4,	CARDProbeEX,		sizeof(CARDProbeEX),	"CARDProbeEx C",		3,		0 },
	{ 0x19C,        32,     14,     11,     12,     3,	CARDMountAsync,		sizeof(CARDMountAsync),	"CARDMountAsync A",		4,		0 },
	{ 0x184,        30,     14,     11,     10,     3,	CARDMountAsync,		sizeof(CARDMountAsync),	"CARDMountAsync B",		4,		0 },	
	{ 0xA8,			15,     8,      6,      3,      2,	CARDCheck,			sizeof(CARDCheck),		"CARDUnMount",			0,		0 },

	{ 0x174,        23,     6,      7,      14,     5,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen A",			5,		0 },
	{ 0x118,        14,     6,      6,      11,     4,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen B",			5,		0 },
	{ 0x170,        23,     6,      7,      14,     5,	CARDOpen,			sizeof(CARDOpen),		"CARDOpen C",			5,		0 },
	{ 0x15C,        27,     6,      5,      15,     6,	CARDFastOpen,		sizeof(CARDFastOpen),	"CARDFastOpen A",		11,		0 },
	{ 0x100,        20,     10,     4,      10,     4,	CARDFastOpen,		sizeof(CARDFastOpen),	"CARDFastOpen B",		11,		0 },
	{ 0x50,			8,      4,      2,      2,      3,	CARDClose,			sizeof(CARDClose),		"CARDClose",			0,		0 },
	{ 0x21C,        44,     6,      13,     19,     12,	CARDCreate,			sizeof(CARDCreate),		"CARDCreateAsync A",	6,		0 },
	{ 0x214,        42,     6,      13,     19,     12,	CARDCreate,			sizeof(CARDCreate),		"CARDCreateAsync B",	6,		0 },
	{ 0x10C,        25,     6,      9,      9,      5,	CARDDelete,			sizeof(CARDDelete),		"CARDDeleteAsync A",	12,		0 },
	{ 0x10C,        25,     6,      9,      9,      5,	CARDDelete,			sizeof(CARDDelete),		"CARDDeleteAsync C",	12,		0 },
	{ 0x128,        24,     7,      9,      12,     5,	CARDFastDelete,		sizeof(CARDFastDelete),	"CARDFastDelete",		0,		0 },
	{ 0x144,        27,     3,      8,      10,     9,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync A",		7,		0 },
	{ 0x140,        30,     7,      7,      10,     10,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync B",		7,		0 },
	{ 0x140,        27,     3,      8,      10,     9,	CARDRead,			sizeof(CARDRead),		"CARDReadAsync C",		7,		0 },
	{ 0x110,        24,     4,      8,      9,      6,	CARDWrite,			sizeof(CARDWrite),		"CARDWriteAsync A",		8,		0 },
	{ 0x10C,        23,     4,      8,      9,      6,	CARDWrite,			sizeof(CARDWrite),		"CARDWriteAsync B",		8,		0 },
	{ 0x1F8,        37,     3,      17,     18,     9,	CARDRename,			sizeof(CARDRename),		"CARDRenameAsync",		0,		0 },
	{ 0x128,        25,     9,      9,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus A",		9,		0 },
	{ 0x110,        25,     9,      8,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus B",		9,		0 },
	{ 0x124,        25,     9,      9,      6,      5,	CARDGetStats,		sizeof(CARDGetStats),	"CARDGetStatus C",		9,		0 },
	{ 0x170,        29,     9,      9,      12,     5,	CARDSetStats,		sizeof(CARDSetStats),	"CARDSetStatusAsync A",	10,		0 },
	{ 0x16C,        29,     9,      9,      12,     5,	CARDSetStats,		sizeof(CARDSetStats),	"CARDSetStatusAsync B",	10,		0 },
};

u32 CardLowestOff = 0;

u32 FB[MAX_FB];

void SMenuAddFramebuffer( void )
{
	u32 i,j,f;

	if( *(vu32*)FBEnable != 1 )
		return;

	FrameBuffer = (*(vu32*)FBOffset) & 0x7FFFFFFF;

	for( i=0; i < MAX_FB; i++)
	{
		if( FB[i] )	//add a new entry
			continue;
		
		//check if we already know this address
		f=0;
		for( j=0; j<i; ++j )
		{
			if( FrameBuffer == FB[j] )	// already known!
			{
				f=1;
				return;
			}
		}
		if( !f && FrameBuffer && FrameBuffer < 0x14000000 )	// add new entry
		{
			dbgprintf("ES:Added new FB[%d]:%08X\n", i, FrameBuffer );
			FB[i] = FrameBuffer;

			switch( *(vu32*)(FBEnable+0x20) )
			{
				case VI_NTSC:
					FBSize = 304*480*4;
					break;
				case VI_PAL:
					FBSize = 320*480*4;
					break;
				case VI_EUR60:
					FBSize = 320*480*4;
					break;
				default:
					dbgprintf("ES:SMenuFindOffsets():Invalid Video mode:%d\n", *(vu32*)(FBEnable+0x20) );
					break;
			}
		}
	}
}
void ScreenShot( void )
{
	if( *(vu32*)FBEnable != 1 )
		return;			
			
	set32( HW_GPIO_OUT, 1<<5 );

	char *str = (char*)malloc( 64 );

	u32 i=0,r,wrote;
	FIL SCRFile;
	
	sprintf( str, "/screenshots" );
	f_mkdir( str );

	do
	{
		sprintf( str, "/screenshots/scrn_%02X.raw", i++ );
		r = f_open( &SCRFile, str, FA_CREATE_NEW|FA_WRITE );
		if( r == FR_OK )
		{
			break;
		} else {
			if( r != FR_EXIST )
			{
				dbgprintf("Failed to create file:%u\n", r );				
				clear32( HW_GPIO_OUT, 1<<5 );
				return;
			}
		}
	} while(1);

	free(str);

	if( r == FR_OK )
	{
		f_write( &SCRFile, (void*)(FB[0]), FBSize, &wrote );
		f_close( &SCRFile );
	}

	clear32( HW_GPIO_OUT, 1<<5 );
}
void PatchB( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000000;
	write32( src, newval );	
}
void PatchBL( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000001;
	write32( src, newval );	
}
void PatchFunc( char *ptr )
{
	u32 i	= 0;
	u32 reg=-1;

	while(1)
	{
		u32 op = read32( (u32)ptr + i );

		if( op == 0x4E800020 )	// blr
			break;

		if( (op & 0xFC00FFFF) == 0x3C00CC00 )	// lis rX, 0xCC00
		{
			reg = (op & 0x3E00000) >> 21;
			
			write32( (u32)ptr + i, (reg<<21) | 0x3C00C000 );	// Patch to: lis rX, 0xC000
			dbgprintf("[%08X] %08X: lis r%u, 0xC000\n", (u32)ptr+i, read32( (u32)ptr+i), reg );
		}
		
		if( (op & 0xFC00FFFF) == 0x3C00A800 )	// lis rX, 0xA800
		{			
			write32( (u32)ptr + i, (op & 0x3E00000) | 0x3C00A700 );		// Patch to: lis rX, 0xA700
			dbgprintf("[%08X] %08X: lis rX, 0xA700\n", (u32)ptr+i, read32( (u32)ptr+i) );
		}

		if( (op & 0xFC00FFFF) == 0x38006000 )	// addi rX, rY, 0x6000
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;

			if( src == reg )
			{
				write32( (u32)ptr + i, (dst<<21) | (src<<16) | 0x38002F00 );	// Patch to: addi rX, rY, 0x2F00
				dbgprintf("[%08X] %08X: addi r%u, r%u, 0x2F00\n", (u32)ptr+i, read32( (u32)ptr+i), dst, src );

			}
		}

		if( (op & 0xFC000000 ) == 0x90000000 )
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			u32 val = op & 0xFFFF;
			
			if( src == reg )
			{
				if( (val & 0xFF00) == 0x6000 )	// case with 0x60XY(rZ)
				{
					write32( (u32)ptr + i,  (dst<<21) | (src<<16) | 0x2F00 | (val&0xFF) | 0x90000000 );	// Patch to: stw rX, 0x2FXY(rZ)
					dbgprintf("[%08X] %08X: stw r%u, 0x%04X(r%u)\n", (u32)ptr+i, read32( (u32)ptr+i), dst, 0x2F00 | (val&0xFF), src );
				}
			}
			
		}

		i += 4;
	}
}
void MPattern( u8 *Data, u32 Length, FuncPattern *FunctionPattern )
{
	u32 i;

	memset( FunctionPattern, 0, sizeof(FuncPattern) );

	for( i = 0; i < Length; i+=4 )
	{
		u32 word = read32( (u32)Data + i );
		
		if( (word & 0xFC000003) ==  0x48000001 )
			FunctionPattern->FCalls++;

		if( (word & 0xFC000003) ==  0x48000000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) ==  0x40800000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) ==  0x41800000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) ==  0x40810000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) ==  0x41820000 )
			FunctionPattern->Branch++;
		
		if( (word & 0xFC000000) ==  0x80000000 )
			FunctionPattern->Loads++;
		if( (word & 0xFF000000) ==  0x38000000 )
			FunctionPattern->Loads++;
		if( (word & 0xFF000000) ==  0x3C000000 )
			FunctionPattern->Loads++;
		
		if( (word & 0xFC000000) ==  0x90000000 )
			FunctionPattern->Stores++;
		if( (word & 0xFC000000) ==  0x94000000 )
			FunctionPattern->Stores++;

		if( (word & 0xFF000000) ==  0x7C000000 )
			FunctionPattern->Moves++;

		if( word == 0x4E800020 )
			break;
	}

	FunctionPattern->Length = i;
}
bool CPattern( FuncPattern *FPatA, FuncPattern *FPatB  )
{
	if( memcmp( FPatA, FPatB, sizeof(u32) * 6 ) == 0 )
		return true;
	else
		return false;
}
void DoCardPatches( char *ptr, u32 size, u32 SectionOffset )
{
	u32 i,j,k,offset,fail,FoundCardFuncStart=0;

	dbgprintf("DoCardPatches( 0x%p, %d, 0x%X)\n", ptr, size, SectionOffset );
		
	for( i=0; i < size; i+=4 )
	{
		if( read32( (u32)ptr + i) == 0x7C630214 && read32( (u32)ptr + i + 4) == 0x806300B8 )
		{
			dbgprintf("Found [CARDGetXferredBytes] @ 0x%08X\n", (u32)ptr + i - 12 );
			memcpy( ptr + i - 12, CARDGetXferredBytes, sizeof(CARDGetXferredBytes) );
		}

		if( read32( (u32)ptr + i ) != 0x7C0802A6 )	// MFLR
			continue;

		FuncPattern fp;
		MPattern( (u8*)(ptr+i), size, &fp );

		for( j=0; j < sizeof(CPatterns)/sizeof(FuncPattern); ++j )
		{
			if( CPatterns[j].PatchLength == 0 )
				continue;
			
			if( CPatterns[j].Found )				// Skip already found patches
				continue;

			if( CPattern( &fp, &(CPatterns[j]) ) )
			{
				if( CPatterns[j].Patch == CARDFreeBlocks )
				{
					if( CardLowestOff == 0 )
					{
						dbgprintf("CardLowestOff:0x%08X\n", i );
						CardLowestOff = i;
					}

					//Check for CARDGetResultCode which is always (when used) above CARDFreeBlocks
					if( read32( (u32)ptr + i - 0x30 ) == 0x2C030000 )
					{
						dbgprintf("Found [CARDGetResultCode] @ 0x%08X\n", (u32)ptr + i - 0x30 + SectionOffset );
						memcpy( ptr + i - 0x30, CARDGetResultCode, sizeof(CARDGetResultCode) );
					}

					FoundCardFuncStart = 1;
				}

				if( FoundCardFuncStart == 0 )
					continue;

				dbgprintf("Found [%s] @ 0x%08X\n", CPatterns[j].Name, (u32)ptr + i + SectionOffset );
				CPatterns[j].Found = (u32)ptr + i;

				// If this is a patch group set all others of this group as found aswell
				if( CPatterns[j].Group )
				{
					for( k=0; k < sizeof(CPatterns)/sizeof(FuncPattern); ++k )
					{
						if( CPatterns[k].Group == CPatterns[j].Group )
						{
							if( !CPatterns[k].Found )	//Don't overwrite the offset!
								CPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
							
							//dbgprintf("Setting [%s] to found!\n", CPatterns[k].Name );
						}
					}
				}

				//If by now no CARDProbe is found it won't be so set it to found to prevent CARDProbe B false hits
				if( CPatterns[j].Patch == CARDRead )
				{
					for( k=0; k < sizeof(CPatterns)/sizeof(FuncPattern); ++k )
					{
						if( CPatterns[k].Patch == CARDProbe )
						{
							if( !CPatterns[k].Found )	//Don't overwrite the offset!
								CPatterns[k].Found = -1;
						}
					}
				}


				if( strstr( CPatterns[j].Name, "Async" ) != NULL )
				{
					//dbgprintf("Async!\n");

					//Most games only use the normal functions so we patch a branch over to async and clear the CB
					//Find function call to our function

					offset = (u32)ptr + i;
					fail = 0;
					
					while(fail < 3)
					{
						//dbgprintf("[%08X] %08X %08X(%08X)\n", offset, read32( offset ) & 0xFC000003,read32( offset ) & 0x03FFFFFC ,(read32( offset ) & 0x03FFFFFC ) + offset);
						if( (read32( offset ) & 0xFC000003 ) == 0x48000001 )
						{
							if( (((read32( offset ) & 0x03FFFFFC ) + offset) & 0x03FFFFFC) == (u32)ptr+i ) 
								break;
						}

						if( read32( offset ) == 0x4E800020 )
							fail++;

						offset+=4;
					}

					if( fail < 3 )
					{
						dbgprintf("Found function call to [%s] @ 0x%08X\n", CPatterns[j].Name, offset + SectionOffset );
			
						//Now find function start 
						offset -= 4;
						while(1)
						{
							if( read32( offset ) == 0x7C0802A6 )
								break;

							offset-=4;
						}
					
						dbgprintf("Found function start of [%s(Sync)] @ 0x%08X\n", CPatterns[j].Name, offset + SectionOffset );

						//This patches a li rX, 0 before the Async function call for the Sync call
						//Since this register of the cb is different per function we do this haxx
						if( (read32( offset + 0x04 ) & 0x0000F000 ) == 0x00008000 )	// lis
						{
							write32( offset, read32( offset + 0x0C ) & 0xFBE00000 );
							offset += 4;

							if( CPatterns[j].Patch == CARDCheckEX )
							{
								write32( offset, 0x38800000 );	// lis r4,0
								offset += 4;
							}

							//Forge a branch to the async function

							u32 newval = ((u32)ptr + i) - offset;
							newval&= 0x03FFFFFC;
							newval|= 0x48000000;
							write32( offset, newval );
						} else {
							dbgprintf("Unhandled Async cb case!\n");
						}

					} else {
						dbgprintf("No sync function found!\n");
					}

					memcpy( ptr + i, CPatterns[j].Patch, CPatterns[j].PatchLength );

				} else {
					memcpy( ptr + i, CPatterns[j].Patch, CPatterns[j].PatchLength );					
				}
			}
		}
	}
	

	//if( CardLowestOff )
	//{
	//	for( j=0; j < sizeof(CPatterns)/sizeof(FuncPattern); ++j )
	//	{
	//		if( CPatterns[j].Found == 0 )
	//			dbgprintf("Pattern %s not found!\n", CPatterns[j].Name );
	//	}
	//}

	return;

}
void DoPatches( char *ptr, u32 size, u32 SectionOffset )
{
	u32 i=0,j=0,k=0,value;
	u32 PatchCount = 0;
	u32 r13  = 0;

	FBOffset = 0;
	FBEnable = 0;

	dbgprintf("DoPatches( 0x%p, %d, 0x%X)\n", ptr, size, SectionOffset );

	// HACK: PokemonXD and Pokemon Colosseum low memory clear patch
	if(( (read32(0)>>8) == 0x475858 ) || ( (read32(0)>>8) == 0x474336 ))
	{
		// patch out initial memset(0x1800, 0, 0x1800)
		if( (read32(0) & 0xFF) == 0x4A )	// JAP
			write32( 0x560C, 0x60000000 );
		else								// EUR/USA
			write32( 0x5614, 0x60000000 );

		// patch memset to jump to test function
		write32(0x00005498, 0x4BFFABF0);

		// patch in test < 0x3000 function
		write32(0x00000088, 0x3D008000);
		write32(0x0000008C, 0x61083000);
		write32(0x00000090, 0x7C044000);
		write32(0x00000094, 0x4180542C);
		write32(0x00000098, 0x90E40004);
		write32(0x0000009C, 0x48005400);

		// skips __start init of debugger mem
		write32(0x00003194, 0x48000028);
	}
	

	// Reset Found
	for( k=0; k < sizeof(FPatterns)/sizeof(FuncPattern); ++k )
		FPatterns[k].Found = 0;
	
	
	if( ConfigGetConfig(DML_CFG_NMM) )
		DoCardPatches( ptr, size, SectionOffset );
	
//Note: ORing the values prevents an early break out when a single patterns has multiple hits
	PatchCount=1;
	
	for( i=0; i < size; i+=4 )
	{
		if( (PatchCount & 2) == 0 )
		if( (read32( (u32)ptr + i )&0xFC00FFFF) == 0x5400077A &&
			(read32( (u32)ptr + i + 4 )&0xFC00FFFF) == 0x28000000 &&
			read32( (u32)ptr + i + 8 )	== 0x41820008 &&
			(read32( (u32)ptr + i +12 )&0xFC00FFFF)	== 0x64002000
			) 
		{
			dbgprintf("Patch:Found [__OSDispatchInterrupt]: 0x%08X 0x%08X\n", (u32)ptr + i + 0 + SectionOffset, (u32)ptr + i + 0x1A8 + SectionOffset );
				
			write32( (u32)ptr + i + 0,		(read32( (u32)ptr + i + 0 )	& 0xFFFF0000) | 0x0463 );
			write32( (u32)ptr + i + 0x1A8,	(read32( (u32)ptr + i + 0x1A8 )	& 0xFFFF0000) | 0x0463 );

			PatchCount |= 2;
		}

		if( (PatchCount & 4) == 0 )
		if( read32( (u32)ptr + i )		== 0x5480056A &&
			read32( (u32)ptr + i + 4 )	== 0x28000000 &&
			read32( (u32)ptr + i + 8 )	== 0x40820008 &&
			(read32( (u32)ptr + i +12 )&0xFC00FFFF) == 0x60000004
			) 
		{
			dbgprintf("Patch:Found [SetInterruptMask]: 0x%08X\n", (u32)ptr + i + 12 + SectionOffset );

			write32( (u32)ptr + i + 12, (read32( (u32)ptr + i + 12 ) & 0xFFFF0000) | 0x4000 );

			PatchCount |= 4;
		}

		if( (PatchCount & 8) == 0 )
		if( (read32( (u32)ptr + i + 0 ) & 0xFFFF) == 0x6000 &&
			(read32( (u32)ptr + i + 4 ) & 0xFFFF) == 0x002A &&
			(read32( (u32)ptr + i + 8 ) & 0xFFFF) == 0x0054 
			) 
		{
			u32 Offset = (u32)ptr + i - 8;

			dbgprintf("Patch:Found [__DVDIntrruptHandler]: 0x%08X\n", Offset + SectionOffset );
			
			if( (read32(Offset+4) & 0xFFFF) == 0xCC00 )	// Loader
			{
				value = *(vu32*)(Offset+4);
				value&= 0xFFFF0000;
				value|= 0x0000C000;
				*(vu32*)(Offset+4) = value;

			} else {

				value = *(vu32*)Offset;
				value&= 0xFFFF0000;
				value|= 0x0000C000;
				*(vu32*)Offset = value;
			}

			Offset += 8;

			value = *(vu32*)Offset;
			value&= 0xFFFF0000;
			value|= 0x00002F30;
			*(vu32*)Offset = value;
				
			Offset += 20;
				
			dbgprintf("Patch:[__DVDInterruptHandler] 0x%08X\n", Offset + SectionOffset );
			*(vu32*)Offset = 0x3D00CD00; Offset += 4;
			*(vu32*)Offset = 0x38000034; Offset += 4;
			*(vu32*)Offset = 0x90080004; Offset +=16;

			dbgprintf("Patch:[__DVDInterruptHandler] 0x%08X\n", Offset + SectionOffset );
			*(vu32*)Offset = 0x3D00CD00; Offset += 4;
			*(vu32*)Offset = 0x3C004000; Offset += 4;
			*(vu32*)Offset = 0x90080030; Offset +=32;
						
			if( (read32(Offset-8) & 0xFFFF) == 0xCC00 )	// Loader
			{
				Offset -= 8;
			}

			dbgprintf("Patch:[__DVDInterruptHandler] 0x%08X\n", Offset + SectionOffset );				

				value = *(vu32*)Offset;
				value&= 0xFFFF0000;
				value|= 0x0000C000;
			*(vu32*)Offset = value;

			Offset += 4;

				value = *(vu32*)Offset;
				value&= 0xFFFF0000;
				value|= 0x00002F08;
			*(vu32*)Offset = value;

			PatchCount |= 8;
		}

		if( (PatchCount & 16) == 0 )
		if( ConfigGetConfig(DML_CFG_CHEATS) || ConfigGetConfig( DML_CFG_DEBUGGER ) )
		{
			// OSSleepThread(Pattern 1)
			if( (PatchCount & 16) == 0 )
			if( read32((u32)ptr + i + 0 ) == 0x3C808000 &&
				( read32((u32)ptr + i + 4 ) == 0x38000004 || read32((u32)ptr + i + 4 ) == 0x808400E4 ) &&
				( read32((u32)ptr + i + 8 ) == 0x38000004 || read32((u32)ptr + i + 8 ) == 0x808400E4 )
				)
			{
				int j = 12;

				while( read32((u32)ptr + i + j ) != 0x4E800020 )
					j+=4;
			
				dbgprintf("Patch:[Hook:OSSleepThread] at 0x%08X\n", ((u32)ptr + i + j) | 0x80000000 );

				u32 DBGSize;
	
				//if( ConfigGetConfig( DML_CFG_DEBUGGER ) )
				//{
					memcpy( (void*)0x1800, kenobigcDBG, sizeof(kenobigcDBG) );
					DBGSize = sizeof(kenobigcDBG);
				//} else {
				//	memcpy( (void*)0x1800, kenobigc, sizeof(kenobigc) );
				//	DBGSize = sizeof(kenobigc);
				//}					

				if( ConfigGetConfig(DML_CFG_DEBUGWAIT) )
					write32( P2C(read32(0x1808)), 1 );
				else
					write32( P2C(read32(0x1808)), 0 );

				u32 newval = 0x18A8 - ((u32)ptr + i + j);
				newval&= 0x03FFFFFC;
				newval|= 0x48000000;
				write32( (u32)ptr + i + j, newval );

				memcpy( (void*)0x1800, (void*)0, 6 );

				char *path = (char*)malloc( 128 );

				if( ConfigGetConfig(DML_CFG_CHEAT_PATH) )
				{
					sprintf( path, "%s", ConfigGetCheatPath() );
				} else {
					sprintf( path, "/games/%.6s/%.6s.gct", (char*)0x1800, (char*)0x1800 );
				}

				FIL CodeFD;
				u32 read;

				if( f_open( &CodeFD, path, FA_OPEN_EXISTING|FA_READ ) == FR_OK )
				{
					if( CodeFD.fsize >= 0x2E80 - (0x1800+DBGSize-8) )
					{
						dbgprintf("Patch:Cheatfile is too large, it must not be large than %d bytes!\n",
							0x2E80 - (0x1800+DBGSize-8));
					} else {
						if( f_read( &CodeFD, (void*)(0x1800+DBGSize-8), CodeFD.fsize, &read ) == FR_OK )
						{
							dbgprintf("Patch:Copied cheat file to memory\n");
							write32( 0x1804, 1 );
						} else
							dbgprintf("Patch:Failed to read cheat file:\"%s\"\n", path );
					}

					f_close( &CodeFD );

				} else {
					dbgprintf("Patch:Failed to open/find cheat file:\"%s\"\n", path );
				}

				free(path);

				PatchCount |= 16;
			}
		}

		if( (PatchCount & 32) == 0 )
		{
			if( (read32( (u32)ptr + i + 0 ) & 0xFFFF) == 0xCC00 &&			// Game
				(read32( (u32)ptr + i + 4 ) & 0xFFFF) == 0x6000 &&
				(read32( (u32)ptr + i +12 ) & 0xFFFF) == 0x001C 
				) 
			{
				u32 Offset = (u32)ptr + i;

				dbgprintf("Patch:[cbForStateBusy] 0x%08X\n", Offset + SectionOffset );

				write32( Offset, 0x3C80C000 );
				write32( Offset+4, 0x38842F30 );
		
				PatchCount |= 32;

			} else if(	(read32( (u32)ptr + i + 0 ) & 0xFFFF) == 0xCC00 && // Loader
						(read32( (u32)ptr + i + 4 ) & 0xFFFF) == 0x6018 &&
						(read32( (u32)ptr + i +12 ) & 0xFFFF) == 0x001C 
				)
			{
				u32 Offset = (u32)ptr + i;

				dbgprintf("Patch:[cbForStateBusy] 0x%08X\n", Offset + SectionOffset );

				write32( Offset, 0x3C60C000 );
				write32( Offset+4, 0x80832F48 );
		
				PatchCount |= 32;
				
			}
		}

		if( (PatchCount & 64) == 0 )
		{
			if( read32( (u32)ptr + i + 0 ) == 0x3C608000 )
			{
				if( ((read32( (u32)ptr + i + 4 ) & 0xFC1FFFFF ) == 0x800300CC) && ((read32( (u32)ptr + i + 8 ) >> 24) == 0x54 ) )
				{
					dbgprintf( "Patch:[VIConfgiure] 0x%08X\n", (u32)(ptr+i) );
					write32( *(vu32*)(ptr+i+4), 0x5400F0BE | ((read32( (u32)ptr + i + 4 ) & 0x3E00000) >> 5	) );
					PatchCount |= 64;
				}
			}
		}

		if( (PatchCount & 128) == 0 )
		{
			if( ConfigGetConfig(DML_CFG_SCREENSHOT) )
			{
				if( *(u32*)(ptr+i) >> 16 == 0x3DA0 && r13 == 0 )
				{
					r13 = ((*(u32*)(ptr+i)) & 0xFFFF) << 16;
					r13|= (*(u32*)(ptr+i+4)) & 0xFFFF;

					dbgprintf("Patch:r13:%08X\n", r13 );
				}

				if( memcmp( ptr+i, VISetFB, sizeof(VISetFB) ) == 0 && FBEnable == 0 )
				{
					dbgprintf("Patch:[VISetFB]%08X\n", (u32)ptr+i );

					FBEnable = ( *(u32*)(ptr+i-4) );

					if( FBEnable & 0x8000 )
					{
						FBEnable = ((~FBEnable) & 0xFFFF) + 1;
						FBEnable = (r13 - FBEnable) & 0x7FFFFFF;
					} else {
						FBEnable = FBEnable & 0xFFFF;
						FBEnable = (r13 + FBEnable) & 0x7FFFFFF;
					}

					FBOffset = FBEnable - 0x08;
				//	dbgprintf("FBOffset:%08X\n", FBOffset );
				//	dbgprintf("FBEnable:%08X\n", FBEnable );
				
					for( j=0; j < MAX_FB; ++j )
						FB[j] = 0;

					PatchCount |= 128;
				}
			} else  {
				PatchCount |= 128;

			}
		}

		if( ConfigGetConfig(DML_CFG_CHEATS) || ConfigGetConfig( DML_CFG_DEBUGGER ) )
		{
			if( PatchCount == 255 )
				break;
		} else {
			if( PatchCount == 239 )
				break;
		}
	}

	if( ConfigGetVideMode() & DML_VID_FORCE )
	{
		k=0;
		for( i=0; i < size; i+=4 )
		{
			for( j=0; j <= GXNtsc480Int; ++j )
			{
				if( memcmp( ptr+i, GXMObjects[j], 0x3C ) == 0 )
				{
					dbgprintf("Patch:Found GX pattern %u at %08X\n", j, ptr+i );
					switch( ConfigGetVideMode() & 15 )
					{
						case DML_VID_FORCE_PAL50:
						{
							memcpy( ptr+i, GXMObjects[GXPal528IntDf], 0x3C );
						} break;
						case DML_VID_FORCE_PAL60:
						{
							memcpy( ptr+i, GXMObjects[GXEurgb60Hz480IntDf], 0x3C );
						} break;
						case DML_VID_FORCE_NTSC:
						{
							memcpy( ptr+i, GXMObjects[GXNtsc480IntDf], 0x3C );
						} break;
					}
					k++;
				}
			}

			if( k > 4 )
				break;
		}
	}
		
	for( i=0; i < size; i+=4 )
	{
		if( read32( (u32)ptr + i ) != 0x4E800020 )
			continue;

		i+=4;

		FuncPattern fp;
		MPattern( (u8*)(ptr+i), size, &fp );

		for( j=0; j < sizeof(FPatterns)/sizeof(FuncPattern); ++j )
		{
			if( FPatterns[j].Found ) //Skip already found patches
				continue;
				
			if( CPattern( &fp, &(FPatterns[j]) ) )
			{
				u32 FOffset = (u32)ptr + i;

				dbgprintf("Patch:Found [%s]: 0x%08X\n", FPatterns[j].Name, FOffset + SectionOffset );

				switch( FPatterns[j].PatchLength )
				{
					case 0xdead0001:	// Patch for __GXSetVAT, fixes the dungeon map freeze in Wind Waker
					{
						switch( read32(0) >> 8 )
						{
							case 0x505A4C:	// The Legend of Zelda: Collector's Edition
								if( !(DOLSize == 3847012 || DOLSize == 3803812) )	// only patch the main.dol of the Zelda:ww game 
									break;
							case 0x475A4C:	// The Legend of Zelda: The Wind Waker
							{
								write32(FOffset, (read32(FOffset) & 0xff00ffff) | 0x220000);
								memcpy((void *)(FOffset + 4), __GXSetVAT_patch, sizeof(__GXSetVAT_patch));

								dbgprintf("Patch:Applied __GXSetVAT patch\n");

							} break;
							default:
							break;
						}			
					} break;
					case 0xdead0002:	// DVDLowRead
					{
						PatchFunc( (char*)FOffset );
					} break;
					case 0xdead0004:	// Audiostreaming hack
					{
						switch( read32(0) >> 8 )
						{
							case 0x474544:	// Eternal Darkness
							break;
							default:
							{
								write32( FOffset + 0xB4, 0x60000000 );
								write32( FOffset + 0xC0, 0x60000000 );
							} break;
						}
					} break;
					case 0xdead000B:	//	PADRead hook
					{				
						if( !ConfigGetConfig(DML_CFG_PADHOOK) )
							break;

						//Find blr

						j=0;
						while(1)
						{
							if( read32( FOffset + j ) == 0x4E800020 )
								break;
							j+=4;
						}

						dbgprintf("Patch:[PADRead hook] %08X\n", FOffset + j );
				
						memcpy( (void*)0x2ECC, padipc, sizeof(padipc) );
						PatchB( 0x2ECC, FOffset + j );
						write32( 0x12FC, 0 );
	
					} break;
					case 0xdead000C:	// Widescreen hack by Extrems
					{
						if( !ConfigGetConfig(DML_CFG_FORCE_WIDE) )
							break;

						dbgprintf("Patch:[MTXPerspectiveSig] 0x%08X \n", (u32)FOffset);

						*(volatile float *)0x00000050 = 0.5625f;
						memcpy((void*)(FOffset+ 28),(void*)(FOffset+ 36),44);
						memcpy((void*)(FOffset+188),(void*)(FOffset+192),16);
						*(unsigned int*)(FOffset+52) = 0x48000001 | ((*(unsigned int*)(FOffset+52) & 0x3FFFFFC) + 8);
						*(unsigned int*)(FOffset+72) = 0x3C600000 | (0x80000050 >> 16);			// lis		3, 0x8180
						*(unsigned int*)(FOffset+76) = 0xC0230000 | (0x80000050 & 0xFFFF);	// lfs		1, -0x1C (3)
						*(unsigned int*)(FOffset+80) = 0xEC240072; // fmuls	1, 4, 1

					} break;
					default:
					{
						if( ConfigGetConfig( DML_CFG_CHEATS ) )
						{
							if( FPatterns[j].Patch == patch_fwrite_GC )
								break;
						}

						if( FPatterns[j].Patch == (u8*)DVDGetDriveStatus )
						{
							if( (read32(0) >> 8) != 0x474754 &&	// Chibi-Robo!
								(read32(0) >> 8) != 0x475041 )	// Pokémon Channel
								break;

							dbgprintf("Patch:DVDGetDriveStatus\n");
						}

						if( (FPatterns[j].Length >> 16) == 0xdead )
						{
							dbgprintf("DIP:Unhandled dead case:%08X\n", FPatterns[j].Length );
						} else
						{
							memcpy( (void*)(FOffset), FPatterns[j].Patch, FPatterns[j].PatchLength );
							
							if ((FPatterns[j].Patch == (u8 *)__dvdLowAudioStatusNULL) && ((read32(0) >> 8) == 0x47494B))
							{
								// Ikaruga resets to the main menu, if the returned status is 0(finished playing the stream), but it works if 1(still playing) is returned
								write32(FOffset + 36, 0x38600001);
								dbgprintf("Patch:LowAudioStatus patched for Ikaruga\n");
							}
						}

					} break;
				}

				// If this is a patch group set all others of this group as found aswell
				if( FPatterns[j].Group )
				{
					for( k=0; k < sizeof(FPatterns)/sizeof(FuncPattern); ++k )
					{
						if( FPatterns[k].Group == FPatterns[j].Group )
						{
							if( !FPatterns[k].Found )		// Don't overwrite the offset!
								FPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
							
							//dbgprintf("Setting [%s] to found!\n", FPatterns[k].Name );
						}
					}
				}
			}
		}
	}
}
