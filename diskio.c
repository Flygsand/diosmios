#include "diskio.h"
#include "string.h"
#include "memory.h"

extern u32 IsInit;

DSTATUS disk_initialize( BYTE drv )
{
	s32 r, s_cnt;
	u32 s_size;

	while( 1 )
	{
		udelay( 50000 );

		tiny_ehci_init();

		int ret = -ENODEV;

		do {

			udelay( 4000 );
			ret = ehci_discover();

		} while( ret == -ENODEV );
		
		dbgprintf("ehci_discover():%d\n", ret );

		r = USBStorage_Init();
	
		if( r == 0 )
			break;
	}
	
	s_cnt = USBStorage_Get_Capacity( &s_size );

	dbgprintf( "DIP: Drive size: %dMB SectorSize:%d\n", s_cnt / 1024 * s_size / 1024, s_size);
	
	return r;

}

DSTATUS disk_status( BYTE drv )
{
	(void)drv;
	return 0;
}

DRESULT disk_read( BYTE drv, BYTE *buff, DWORD sector, BYTE count )
{
	u8 *buffer = (u8*)buff;
	//dbgprintf("disk_read( %d, %d, %p, %p)\n", sector, count, buff, buffer );

	if( (u32)buff & 0xF0000000 )
	{
		buffer = (u8*)0x1000;
		u32 i=0;
		u32 Blocks = 3;
		while(1)
		{
			if( (count-i) < Blocks )
				Blocks = (count-i);

			USBStorage_Read_Sectors( sector + i, Blocks, buffer );		
			memcpy( buff + i * 512, buffer, Blocks * 512 );

			i+=Blocks;

			if( i >= count )
				break;
		}
	} else {
		USBStorage_Read_Sectors( sector, count, buffer );
		dc_flushrange( buffer, count*512 );
		ahb_flush_from( AHB_SDHC );
	}

	return RES_OK;
}
// Write Sector(s)
DRESULT disk_write( BYTE drv, const BYTE *buff, DWORD sector, BYTE count )
{
	u8 *buffer = (u8*)buff;

	if( (u32)buff & 0xF0000000 )
	{
		buffer = (u8*)0x1000;
		u32 i;
		for( i=0; i < count; ++i )
		{
			memcpy( buffer, (void*)buff + i * 512, 512 );	
			USBStorage_Write_Sectors( sector + i, 1, buffer );
		}
	} else {

		ahb_flush_to( AHB_SDHC );
		USBStorage_Write_Sectors( sector, count, buffer );

	}

	return RES_OK;
}
