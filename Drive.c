#include "Drive.h"

void DVDLowReset( void )
{
	clear32( HW_RESETS, (1<<10) | (1<<17) );
	set32( HW_RESETS, (1<<10) | (1<<17) );
}
u32 DVDLowGetError( void )
{
	write32( 0x0D806000, 0x2E );
	write32( 0x0D806008, 0xE0000000 );
	write32( 0x0D806020, 0 );
	write32( 0x0D80601c, 1 );

	while( read32(0x0D80601c) & 1 );
	
	set32( 0x0D806000, (1<<4) );

	return read32( 0x0D806020 );
}
u32 DVDLowSeek( void )
{
	write32( 0x0D806000, 0x2E );
	write32( 0x0D806008, 0xAB000000 );
	write32( 0x0D806020, 0 );
	write32( 0x0D80601c, 1 );

	while( read32(0x0D80601c) & 1 );
	
	set32( 0x0D806000, (1<<4) );

	return read32( 0x0D806020 );
}
u32 DVDLowStopMotor( void )
{
	write32( 0x0D806000, 0x2E );
	write32( 0x0D806008, 0xE3000000 );
	write32( 0x0D806020, 0 );
	write32( 0x0D80601c, 1 );

	while( read32(0x0D80601c) & 1 );
	
	set32( 0x0D806000, (1<<4) );

	return read32( 0x0D806020 );
}
u32 LowReadDiscID( void *data )
{	
	write32( 0x0D806008, 0xA8000040 );
	write32( 0x0D80600C, 0 );
	write32( 0x0D806010, 0x20 );
	write32( 0x0D806018, 0x20 );
	
	write32( 0x0D806014, (u32)data );

	write32( 0x0D806000, 0x3A );
	
	write32( 0x0D80601C, 3 );
	
	while (1)
	{
		if( read32( 0x0D806000 ) & (1<<2) )
		{
			set32( 0x0D806000, (1<<2) );
			return 1;
		}
		if( read32( 0x0D806000 ) & (1<<4) )
		{
			set32( 0x0D806000, (1<<4) );
			return 0;
		}
	}
	
	return 0;
}
u32 LowRead( void *data, u32 Offset, u32 Length )
{	
	write32( 0x0D806008, 0xA8000000 );
	write32( 0x0D80600C, Offset>>2 );
	write32( 0x0D806010, Length );
	write32( 0x0D806018, Length );
	
	write32( 0x0D806014, (u32)data );

	write32( 0x0D806000, 0x3A );
	
	write32( 0x0D80601C, 3 );
	
	while (1)
	{
		if( read32( 0x0D806000 ) & (1<<2) )
		{
			set32( 0x0D806000, (1<<2) );
			return 1;
		}
		if( read32( 0x0D806000 ) & (1<<4) )
		{
			set32( 0x0D806000, (1<<4) );
			return 0;
		}
	}
	
	return 0;
}
u32 DVDEnableAudioStreaming( u32 Enable )
{
	write32( 0x0D806004, read32( 0x0D806004 ) );

	write32( 0x0D806008, 0xE4000000 | (Enable<<16) | 0x0A );
	
	write32( 0x0D80601C, 1 );

	while( read32(0x0D80601C) & 1 );
	
	while(1)
	{
		if( read32( 0x0D806000 ) & 4 )
			return DI_ERROR;
		if(!read32(0x0D806018))
			return DI_SUCCESS;
	}

	return DI_FATAL;
}
