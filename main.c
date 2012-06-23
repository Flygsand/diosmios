/*

DIOS MIOS - Gamecube USB loader for Nintendo Wii

Copyright (C) 2010-2012  crediar

*/
#include "string.h"
#include "global.h"
#include "ipc.h"
#include "alloc.h"
#include "ff.h"
#include "diskio.h"
#include "dol.h"
#include "GCPad.h"
#include "HW.h"
#include "Patches.h"
#include "Config.h"
#include "Card.h"
#include "DVD.h"
#include "Drive.h"
#include "dip.h"

char __aeabi_unwind_cpp_pr0[0];

void Syscall( u32 a, u32 b, u32 c, u32 d )
{
	dbgprintf("Syscall,%d,%d,%d,%d\n", a, b, c, d);
	return;
}
void SWI( u32 a, u32 b )
{
	dbgprintf("SWI:%X,%X\n", a, b );
	return;
}
void PrefetchAbort( void )
{
	u32 val;
	__asm("mov	%0,lr": "=r" (val) );
	
	*(vu32*)0xD800070 |= 1;

	dbgprintf("PrefetchAbort LR:%08x\n", val-8 );
	while(1);
	return;
}
void DataAbort( u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h )
{
	u32 val;

	__asm("mov	%0,lr": "=r" (val) );

	*(vu32*)0xD800070 |= 1;
	
	dbgprintf("DataAbort: LR:%08x, %x, %x, %x, %x, %x, %x, %x\n",val-8,b,c,d,e,f,g,h);
	Shutdown();
}
void IRQHandler( void )
{
	u32 IRQs = read32(HW_ARMIRQFLAG) /*& read32(HW_ARMIRQMASK)*/;

	if( IRQs & IRQ_GPIO1 )	// Starlet GPIO IRQ
	{
		if( read32(HW_GPIO_INTFLAG) & (1) )
		{
			set32( HW_EXICTRL, 1 );

			int i;
			for( i = 0; i < 0x20; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( CARD_BASE + i ), read32( CARD_SHADOW + i ) );
			dbgprintf("\n");
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", i, read32( 0x00002F00 + i ), read32( 0x00002F30 + i ) );
			dbgprintf("\n");
			
			for( i = 0; i < 0x30; i+=4 )
				dbgprintf("0x%08X:0x%08X\t0x%08X\n", 0x0D806000 + i, read32( 0x0D806000 + i ), read32( 0x0D006000 + i ) );
						
			dbgprintf("DVD:Error:%08X\n", DVDLowGetError() );

			udelay(10000);

			set32( HW_GPIO_ENABLE, GPIO_POWER );
			set32( HW_GPIO_OUT, GPIO_POWER );

			while(1);
		}
	} else if( IRQs & IRQ_RESET )
	{
		;
	} else {
		
		set32( HW_EXICTRL, 1 );

		udelay(1000);
		dbgprintf("IRQ:%08X %08X\n", read32(HW_ARMIRQFLAG), read32(HW_GPIO_INTFLAG) );
		set32( HW_EXICTRL, 0 );
	}

	return;
}
void FIQHandler( void )
{
	//dbgprintf("FIQHandler\n");
	return;
}
void DebugPoke( u8 Value )
{
	clear32( 0xD8000E0, 0xFF0000 );
	set32( 0xD8000E0, Value<<16 );
}
void SysReset( void )
{
	write32( HW_RESETS, (read32( HW_RESETS ) | 0x20 ) & (~1) );
}
void SysShutdown( void )
{
	set32( HW_GPIO_ENABLE, GPIO_POWER );
	set32( HW_GPIO_OUT, GPIO_POWER );

	while(1);
}

u32 fail;
FIL Log;

int main( int argc, char *argv[] )
{
	udelay(800);

#ifndef REALNAND	
	PPCReset();
	clear32( HW_RESETS, 0x48000 );
	clear32( 0xD800184, 0x438E );
	
	ChangeClock();

	DRAMInit(1,0);

	set32( HW_RESETS, 0x48000 );
	set32( 0xD800184, 0x438E );

	UNKInit( 1, 1 );
#endif

	set32( 0xD800038, IRQ_RESET|IRQ_GPIO1 );
	set32( 0xD80003C, IRQ_RESET|IRQ_GPIO1 );
	udelay(200);

	u32 SP[2];
	GetRevision( SP+1, SP );
	if( SP[1] == 0 )
	{
		write32( HW_MEMIRR, 0x67 );
	} else {
		write32( HW_MEMIRR, 0x27 );
	}

	MIOSInit();

#ifdef DEBUG
	dbgprintf("DIOS-MIOS [DEBUG] v%d.%d\n", DM_VERSION>>16, DM_VERSION & 0xFFFF );
#else
#ifdef REALNAND
	dbgprintf("DIOS-MIOS v%d.%db\n", DM_VERSION>>16, DM_VERSION & 0xFFFF );
#else
	dbgprintf("DIOS-MIOS v%d.%da\n", DM_VERSION>>16, DM_VERSION & 0xFFFF );
#endif
#endif
	dbgprintf("Built: " __DATE__ " " __TIME__ "\n");
	dbgprintf("This software is licensed under GPLv3, for more details visit:\nhttp://code.google.com/p/diosmios\n");

	//dbgprintf("CPU Ver:%d.%d\n", SP[1], SP[0] );
	
	//dbgprintf("MEMInitLow()...\n");
	MEMInitLow();
	
//	EHCI

	*(vu32*)0x0D0400A4 = 0x00004026;
	*(vu32*)0x0D0400B0 = 0x0002422E;
	*(vu32*)0x0D0400B4 = 0x03802E14;

// DDR control

	*(vu16*)0x0D8B4034 = 0x0000;
	*(vu16*)0x0D8B403C = 0x0000;
	*(vu16*)0x0D8B4034 = 0x0000;
	*(vu16*)0x0D8B403C = 0x0000;
	*(vu16*)0x0D8B4040 = 0x0000;
	*(vu16*)0x0D8B4044 = 0x0000;
	*(vu16*)0x0D8B4048 = 0x0000;
	*(vu16*)0x0D8B404C = 0x0000;
	*(vu16*)0x0D8B4050 = 0x0000;
	*(vu16*)0x0D8B4054 = 0x13EB;
	*(vu16*)0x0D8B4058 = 0x09B5;
	*(vu16*)0x0D8B4060 = 0x0000;
	*(vu16*)0x0D8B4064 = 0x0000;
	*(vu16*)0x0D8B420C = 0x3620;
	*(vu16*)0x0D8B4220 = 0xF000;

	udelay(8000);
	
	HeapInit( (u8*)0x13600000 );
	
	set32( HW_EXICTRL, 1 );

	DVDInit();

	ConfigInit( (DML_CFG*)0x01200000 );

	if( !ConfigGetConfig(DML_CFG_BOOT_DISC) )
	{
		if( DVDSelectGame() == DI_SUCCESS )
		{
			if( ConfigGetConfig(DML_CFG_NMM) )
				CardInit();
		} else {
			dbgprintf("Loading disc\n");
			
			((DML_CFG*)0x01200000)->Version		= CONFIG_VERSION;
			((DML_CFG*)0x01200000)->Magicbytes	= 0xD1050CF6;
			((DML_CFG*)0x01200000)->VideoMode	= DML_VID_DML_AUTO;
			((DML_CFG*)0x01200000)->Config		= DML_CFG_BOOT_DISC;
		}
	}

	DIInit();
	
//Switch mem2 to ARAM
	DRAMCTRLWrite( 0x49, 0x0E );
	udelay(2);

	DRAMCTRLWrite( 0x49, 0x0F );
	udelay(2);

	HeapInit( (u8*)0xFFFE5000 );

	write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
	write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
	
	set32( HW_PPCIRQMASK, (1<<31) );
	set32( HW_IPC_PPCCTRL, 0x30 );

	write32( 0x0D806008, 0 );
	
	EXIControl(0);

	write32( 0x1860, 0xdeadbeef );	// Clear OSReport area
	write32( 0x30F8, 0 );			// Tell PPC side to start

	ahb_flush_to( AHB_PPC );
	
	while (1)
	{
		ahb_flush_from( AHB_STARLET );	//flush to arm

		if( (((read32(0x12FC) >> 16) & 0x1030) == 0x1030 ) )
		{
			SysReset();
		}
		if( (((read32(0x12FC) >> 16) & 0x234) == 0x234 ) )
		{
			SysShutdown();
		}

		//Baten Kaitos save hax
		if( read32(0) == 0x474B4245 )
		{
			if( read32( 0x0073E640 ) == 0xFFFFFFFF )
			{
				write32( 0x0073E640, 0 );
			}
		}

		//if( read32(0x1860) != 0xdeadbeef )
		//{
		//	if( read32(0x1860) != 0 )
		//	{
		//		dbgprintf(	(char*)(P2C(read32(0x1860))),
		//					(char*)(P2C(read32(0x1864))),
		//					(char*)(P2C(read32(0x1868))),
		//					(char*)(P2C(read32(0x186C))),
		//					(char*)(P2C(read32(0x1870))),
		//					(char*)(P2C(read32(0x1864)))
		//				);				
		//	}

		//	write32(0x1860, 0xdeadbeef);
		//}

		DIUpdateRegisters();

		if( ConfigGetConfig(DML_CFG_NMM) )
			CARDUpdateRegisters();

		ahb_flush_to( AHB_PPC );	//flush to ppc
	}
}
