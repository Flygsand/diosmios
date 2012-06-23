#include "HW.h"

void EXIControl( u32 value )
{
	if( value == 0 )
	{
		*(vu32*)0x3160 = 0xDEADBEEF; 
		*(vu32*)0xD800070 &= ~1;
	} else {
		*(vu32*)0x3160 = 0; 
		*(vu32*)0xD800070 |= 1;
	}
}
void HWRegWriteBatch( u32 A, u32 B, u32 C, u32 D, u32 delay )
{
	DRAMWrite( 0x160, A );
	DRAMWrite( 0x161, B );
	DRAMWrite( 0x161, C );
	DRAMWrite( 0x161, D );
	udelay(delay);
}
//inline void RegisterWrite( u32 Offset, u32 Value )
//{
//	*(vu16*)(0x0D8B4000+Offset) = Value;
//}
//inline volatile u32 RegisterRead(  u32 Offset )
//{
//	return *(vu16*)(0x0D8B4000+Offset);
//}
//void DRAMWrite( u32 Register, u32 Value )
//{
//	RegisterWrite( 0x74, Register );
//	RegisterRead( 0x74 );
//	RegisterWrite( 0x76, Value );
//}
//u32 DRAMRead( u32 Register )
//{
//	RegisterWrite( 0x74, Register );
//	RegisterRead( 0x74 );
//	return RegisterRead( 0x76 );
//}
//void DRAMCTRLWrite( u32 ValueA, u32 ValueB )
//{
//	DRAMWrite( 0x0163, ValueA );
//	DRAMRead( 0x0163 );
//	DRAMWrite( 0x0162, ValueB );
//}
//u32 DRAMCTRLRead( u32 ValueA )
//{
//	DRAMWrite( 0x0163, ValueA );
//	DRAMRead( 0x0163 );
//	return DRAMRead( 0x0162 );
//}
void SomeFuncA( void )
{
	set32( 0xD80018C, 0x400 );
	set32( 0xD80018C, 0x800 );
}
void SomeFuncD( void )
{
	u32 cookie = *(vu32*)(0xD8001D8) & 0x7FFFFFFF;
	write32( 0xD8001D8, cookie );
	udelay(2);

	write32( 0xD8001D8, cookie & 0xBFFFFFFF );
	udelay(10);

	write32( 0xD8001D8, (read32(0xD8001D8) & 0xBFFFFFFF) | 0x40000000 );
	udelay(50);

	write32( 0xD8001D8, (read32(0xD8001D8) & 0x7FFFFFFF) | 0x80000000 );
	udelay(2);

}
void EHCIInit( void )
{
	SomeFuncA();

	u32 HWVerFlag = ((*(vu32*)0xD800214) << 0x18) >> 0x1C;

	write32( 0xD800088, 0xFE );
	udelay(2);
	
	SomeFuncD();

	write32( 0xD800088, 0xF6 );
	udelay(50);

	write32( 0xD800088, 0xF4 );
	udelay(1);

	write32( 0xD800088, 0xF0 );
	udelay(1);

	write32( 0xD800088, 0x70 );
	udelay(1);

	write32( 0xD800088, 0x60 );
	udelay(1);

	write32( 0xD800088, 0x40 );
	udelay(1); 

	write32( 0xD800088, 0x00 );
	udelay(1);

	write32( 0xD0400B4, 0x2214 );

	if( HWVerFlag == 0 )
	{
		write32( 0xD0400B0, 0x20400 );
	} else {
		write32( 0xD0400B0, 0x20600 );
	}

	write32( 0xD0400A4, 0x26 );
	udelay(1);

	write32( 0xD0400A4, 0x2026 );
	udelay(1); 

	write32( 0xD0400A4, 0x4026 );
	udelay(20);

	write32( 0xD0400CC, 0x111 );
	udelay(1);

	//set32( 0xD800194, 0x7FDFBCF );

	write32( 0xD8001E0, 0x65244A );
	write32( 0xD8001E4, 0x46A024 );
	
	return;
}
void Shutdown( void )
{
	udelay(200);
	set32( 0x0d8000E0, 1<<1);
	while(1);
}

void GetRevision( u32 *Version, u32 *Revision )
{
	u32 val = read32(HW_VERSION);

	*Version = (val << 24 ) >> 28;
	*Revision = val & 0x0F;
}
void PPCReset( void )
{
	clear32( HW_RESETS, 0x30 );
	udelay(15);
	set32( HW_RESETS, 0x20 );
}

void HWResetDisable( void )
{
	clear32( HW_RESETS, 0x4800 );
}
void HWResetEnable( void )
{
	set32( HW_RESETS, 0x4800 );
}
void HW_184( void )
{
	clear32( 0xD800184, 0x438E );
}
void HW_184_2( void )
{
	set32( 0xD800184, 0x438E );
}
void ChangeClock( void )
{
	write32( HW_CLOCKS, 1 );
	udelay(1);
	write32( HW_RESETS, 0x7FFFFFF7 );
	udelay(1);

	write32( HW_CLOCKS, 3 );
	write32( HW_RESETS, 0x7FFFFFFF );
	udelay(4);

	write32( HW_CLOCKS, 2 );
	udelay(1);
}

#define RAMTEST {write32( 0x13020000, 0xdeadbeaf); if( read32( 0x13020000 ) != 0xdeadbeaf ){ dbgprintf("Fail at:%d\n", __LINE__ );/*Shutdown();*/}}

u32 SP[3];
void DRAMInit( u32 A, u32 B )
{	
	SP[0] = 0;

	HWMAgic( 162, 999, A, B );
}
void HWMAgic( u32 R0, u32 R1, u32 R2, u32 R3 )
{
	u32 R10,R11,R7,R8,R9,value;

	R3 = R3 << 24;
	SP[1] = R3 >> 24;
	SP[2] = R0;

	R10 = (R1<<16)>>16;

	//if( read32( HW_RESETS ) & 0x800 )
	//{
	//	DRAMCTRLWrite( 0x18, 0 );
	//	DRAMCTRLWrite( 0x19, 0 );
	//	DRAMCTRLWrite( 0x17, 0 );
	//	udelay(10);
	//}

	//clear32( HW_RESETS, 0x800 );
	//udelay(100);

	clear32( HW_DIFLAGS, 0x20 );
	clear32( HW_RESETS, 0x100 );
	udelay(10);
	
	write32( HW_RESETS, (read32( HW_RESETS ) & 0xFFFFF7FF) | 0x800 );
	udelay(5);

	write32( HW_RESETS, (read32( HW_RESETS ) & 0xFFFFFEFF) | 0x100 );
	udelay(5);

	value = read32( HW_RESETS ) & 0xFFFFFEFF;
	write32( HW_RESETS, value );
	udelay(100);

	value |= 0x100;
	write32( HW_RESETS, value );
	udelay(5);
	
//SubD
	clear32( 0xD8001C0, 0xC0000000 );
	udelay(100);

	value = read32( 0xD8001BC );

		value&= ~0x3F;

		value|= 0;
		value&=0xFFFC003F;

		value|= 0x00001240;
		value&=0xF803FFFF;
	
		value|= 0x00100000;
		value&=0xEFFFFFFF;
	
		value|= 0;

	write32( 0xD8001BC, value );
	udelay(100);
	
	value = read32( 0xD8001C0 );

		value&=0x7FFFFFFF;
		value&=0xBFFFFFFF;
		value|=0x40000000;

		value&=0xEFFFFFFF;
		value|=0x10000000;

		value&=0xF7FFFFFF;
		value|=0x08000000;
		
	write32( 0xD8001C0, value );
	udelay(1000);
	
	value = read32( 0xD8001C0 );

		value&=0x7FFFFFFF;
		value|=0x80000000;
		
	write32( 0xD8001C0, value );
	udelay(1000);
	
	//DRAMWrite( 0x100, 0x24 );
	//udelay(5);
	//DRAMWrite( 0x100, 0x20 );

	DRAMCTRLWrite( 0x4B, 0 );
			
	if( SP[0] == 1 )
	{
		DRAMCTRLWrite( 0x0048, 0xD09 );
		udelay(50);
		if( SP[0] == 1 )
		{
			DRAMCTRLWrite( 0x0048, 0x509 );
		} else {
			DRAMCTRLWrite( 0x0048, 0x50B );
		}
	} else {
		DRAMCTRLWrite( 0x0048, 0xD0B );
		udelay(50);
		if( SP[0] != 1 )
		{
			DRAMCTRLWrite( 0x0048, 0x50B );
		} else {
			DRAMCTRLWrite( 0x0048, 0x509 );
		}
	}
	udelay(50);
	
//SubJ
	DRAMCTRLWrite( 0x003E, 0xF0F0 );
	DRAMCTRLWrite( 0x003F, 0xF0F0 );
	DRAMCTRLWrite( 0x0040, 0x1616 );
	DRAMCTRLWrite( 0x0041, 0x1616 );
	DRAMCTRLWrite( 0x0042, 0x1616 );
	DRAMCTRLWrite( 0x0043, 0x1616 );
	udelay(50);
	
//SubO
	if( SP[0] == 1 )
	{
		DRAMCTRLWrite( 0x0048, 0x0109 );
	} else {
		DRAMCTRLWrite( 0x0048, 0x010B );
	}
	udelay(10);
	
	DRAMCTRLWrite( 0x0047, 0x8000 );
	DRAMCTRLWrite( 0x0027, 0x0000 );

	DRAMWrite( 0x010C, 0x01FF );
	DRAMWrite( 0x010D, 0x0FFF );
	DRAMWrite( 0x010E, 0x0007 );
	DRAMWrite( 0x010B, 0x0001 );
	DRAMWrite( 0x0109, 0x0004 );
	DRAMWrite( 0x0108, 0x0006 );
	DRAMWrite( 0x010A, 0x0002 );
	DRAMWrite( 0x015B, 0x0EFF );
	DRAMWrite( 0x0134, 0x0008 );
	DRAMWrite( 0x0135, 0x000C );
	DRAMWrite( 0x0136, 0x0018 );
	DRAMWrite( 0x0140, 0x0006 );
	DRAMWrite( 0x015A, 0x0005 );
	DRAMWrite( 0x0137, 0x0005 );
	DRAMWrite( 0x0138, 0x0005 );
	DRAMWrite( 0x0139, 0x0005 );
	DRAMWrite( 0x013A, 0x0005 );
	DRAMWrite( 0x013B, 0x0005 );
	DRAMWrite( 0x013C, 0x0005 );
	DRAMWrite( 0x013D, 0x0005 );
	DRAMWrite( 0x013E, 0x0005 );
	DRAMWrite( 0x013F, 0x0005 );
	
	DRAMCTRLWrite( 0x001C, 0x0000 );
	DRAMCTRLWrite( 0x001B, 0x0000 );
	DRAMCTRLWrite( 0x0000, 0x0000 );
	DRAMCTRLWrite( 0x0015, 0x0001 );
	DRAMCTRLWrite( 0x0016, 0x0000 );
	DRAMCTRLWrite( 0x0025, 0x0001 );
	DRAMCTRLWrite( 0x0010, 0x0000 );
	DRAMCTRLWrite( 0x0023, 0x0008 );
	DRAMCTRLWrite( 0x0001, 0x0007 );
	DRAMCTRLWrite( 0x0002, 0x0004 );
	DRAMCTRLWrite( 0x0005, 0x0007 );
	DRAMCTRLWrite( 0x0008, 0x0004 );
	DRAMCTRLWrite( 0x0009, 0x0018 );
	DRAMCTRLWrite( 0x000A, 0x001B );
	DRAMCTRLWrite( 0x0004, 0x0017 );
	DRAMCTRLWrite( 0x0021, 0x000B );
	DRAMCTRLWrite( 0x000B, 0x0009 );
	DRAMCTRLWrite( 0x000C, 0x000B );
	DRAMCTRLWrite( 0x000D, 0x0006 );
	DRAMCTRLWrite( 0x000E, 0x000C );
	DRAMCTRLWrite( 0x000F, 0x0017 );
	DRAMCTRLWrite( 0x0011, 0xFC00 );
	DRAMCTRLWrite( 0x0012, 0x001F );
	DRAMCTRLWrite( 0x0013, 0x0000 );
	DRAMCTRLWrite( 0x0014, 0x0000 );
	DRAMCTRLWrite( 0x0006, 0x0002 );
	DRAMCTRLWrite( 0x0007, 0x000A );
	DRAMCTRLWrite( 0x0022, 0x0008 );
	DRAMCTRLWrite( 0x001F, 0x1FE0 );
	DRAMCTRLWrite( 0x0020, 0x0000 );
	DRAMCTRLWrite( 0x002C, 0x7252 );
	DRAMCTRLWrite( 0x002D, 0x4A5E );
	DRAMCTRLWrite( 0x002E, 0x7BDE );
	DRAMCTRLWrite( 0x002F, 0x00DE );
	DRAMCTRLWrite( 0x0030, 0x00CC );
	DRAMCTRLWrite( 0x0031, 0x0000 );
	DRAMCTRLWrite( 0x0032, 0x00CC );
	DRAMCTRLWrite( 0x0033, 0x0000 );
	DRAMCTRLWrite( 0x0034, 0x00CC );
	DRAMCTRLWrite( 0x0035, 0x0000 );
	DRAMCTRLWrite( 0x0036, 0x08EC );
	DRAMCTRLWrite( 0x0037, 0x0000 );
	DRAMCTRLWrite( 0x0038, 0x0476 );
	DRAMCTRLWrite( 0x0039, 0x0000 );
	
	if( SP[0] == 1 )
	{
		DRAMCTRLWrite( 0x3A, 0x800F );
		DRAMCTRLWrite( 0x3B, 7 );
		DRAMCTRLWrite( 0x3C, 0x800F );
		DRAMCTRLWrite( 0x3D, 7 );
		
	} else {
	
		DRAMCTRLWrite( 0x3A, 0 );
		DRAMCTRLWrite( 0x3B, 0 );
		DRAMCTRLWrite( 0x3C, 0 );
		DRAMCTRLWrite( 0x3D, 0 );
	}
		
	DRAMCTRLWrite( 0x45, 0 );

	DRAMCTRLWrite( 0x100, 0 );
	udelay(5);

	DRAMCTRLWrite( 0x18, 1 );
	udelay(5);

	DRAMCTRLWrite( 0x17, 1 );
	udelay(200);

	DRAMCTRLWrite( 0x4B, 1 );
	DRAMCTRLWrite( 0x4C, 1 );
	
	HWRegWriteBatch( 0xFFFF, 0x20, 0x21, 0x20, 1 );
	HWRegWriteBatch( 0x2882, 0x22, 0x23, 0x22, 5 );
	HWRegWriteBatch( 0x2882, 0x24, 0x25, 0x24, 5 );
	HWRegWriteBatch( 0x2C82, 0x22, 0x23, 0x22, 5 );

	u32 r5 = (DRAMCTRLRead(0x29) << 0x10) >> 0x18;
	
	HWRegWriteBatch( 0x2882, 0x22, 0x23, 0x22, 5 );
	HWRegWriteBatch( 0x2C82, 0x24, 0x25, 0x24, 5 );

	u32 r4 = (DRAMCTRLRead(0x29) << 0x10) >> 0x18;
	
	HWRegWriteBatch( 0x2882, 0x24, 0x25, 0x24, 5 );
	HWRegWriteBatch( 0x0903, 0x22, 0x23, 0x22, 1 );
	HWRegWriteBatch( 0x0903, 0x24, 0x25, 0x24, 1 );
	
	DRAMCTRLWrite( 0x4C, 0 );
	DRAMCTRLWrite( 0x18, 0 );
	DRAMCTRLWrite( 0x17, 0 );
	
	udelay(200);
	
	if( r4 == r5 )
	{
		DRAMCTRLWrite( 0x18, 1 );
		udelay(5);
		DRAMCTRLWrite( 0x17, 1 );

	} else {

		DRAMCTRLWrite( 0x17, 1 );
		udelay(5);
		DRAMCTRLWrite( 0x18, 1 );

	}
	udelay(200);
	
	DRAMCTRLWrite( 0x4B, 0 );

	if( r4 != r5 )
	{
		DRAMWrite( 0x10B, 7 );
		DRAMCTRLWrite( 0x15, 0 );
	}
	
	HWRegWriteBatch( 0xFFFF, 0x20, 0x21, 0x20, 2 );

	if( r4 == r5 )
	{
		HWRegWriteBatch( 0x288E, 0x22, 0x23, 0x22, 1 );
	} else {
		HWRegWriteBatch( 0x288A, 0x22, 0x23, 0x22, 1 );
	}

	if( r4 == r5 )
	{
		HWRegWriteBatch( 0x288E, 0x24, 0x25, 0x24, 1 );
	}
	
	HWRegWriteBatch( 0x903, 0x22, 0x23, 0x22, 1 );

	if( r4 == r5 )
	{
		HWRegWriteBatch( 0x903, 0x24, 0x25, 0x24, 1 );
	}
	
	udelay(70);

//SubEND
	HWRegWriteBatch( 0xFFFF, 0x20, 0x21, 0x20, 2 );
	HWRegWriteBatch( 0xFFFF, 2, 3, 2, 5 );
	HWRegWriteBatch( 0xFFFF, 2, 3, 2, 5 );

	int i;
	for( i=0; i < 0x10; i+=2 )
		write16( 0xD8B4000 + i, 0 );
	
	write16( 0xD8B4026, 65 );

//GC-MODE
	DRAMCTRLWrite( 0x18, 0 );
	DRAMCTRLWrite( 0x19, 1 );
	
	DRAMWrite( 0x113, 631 );
	
//END
	DRAMWrite( 0x165, 0x29 );
	DRAMWrite( 0x164, r5 );
	DRAMWrite( 0x165, 0x2B );
	DRAMWrite( 0x164, r4 );	
}
void MIOSInit( void )
{
	ahb_flush_from(1);
	ahb_flush_to(1);
	
	MIOSHWInit( 1, 1 );
}
void MIOSDoStuff( u32 R0, u32 R1 )
{
	if( R0 == 0 )
	{
		write32( 0xD8B0010, R0 );		
	}

	write32( 0xD8B0010, 0 );

	if( R0 == 1 )
	{
		if( R1 == 0 )
		{
			u32 value = read32( 0xD800140 );
			value |= R0;
			value &= 0xFFFF000F;
			write32( 0xD800140, value );			
		}
	}
}
void MIOSUnkInit( void )
{	
	u32 value = read32( HW_DIFLAGS );

	value &= 0xFFFFFEFF;
	value &= ~0x80;
	
	write32( HW_DIFLAGS, value );
	
	clear32( 0xD8001D0, 0x80000000 );
	udelay(2);
	
	clear32( 0xD8001D0, 0x40000000 );

	if( SP[1] <= 1 )
	{
		value = read32( 0xD8001CC );

		value&= 0xFFFC003F;
		value|= 0x00000FC0;
		value&=~0x0000003F;
		value&= 0xF803FFFF;
		value|= 0x04640000;

		write32( 0xD8001CC, value );

	} else {

		value = read32( 0xD8001A8 );

		value&= 0xFFFC003F;
		value|= 0x00000FC0;
		value&=~0x0000003F;
		value&= 0xF803FFFF;
		value|= 0x04640000;

		write32( 0xD8001A8, value );		
	}

	udelay(10);

	value = read32( 0xD8001D0 );
	value&= 0xBFFFFFFF;
	value|= 0x40000000;
	write32( 0xD8001D0, value );
	udelay(500);

	value = read32( 0xD8001D0 );
	value&= 0x7FFFFFFF;
	value|= 0x80000000;
	write32( 0xD8001D0, value );
	udelay(2);

}
void MIOSEHCISub( void )
{
	u32 v = read32( 0xD8001D8 ) & 0x7FFFFFFF;

	write32( 0xD8001D8, v );
	udelay(2);

	write32( 0xD8001D8, v & 0xBFFFFFFF );
	udelay(10);
	
	write32( 0xD8001D8, ( read32( 0xD8001D8 ) & 0xBFFFFFFF ) | 0x40000000 );
	udelay(50);
	
	write32( 0xD8001D8, ( read32( 0xD8001D8 ) & 0x7FFFFFFF ) | 0x80000000 );
	udelay(2);
}
char EHCIData[] = {
	0x02, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00,
	0x05, 0x0F, 0x00, 0x00,
};
void MIOSEHCISub2( u32 A )
{
	if( A > 2 )
		A = 2;

	A <<= 2;
	
	u32 R1 = *(u8*)(EHCIData+A);
	u32 R3 = *(u8*)(EHCIData+A+1);
	
	R1 &= 0xF;
	R3 &= 0xF;

u32 R4 = R1 << 8;
	R3 = R3 << 0x17;

	R4 = R4 | R3;
u32	R2 = R1 << 2;
	R2 = R2 + R1;
	R4 = R4 | 0x2014;

	R2 = R2 - 4;
	R2 = R2 & 0xFF;

	R2 = R2 << 8;
	R2 = R2 | 0x20000;

	write32( 0xD0400B4, R4 );
	write32( 0xD0400B0, R2 );	
}
void MIOSEHCIInit( u32 A )
{
	write32( 0xD800088, 0xFE );
	udelay(2);
	
	MIOSEHCISub();

	write32( 0xD800088, 0xF6 );
	udelay(50);

	write32( 0xD800088, 0xF4 );
	udelay(1);

	write32( 0xD800088, 0xF0 );
	udelay(1);

	write32( 0xD800088, 0x70 );
	udelay(1);

	write32( 0xD800088, 0x60 );
	udelay(1);

	write32( 0xD800088, 0x40 );
	udelay(1); 

	write32( 0xD800088, 0x00 );
	udelay(1);

	MIOSEHCISub2( A );

	if( A > 1 )
	{
		write32( 0xD0400A4, 0x23 );
		udelay(1);
		write32( 0xD0400A4, 0x2023 );
		udelay(1);
		write32( 0xD0400A4, 0x4023 );
		udelay(20);
	} else {
		write32( 0xD0400A4, 0x26 );
		udelay(1);
		write32( 0xD0400A4, 0x2026 );
		udelay(1);
		write32( 0xD0400A4, 0x4026 );
		udelay(20);
	}

	write32( 0xD0400CC, 0x111 );

}
void MIOSEHCIInit2( void )
{
	GetRevision( SP+1, SP );

	if( SP[1] <= 1 )
	{
		write32( 0xD8001E0, 0x65244A );
		write32( 0xD8001E4, 0x46A024 );
		return;
	}

//	dbgprintf("Unsupported CPU version!\n");
	
}
void MIOSHWInit( u32 A, u32 B )
{
	GetRevision( SP+1, SP );

	set32( HW_EXICTRL, 1 );

	MIOSDoStuff( SP[1], SP[0] );

	MIOSUnkInit();

	MIOSEHCIInit( SP[1] );

	set32( HW_RESETS, 0x7FDFBCF );

	MIOSEHCIInit2();
}
void UNKInit( u32 A, u32 B )
{
	u32 FlagA = (A << 24) >> 24;
	u32 FlagB =  B << 24;

	if( FlagB == 0 )
	{
		if( read32( 0xD8001D0 ) & (1<31) )
		{
			if( read32( 0xD8001D0 ) & (1<30) )
				return;
		}
	}

	u32 value = read32( HW_DIFLAGS );
	if( FlagA )
	{
		value &= 0xFFFFFEFF;
	} else {
		value &= 0xFFFFFEFF;
		value |= 0x100;
	}

	value &= ~0x80;
	
	write32( HW_DIFLAGS, value );


	clear32( 0xD8001D0, 0x80000000 );
	udelay(2);
	
	clear32( 0xD8001D0, 0x40000000 );
	
	if( FlagA )
	{
		value = read32( 0xD8001CC );

		value&= 0xFFFC003F;
		value|= 0x00000FC0;
		value&=~0x0000003F;
		value&= 0xF803FFFF;
		value|= 0x04640000;

		write32( 0xD8001CC, value );

	} else {
		
		clear32( 0xD8001D0, 0x10000000 );
		
		value = read32( 0xD8001CC );

		value&= 0xFFFC003F;
		value|= 0x0000FFC0;
		value&=~0x0000003F;
		value|= 0x0000000E;
		value&= 0xF803FFFF;
		value|= 0x04B00000;

		write32( 0xD8001CC, value );
	}

	udelay(10);

	value = read32( 0xD8001D0 );
	value&= 0xBFFFFFFF;
	value|= 0x40000000;
	write32( 0xD8001D0, value );
	udelay(500);

	value = read32( 0xD8001D0 );
	value&= 0x7FFFFFFF;
	value|= 0x80000000;
	write32( 0xD8001D0, value );
	udelay(2);
	
}
void BootPPC( void )
{
	u32 BootCode[] =	// 16 * 4
	{
		0x3C600000,
		0x60633400,
		0x7C7A03A6,
		0x38600000, 
		0x7C7B03A6,
		0x4C000064, 
		
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
		0x00000000, 
	} ;

	u32 cookie = read32(HW_EXICTRL);

	//Enable EXI
	set32( HW_EXICTRL, 1 );
	
	//Upload BootCode
	int i;
	for( i=0; i < 16; ++i )
		write32( 0xd806840 + i * 4, BootCode[i] );

	u32 val = read32( 0xd806840 + 4 * 16 - 4 );

	write32( HW_EXICTRL, cookie );
	val = read32(HW_EXICTRL);
}
extern u8 *RAM;
//void MEM2Switch( u32 A )
//{
//	clear32( HW_RESETS, 0x48000 );
//	clear32( 0xD800184, 0x428E );
//
//	if( A )
//	{
//
//		DRAMWrite( 0x10B, 7 );
//		DRAMCTRLWrite( 0x15, 0 );
//		DRAMCTRLWrite( 0x18, 1 );
//		DRAMCTRLWrite( 0x19, 0 );
//		DRAMCTRLWrite( 0x4A, 0x0E );
//		DRAMCTRLWrite( 0x0F, 0x08 );
//		DRAMCTRLWrite( 0x03, 0x0E );
//
//		DRAMCTRLWrite( 0x49, 1 );
//		udelay(2);
//
//		DRAMWrite( 0x113, 631 );
//
//	} else {
//		DRAMWrite( 0x10B, 7 );
//		DRAMCTRLWrite( 0x15, 0x00 );
//		DRAMCTRLWrite( 0x18, 0x00 );
//		DRAMCTRLWrite( 0x19, 0x01 );
//		DRAMCTRLWrite( 0x4A, 0x00 );
//		DRAMCTRLWrite( 0x0F, 0x17 );
//		DRAMCTRLWrite( 0x03, 0x00 );
//
//		DRAMCTRLWrite( 0x49, 0x00 );
//		udelay(2);
//
//		DRAMWrite( 0x113, 947 );
//	}
//	set32( HW_RESETS, 0x48000 );
//	set32( 0xD800184, 0x428E );
//
//	udelay(5000);
//
//	
//}
void MEMInitLow( void )
{
	write32( 0x3118, 0x04000000 );
	write32( 0x311C, 0x04000000 );
	
	write32( 0x3124, 0x90000800 );
	write32( 0x3128, 0x935E0000 );
	write32( 0x3130, 0x935E0000 );

	write32( 0x3138, 0x00000101 );
	write32( 0x3140, 0x00000707 );

	write32( 0x3144, 0x00082209 );

	write32( 0x3158, 0xCAFEBABE );

	write32( 0x3114, 0xDEADBEEF );

	write32( 0x312C, 0xDEADBEEF );
	write32( 0x313C, 0xDEADBEEF );

	write32( 0x3148, 0xDEADBEEF );
	write32( 0x314C, 0xDEADBEEF );

	write32( 0x3154, 0xDEADBEEF );
	write32( 0x3150, 0xDEADBEEF );

	write32( 0x315C, 0xDEADBEEF );
	write32( 0x3160, 0xDEADBEEF );
	
	write32( 0x3100, 0x01800000 );
	write32( 0x3104, 0x01800000 );
	write32( 0x3108, 0x81800000 );
	write32( 0x310C, 0x00000000 );
	write32( 0x3110, 0x81800000 );

	write32( 0x3120, 0x93600000 );
	write32( 0x3134, 0x93600000 );

	write32( 0x0028, 0x01800000 );
	write32( 0x00F0, 0x01800000 );
	
	write32( 0x0030, 0x00000000 );
	write32( 0x0034, 0x81800000 );
	
	write32( 0x00D0, 16*1024*1024 );		//Set ARAM size

	write16( 0x0D8B4200, 0x00000000 );
	udelay(1);
	
	write32( 0x00000080, 0x09142001 );	//some date? 14th SEP 2001
		
	clear32( HW_RESETS, 0x00048000 );
	clear32( 0xD800184, 0x0000438E );
	
		DRAMWrite( 0x10B, 7 );
		DRAMCTRLWrite( 0x15, 0 );
		DRAMCTRLWrite( 0x18, 1 );
		DRAMCTRLWrite( 0x19, 0 );
		DRAMCTRLWrite( 0x4A, 0x0E );
		DRAMCTRLWrite( 0x0F, 0x08 );
		DRAMCTRLWrite( 0x03, 0x0E );
		//DRAMCTRLWrite( 0x49, 0x0E );
		//udelay(2);

		//DRAMCTRLWrite( 0x49, 0x0F );
		//udelay(2);

		DRAMWrite( 0x113, 631 );
	
	set32( HW_RESETS, 0x00048000 );
	set32( 0xD800184, 0x0000438E );

	write32( 0x30F8, 0 );
	ahb_flush_to( 1 );

//RegisterSet
	clear32( HW_RESETS, 0x48F200 );
	udelay(1);

	u32 val = read32( HW_DIFLAGS );

	val &= ~0x01;
	//val &= ~0x02; // also required to be changed
	//val &= ~0x04;	//this disables bit 14 of the IRQ Flags
	val &= ~0x08;

	val |=  0x10;
	val |=  0x20;

	val &=  0xFFFFFEFF;
	val &= ~0x80;

	val &=  0xFFFFFDFF;
	val &=  0xFFFFFBFF;
	val &=  0xFFFFF7FF;

	val &=  0xFFFFDFFF;
	val &=  0xFFFFBFFF;
	val &=  0xFFFF7FFF;

	val &=  0xFFFEFFFF;
	val &=  0xFFFDFFFF;
	val &=  0xFFFBFFFF;

	write32( HW_DIFLAGS, val );
	udelay(1);
	
	set32( HW_RESETS, 0x48F200 );
	udelay(1);

//SetUP GPIOs
	clear32( 0xD8000E0, 0xC120 );
	set32( 0xD8000E0, read32( 0xD8000C0 ) );
	write32( 0xD8000FC, 0 );
	set32( 0xD8000E4, 0xFFDF3E );
	
	EXIControl(1);

	BootPPC();
	
	write32( HW_DIFLAGS, (read32(HW_DIFLAGS) & 0xFFEFFFFF) | 0x100000 );
	
	EXIControl(0);
	
	write32( 0xD800034, 0x40000000 );
	
	clear32( HW_RESETS, 0x10 | 0x20 );
	udelay(15);

	set32( HW_RESETS, 0x20 );
	udelay(150);
	
	set32( HW_RESETS, 0x10 );
	udelay(200);
	
	set32( 0xD8000E0, 0xCE0000 );

	udelay(200);
	
	while( read32( 0x30F8 ) == 0 )
		ahb_flush_from(0);
	
	val = read32(HW_DIFLAGS);

	val |=  0x00000040;
	val &= ~0x00200000;
	val |=  0x00200000;
	val &= ~0x00400000;
	val |=  0x00400000;
	val &= ~0x00001000;

	write32( HW_DIFLAGS, val );
	udelay(1);
	
	//write32( 0x30F8, 0 );
	//ahb_flush_to(1);

	clear32( HW_DIFLAGS, 0x180000 );
}
