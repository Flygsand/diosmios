#include "dip.h"

u32	StreamBufferSize= 54*1024;
u32 Streaming		= 0;
u32	StreamOffset	= 0;
u32 StreamDiscOffset= 0;
s32 StreamSize		= 0;
u32 StreamRAMOffset	= 0;
u32 StreamTimer		= 0;
u32 StreamStopEnd	= 0;
u32 GameRun			= 0;
u32 DOLMinOff		= 0;
u32 DOLMaxOff		= 0;
u32 DOLSize			= 0;
u32 DOLOffset		= 0;
s32 ELFNumberOfSections = 0;
u32 FSTMode			= 0;

extern DML_CFG *DMLCfg;

FIL GameFile;
u32 read;
u32 DiscRead=0;

char *getfilenamebyoffset(u32 offset)
{
	u32 fst_offset = read32(0x38) & ~0x80000000;
	
	u32 i;
	for (i = fst_offset + 12; i < 0x01800000; i+=12)
	{
		if (read8(i) == 0 && offset >= read32(i + 4) && offset < read32(i + 4) + read32(i + 8))
		{
			return (char *)(fst_offset + read32(fst_offset+8)*12 + (read32(i) & 0x00ffffff));
		}
	}
	return (char*)NULL;
}

void DIInit( void )
{
	memset32( (void*)DI_BASE, 0xdeadbeef, 0x30 );
	memset32( (void*)(DI_SHADOW), 0, 0x30 );

	write32( DI_SCONFIG, 0xFF );
	write32( DI_SCOVER, 0 );

	write32( HW_TIMER, 0 );
}
u32 DIUpdateRegisters( void )
{	
	u32 read,i,j;
	static u32 PatchState	= 0;
	static u32 DOLReadSize	= 0;
	static u32 PSOHack		= 0;
	
	if( read32(DI_CONTROL) != 0xdeadbeef )
	{
		write32( DI_SCONTROL, read32(DI_CONTROL) & 3 );
		
		clear32( DI_SSTATUS, 0x14 );

		write32( DI_CONTROL, 0xdeadbeef );
			
		if( read32(DI_SCONTROL) & 1 )
		{
			if( ConfigGetConfig(DML_CFG_ACTIVITY_LED) )
				set32( HW_GPIO_OUT, 1<<5 );

			if( read32(DI_CMD_0) != 0xdeadbeef )
			{
				write32( DI_SCMD_0, read32(DI_CMD_0) );
				write32( DI_CMD_0, 0xdeadbeef );
			}
						
			if( read32(DI_CMD_1) != 0xdeadbeef ) 
			{
				write32( DI_SCMD_1, read32(DI_CMD_1) );
				write32( DI_CMD_1, 0xdeadbeef );
			}
						
			if( read32(DI_CMD_2) != 0xdeadbeef )
			{
				write32( DI_SCMD_2, read32(DI_CMD_2) );
				write32( DI_CMD_2, 0xdeadbeef );
			}
						
			if( read32(DI_DMA_ADR) != 0xdeadbeef )
			{
				write32( DI_SDMA_ADR, read32(DI_DMA_ADR) );
				write32( DI_DMA_ADR, 0xdeadbeef );
			}

			if( read32(DI_DMA_LEN) != 0xdeadbeef )
			{
				write32( DI_SDMA_LEN, read32(DI_DMA_LEN) );
				write32( DI_DMA_LEN, 0xdeadbeef );
			}

			if( read32(DI_IMM) != 0xdeadbeef )
			{
				write32( DI_SIMM, read32(DI_IMM) );
				write32( DI_IMM, 0xdeadbeef );
			}

			switch( read32(DI_SCMD_0) >> 24 )
			{
				case 0xA7:
				case 0xA9:
					//dbgprintf("DIP:Async!\n");
				case 0xA8:
				{					
					u32 Buffer	= P2C(read32(DI_SDMA_ADR));
					u32 Length	= read32(DI_SCMD_2);
					u32 Offset	= read32(DI_SCMD_1) << 2;

				//	dbgprintf("DIP:DVDRead%02X( 0x%08x, 0x%08x, 0x%08x )\n", read32(DI_SCMD_0) >> 24, Offset, Length, Buffer|0x80000000  );
					
					//	udelay(250);

					if( FSTMode )
					{
						FSTRead( (char*)Buffer, Length, Offset );

					} else {						

						if( GameFile.fptr != Offset )
						if( f_lseek( &GameFile, Offset ) != FR_OK )
						{
							EXIControl(1);
							dbgprintf("DIP:Failed to seek to 0x%08x\n", Offset );
							while(1);
						}
						if( f_read( &GameFile, (char*)Buffer, Length, &read ) != FR_OK )
						{
							EXIControl(1);
							dbgprintf("DIP:Failed to read from 0x%08x to 0x%08X\n", Offset, Buffer );
							while(1);
						}
					}
					//if( ((read+31)&(~31)) != Length )
					//{
					//	dbgprintf("DIP:DVDLowRead Offset:%08X Size:%08d Dst:%08X\n", Offset, Length, Buffer  );
					//	dbgprintf("DIP:Failed to read %d bytes, only got %d\n", Length, read );
					//	break;
					//}

					if( (u32)Buffer == 0x01300000 )
					{
						DoPatches( (char*)(0x01300000), Length, 0x80000000 );
					}

					// PSO 1&2
					if( (read32(0) >> 8) == 0x47504F )
					{
						switch( Offset )
						{
							case 0x56B8E7E0:	// AppSwitcher	[EUR]
							case 0x56C49600:	// [USA]
							{
								DMLCfg->Config &= ~(DML_CFG_CHEATS|DML_CFG_PADHOOK|DML_CFG_DEBUGGER|DML_CFG_DEBUGWAIT);

								DoPatches( (char*)Buffer, Length, 0x80000000 );

							} break;
							case 0x5668FE20:	// psov3.dol [EUR]
							case 0x56750660:	// [USA]
							{
								PSOHack = 1;
							} break;
						}
					}

					if( PatchState == 0 )
					{
						if( Length == 0x100 || PSOHack )
						{
							if( read32( (u32)Buffer ) == 0x100 )
							{
								//quickly calc the size
								DOLSize = sizeof(dolhdr);
								dolhdr *dol = (dolhdr*)Buffer;
						
								for( i=0; i < 7; ++i )
									DOLSize += dol->sizeText[i];
								for( i=0; i < 11; ++i )
									DOLSize += dol->sizeData[i];
						
								DOLReadSize = Length;

								DOLMinOff=0x81800000;
								DOLMaxOff=0;
								
								for( i=0; i < 7; ++i )
								{
									if( dol->addressText[i] == 0 )
										continue;

									if( DOLMinOff > dol->addressText[i])
										DOLMinOff = dol->addressText[i];

									if( DOLMaxOff < dol->addressText[i] + dol->sizeText[i] )
										DOLMaxOff = dol->addressText[i] + dol->sizeText[i];
								}

								for( i=0; i < 11; ++i )
								{
									if( dol->addressData[i] == 0 )
										continue;

									if( DOLMinOff > dol->addressData[i])
										DOLMinOff = dol->addressData[i];

									if( DOLMaxOff < dol->addressData[i] + dol->sizeData[i] )
										DOLMaxOff = dol->addressData[i] + dol->sizeData[i];
								}

								DOLMinOff -= 0x80000000;
								DOLMaxOff -= 0x80000000;	

								if( PSOHack )
								{
									DOLMinOff = Buffer;
									DOLMaxOff = Buffer + DOLSize;
								}

								dbgprintf("DIP:DOLSize:%d DOLMinOff:0x%08X DOLMaxOff:0x%08X\n", DOLSize, DOLMinOff, DOLMaxOff );

								PatchState = 1;
							}
						
							PSOHack = 0;

						} else if( read32(Buffer) == 0x7F454C46 )
						{
							dbgprintf("DIP:Game is loading an ELF 0x%08x\n", Offset );

							DOLOffset = Offset;
							DOLSize	  = 0;

							if( Length > 0x1000 )
							{
								DOLReadSize = Length;
								DoPatches( (char*)(Buffer), Length, 0x80000000 );								
							} else
								DOLReadSize = 0;

							Elf32_Ehdr *ehdr = (Elf32_Ehdr*)Buffer;
							dbgprintf("DIP:ELF Programheader Entries:%u\n", ehdr->e_phnum );							

							for( i=0; i < ehdr->e_phnum; ++i )
							{
								Elf32_Phdr phdr;

								if( FSTMode )
								{
									FSTRead( (char*)&phdr, sizeof(Elf32_Phdr), DOLOffset + ehdr->e_phoff + i * sizeof(Elf32_Phdr) );

								} else {
									
									f_lseek( &GameFile, DOLOffset + ehdr->e_phoff + i * sizeof(Elf32_Phdr) );
									f_read( &GameFile, &phdr, sizeof(Elf32_Phdr), &read );
								}

								DOLSize += (phdr.p_filesz+31) & (~31);	// align by 32byte
							}

							dbgprintf("DIP:ELF size:%u\n", DOLSize );

							PatchState = 2;
						}

					} else if ( PatchState == 1 )
					{
						DOLReadSize += Length;
						//dbgprintf("DIP:DOL ize:%d DOL read:%d\n", DOLSize, DOLReadSize );
						if( DOLReadSize >= DOLSize )
						{
							DoPatches( (char*)(DOLMinOff), DOLMaxOff-DOLMinOff, 0x80000000 );
							PatchState = 0;
						}

					} else if ( PatchState == 2 )
					{
						DoPatches( (char*)(Buffer), Length, 0x80000000 );

						if( Buffer > DOLMaxOff )
							DOLMaxOff = Buffer;
						
						DOLReadSize += Length;
						//dbgprintf("DIP:ELF size:%d ELF read:%d\n", DOLSize, DOLReadSize );
						if( DOLReadSize >= DOLSize )
						{
							PatchState = 0;
						}
					}
										
					write32( DI_SDMA_LEN, 0 );
					
					while( read32(DI_SCONTROL) & 1 )
						clear32( DI_SCONTROL, 1 );
					
					set32( DI_SSTATUS, 0x3A );

					if( (read32(DI_SCMD_0) >> 24) == 0xA7 )
					{
						write32( 0x0d80000C, (1<<0) | (1<<4) );
						write32( HW_PPCIRQFLAG, read32(HW_PPCIRQFLAG) );
						write32( HW_ARMIRQFLAG, read32(HW_ARMIRQFLAG) );
						set32( 0x0d80000C, (1<<2) );	
					}

					write32( HW_TIMER, 0 );
										
				} break;
				default:
				{
					EXIControl(1);
					dbgprintf("DIP:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(DI_SCMD_0), read32(DI_SCMD_1), read32(DI_SCMD_2), read32(DI_SIMM), read32(DI_SDMA_ADR), read32(DI_SDMA_LEN) );
					while(1);
				} break;
			}

			if( ConfigGetConfig(DML_CFG_ACTIVITY_LED) )
				clear32( HW_GPIO_OUT, 1<<5 );

			return 1;
		} else {
			;//dbgprintf("DIP:DI_CONTROL:%08X:%08X\n", read32(DI_CONTROL), read32(DI_CONTROL) );
		}
	}

	if( (u64)read32(HW_TIMER) >= 2 * 60 * 243000000LL / 128 )
	{
		USBStorage_Read_Sectors( 23, 1, (void*)0x1000 );

		write32( HW_TIMER, 0 );
	}

	return 0;
}
