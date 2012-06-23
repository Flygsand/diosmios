#include "Card.h"

FIL CardStat;

void CardInit( void )
{
	FILINFO f;
	u32 i,wrote;
	CARDStat CStat;

	memset32( (void*)CARD_BASE, 0xdeadbeef, 0x20 );
	memset32( (void*)CARD_SHADOW, 0, 0x20 );

//Create savefile dirs for the current game
	if( f_chdir("/saves") != FR_OK )
	{
		f_mkdir("/saves");
		f_chdir("/saves");
	}

	if( f_chdir((const TCHAR*)0) != FR_OK )
	{
		f_mkdir((const TCHAR*)0);
		f_chdir((const TCHAR*)0);
	}	

	switch( f_stat( "stats.bin", &f ) )
	{
		case FR_NO_FILE:
		{
			if( f_open( &CardStat, "stats.bin", FA_CREATE_ALWAYS | FA_READ | FA_WRITE ) != FR_OK )
			{
				//dbgprintf("MC:Could not create stats file!\n");

			} else {
				
				memset32( &CStat, 0, sizeof( CARDStat ) );
				for( i=0; i < CARD_MAX_FILES; ++i )
				{
					f_write( &CardStat, &CStat, sizeof( CARDStat ), &wrote );
				}
				f_sync( &CardStat );
			}
		} break;
		case FR_OK:
		{
			if( f_open( &CardStat, "stats.bin", FA_OPEN_EXISTING | FA_READ | FA_WRITE ) != FR_OK )
			{
				;//dbgprintf("MC:Could not create stats file!\n");
			}
		} break;
		default:
		{
			;//dbgprintf("MC:CardInit fuck up\n");
		} break;
	}

	write32( 0x2FA0, 0 );
}
s32 CardFindFreeEntry( void )
{
	CARDStat CStat;
	u32 i;
	u32 read;

	for( i=0; i < CARD_MAX_FILES; ++i )
	{
		f_lseek( &CardStat, sizeof(CARDStat) * i );
		f_read( &CardStat, &CStat, sizeof(CARDStat), &read );

		if( CStat.length == 0 )
		{
			//dbgprintf("CardFindFreeEntry(%d)\n", i );
			return i;
		}
	}

	return -1;
}

s32 CardFindEntryByName( char *Filename )
{
	CARDStat CStat;
	u32 i;
	u32 read;

	for( i=0; i < CARD_MAX_FILES; ++i )
	{
		f_lseek( &CardStat, sizeof(CARDStat) * i );
		f_read( &CardStat, &CStat, sizeof(CARDStat), &read );
		
		if( memcmp( Filename, CStat.fileName, strlen(Filename) ) == 0 )
		{
			//dbgprintf("CardFindEntryByName(%d,%s,%s)\n", i, Filename, CStat.fileName );
			return i;
		}
	}
	return -1;	
}


s32 CardOpenFile( char *Filename, CARDFileInfo *CFInfo )
{
	FIL savefile;
	s32 Slot,fres;
	
	Slot = CardFindEntryByName(Filename);
	if( Slot < 0 )
	{	
		write32( CARD_SCMD_4, -1 );
		write32( CARD_SRETURN, CARD_NO_FILE );
		return -4;
	}
		
	fres = f_open( &savefile, Filename, FA_READ|FA_WRITE|FA_OPEN_EXISTING ) ;
	switch( fres )
	{
		case FR_NO_PATH:
		case FR_NO_FILE:
		{
			;//dbgprintf("MC:Failed to open:\"%s\":%d\n", Filename, fres );

			write32( CARD_SCMD_4, -1 );
			write32( CARD_SRETURN, CARD_NO_FILE );
		} break;
		default:
		{
			EXIControl(1);
			;//dbgprintf("MC:Failed to open:\"%s\":%d\n", Filename, fres );
			Shutdown();
		} break;
		case FR_OK:
		{
			f_close( &savefile );

			write32( CARD_SCMD_4, Slot );
			write32( CARD_SRETURN, CARD_SUCCESS );
		} break;
	}	

	return 0;
}
s32 CardFastOpenFile( u32 FileNo, CARDFileInfo *CFInfo )
{
	CARDStat CStat;
	FIL savefile;
	s32 fres;
	u32 read;

	if( FileNo >= CARD_MAX_FILES )
	{
		write32( CARD_SRETURN, CARD_NO_FILE );
		return 0;
	}
	
	f_lseek( &CardStat, sizeof(CARDStat) * FileNo );
	f_read( &CardStat, &CStat, sizeof(CARDStat), &read );

	if( CStat.length == 0 )
	{
		write32( CARD_SRETURN, CARD_NO_FILE );
		return 0;
	}
		
	fres = f_open( &savefile, CStat.fileName, FA_READ|FA_WRITE|FA_OPEN_EXISTING ) ;
	switch( fres )
	{
		case FR_NO_PATH:
		case FR_NO_FILE:
		{
			;//dbgprintf("MC:Failed to open:\"%s\":%d\n", CStat.fileName, fres );

			write32( CARD_SRETURN, CARD_NO_FILE );
		} break;
		default:
		{
			EXIControl(1);
			;//dbgprintf("MC:Failed to open:\"%s\":%d\n", CStat.fileName, fres );
			Shutdown();
		} break;
		case FR_OK:
		{
			write32( CARD_SCMD_4, savefile.fsize );

			f_close( &savefile );

			write32( CARD_SRETURN, CARD_SUCCESS );
		} break;
	}	

	return 0;
}
void CardDeleteFile( char *Filename )
{
	CARDStat CStat;
	u32 wrote;
	s32 Slot;

	Slot = CardFindEntryByName( Filename );
	if( Slot < 0 )
	{
		dbgprintf("MC:\"%s\" doesn't exists\n", Filename );
		write32( CARD_SRETURN, CARD_NO_FILE );
		return;
	}

	memset32( &CStat, 0, sizeof(CARDStat) );
	
	f_lseek( &CardStat, sizeof(CARDStat) * Slot );
	f_write( &CardStat, &CStat, sizeof(CARDStat), &wrote );
	f_sync( &CardStat );

	f_unlink( Filename );
	
	write32( CARD_SRETURN, CARD_SUCCESS );
}
void CardFastDelete( u32 FileNo )
{
	CARDStat CStat;
	u32 read;	

	if( FileNo >= CARD_MAX_FILES )
	{
		write32( CARD_SRETURN, CARD_NO_FILE );
		return;
	}
	
	f_lseek( &CardStat, sizeof(CARDStat) * FileNo );
	f_read( &CardStat, &CStat, sizeof(CARDStat), &read );

	if( f_unlink( CStat.fileName ) == FR_OK )
	{
		memset32( &CStat, 0, sizeof(CARDStat) );

		f_lseek( &CardStat, sizeof(CARDStat) * FileNo );
		f_write( &CardStat, &CStat, sizeof(CARDStat), &read );
		f_sync( &CardStat );

		write32( CARD_SRETURN, CARD_SUCCESS );

	} else {
		write32( CARD_SRETURN, CARD_NO_FILE );
	}

	return;
}
void CardRename( char *NameSrc, char *NameDst )
{
	CARDStat CStat;
	u32 wrote;
	s32 Slot;

	Slot = CardFindEntryByName( NameDst );
	if( Slot != -1 )
	{
		f_unlink( NameDst );
		
		f_lseek( &CardStat, sizeof(CARDStat) * Slot );
		f_read( &CardStat, &CStat, sizeof(CARDStat), &wrote );

		memset32( &CStat, 0, sizeof(CARDStat) );

		f_lseek( &CardStat, sizeof(CARDStat) * Slot );
		f_write( &CardStat, &CStat, sizeof(CARDStat), &wrote );
		f_sync( &CardStat );
	}

	Slot = CardFindEntryByName( NameSrc );
	if( Slot == -1 )
	{
		write32( CARD_SRETURN, CARD_NO_FILE );
		return;
	}

	switch( f_rename( NameSrc, NameDst ) )
	{
		case FR_OK:
		{
			f_lseek( &CardStat, sizeof(CARDStat) * Slot );
			f_read( &CardStat, &CStat, sizeof(CARDStat), &wrote );

			memcpy( CStat.fileName, NameDst, 32 );

			f_lseek( &CardStat, sizeof(CARDStat) * Slot );
			f_write( &CardStat, &CStat, sizeof(CARDStat), &wrote );
			f_sync( &CardStat );

			write32( CARD_SRETURN, CARD_SUCCESS );

		} break;
		//case FR_EXIST:
		//{
		//	write32( CARD_SRETURN, CARD_FILE_EXISTS );
		//} break;
		default:
		{
			write32( CARD_SRETURN, CARD_FATAL_ERROR );
		} break;
	}
}
void CardCreateFile( char *Filename, u32 Size, CARDFileInfo *CFInfo )
{
	CARDStat CStat;
	u32 i,fres,read;
	s32 Slot;
	FIL savefile;

	Slot = CardFindEntryByName( Filename );
	if( Slot >= 0 )
	{
		;//dbgprintf("MC:\"%s\" already exists\n", Filename );
		write32( CARD_SRETURN, CARD_FILE_EXISTS );
		return;
	}

	Slot = CardFindFreeEntry();
	if( Slot < 0 )
		return;
		
	fres = f_open( &savefile, Filename, FA_READ|FA_WRITE|FA_CREATE_NEW );
	switch( fres )
	{
		case FR_EXIST:
		{
			write32( CARD_SCMD_4, 0 );
			write32( CARD_SRETURN, CARD_FILE_EXISTS );
		} break;
		default:
		{
			EXIControl(1);
			;//dbgprintf("MC:Failed to create:\"%s\":%d\n", Filename, fres );
			Shutdown();
		} break;
		case FR_OK:
		{
			f_lseek( &CardStat, sizeof(CARDStat) * Slot );
			f_read( &CardStat, &CStat, sizeof(CARDStat), &read );
			
			memcpy( CStat.fileName, Filename, strlen(Filename) );

			CStat.length = Size;
			CStat.time   = 0x15745a1a;
			memcpy( CStat.gameName, (void*)0, 4 );
			memcpy( CStat.company, (void*)4, 2 );
			
			CStat.bannerFormat		= 0;
			CStat.iconAddr			= -1;
			CStat.iconFormat		= 0;
			CStat.iconSpeed			= 0;
			CStat.commentAddr		= -1;

			CStat.offsetBanner		= -1;
			CStat.offsetBannerTlut	= -1;
			for( i=0; i < 8; ++i )
				CStat.offsetIcon[i] = -1;
			CStat.offsetIconTlut	= -1;
			CStat.offsetData		=  0;
			
			f_lseek( &CardStat, sizeof(CARDStat) * Slot );
			f_write( &CardStat, &CStat, sizeof(CARDStat), &read );
			f_sync( &CardStat );
			
			CFInfo->chan   = 0;
			CFInfo->fileNo = Slot;
			CFInfo->iBlock = 0;
			CFInfo->length = Size;
			CFInfo->offset = 0;

			char *buf = (char*)malloc(512);
			memset32( buf, 0, 512 );
			
			//Create full file
			for( i=0; i < Size; i+=512 )
			{
				f_write( &savefile, buf, 512, &read);
			}

			f_close( &savefile );
			
			write32( CARD_SCMD_4, Slot );
			write32( CARD_SRETURN, CARD_SUCCESS );
		} break;
	}
}
void CardReadFile( u32 FileNo, u8 *Buffer, u32 Length, u32 Offset )
{
	u32 read;
	CARDStat CStat;

	FIL savefile;

	f_lseek( &CardStat, sizeof(CARDStat) * FileNo );
	f_read( &CardStat, &CStat, sizeof(CARDStat), &read );

	if( f_open( &savefile, CStat.fileName, FA_OPEN_EXISTING | FA_READ ) == FR_OK )
	{
		f_lseek( &savefile, Offset );
		f_read( &savefile, (void*)Buffer, Length, &read );
		f_close( &savefile );
	}
}
void CardWriteFile( u32 FileNo, u8 *Buffer, u32 Length, u32 Offset )
{
	u32 read;
	CARDStat CStat;

	FIL savefile;

	f_lseek( &CardStat, sizeof(CARDStat) * FileNo );
	f_read( &CardStat, &CStat, sizeof(CARDStat), &read );

	switch( f_open( &savefile, CStat.fileName, FA_OPEN_EXISTING | FA_WRITE ) )
	{
		case FR_OK:
		{
#ifdef CARD_DEBUG
			if( f_lseek( &savefile, Offset ) != FR_OK )
			{
				EXIControl(1);
				dbgprintf("Failed to seek to %08x\n", Offset );
				Shutdown();
			} else {
				if( f_write( &savefile, (void*)Buffer, Length, &read ) != FR_OK )
				{
					EXIControl(1);
					dbgprintf("Failed to write %d bytes to %p\n", Length, Buffer );
					Shutdown();					
				}
				if( read != Length )
				{
					EXIControl(1);
					dbgprintf("Short write; %d of %d bytes written!\n", read, Length );
					Shutdown();			
				}
			}
#else
			f_lseek( &savefile, Offset );
			f_write( &savefile, (void*)Buffer, Length, &read );
#endif
			f_close( &savefile );
		} break;
		default:
		{
			EXIControl(1);
			dbgprintf("Failed to open:\"%s\"\n", CStat.fileName );
			Shutdown();
		} break;
	}
}
void CardUpdateStats( CARDStat *CStat )
{
	u32 IconSize,BannerSize,Format,Offset,i,TLut=0;

	BannerSize	= CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT;
	IconSize	= CARD_ICON_WIDTH * CARD_ICON_HEIGHT;
	Offset		= CStat->iconAddr;

	if( CStat->bannerFormat & CARD_STAT_BANNER_C8 )
	{
		CStat->offsetBanner		= Offset;
		CStat->offsetBannerTlut	= Offset + BannerSize;

		Offset += BannerSize + 512;

	} else if( CStat->bannerFormat & CARD_STAT_BANNER_RGB5A3 ) {

		CStat->offsetBanner		=  Offset;
		CStat->offsetBannerTlut	= -1;

		Offset += BannerSize * 2;

	} else {
					
		CStat->offsetBanner		= -1;
		CStat->offsetBannerTlut	= -1;
	}
				
	for( i=0; i < CARD_ICON_MAX; ++i )
	{
		Format = CStat->iconFormat >> ( i * 2 );

		if( Format & CARD_STAT_ICON_C8 )
		{
			CStat->offsetIcon[i] = Offset;
			Offset += IconSize;
			TLut = 1;

		} else if ( Format & CARD_STAT_ICON_RGB5A3 ) {
						
			CStat->offsetIcon[i] = Offset;
			Offset += IconSize * 2;

		} else {
			CStat->offsetIcon[i] = -1;
		}
	}

	if( TLut )
	{
		CStat->offsetIconTlut = Offset;
		Offset += 512;
	} else {
		CStat->offsetIconTlut = -1;
	}

	CStat->offsetData = Offset;
}
u32 Device = 0;
void CARDUpdateRegisters( void )
{
	u32 read;

	if( read32(CARD_CONTROL) != 0xdeadbeef )
	{
		if( read32( CARD_CONTROL ) & (~3) )
		{
			write32( CARD_CONTROL, 0xdeadbeef );
			return;			
		}

		write32( CARD_SCONTROL, read32(CARD_CONTROL) & 3 );
		
		clear32( CARD_SSTATUS, 0x14 );

		write32( CARD_CONTROL, 0xdeadbeef );
		
#ifdef ACTIVITYLED
		set32( HW_GPIO_OUT, 1<<5 );
#endif

		while( read32(CARD_CMD) == 0xdeadbeef );
		write32( CARD_SCMD, read32(CARD_CMD) );
		write32( CARD_CMD, 0xdeadbeef );
		
		if( read32(CARD_CMD_1) != 0xdeadbeef )
		{
			write32( CARD_SCMD_1, read32(CARD_CMD_1) );
			write32( CARD_CMD_1, 0xdeadbeef );
		}

		if( read32(CARD_CMD_2) != 0xdeadbeef )
		{
			write32( CARD_SCMD_2, read32(CARD_CMD_2) );
			write32( CARD_CMD_2, 0xdeadbeef );
		}

		if( read32(CARD_CMD_3) != 0xdeadbeef )
		{
			write32( CARD_SCMD_3, read32(CARD_CMD_3) );
			write32( CARD_CMD_3, 0xdeadbeef );
		}
		
		if( read32(CARD_CMD_4) != 0xdeadbeef )
		{
			write32( CARD_SCMD_4, read32(CARD_CMD_4) );
			write32( CARD_CMD_4, 0xdeadbeef );
		}

		switch( read32(CARD_SCMD) >> 24 )
		{
			case 0x00:
			{
				;//dbgprintf("CARD:Warning unknown command!\n");
			} break;
			default:
			{
				//EXIControl(1);

				dbgprintf("CARD:Unknown CMD:%08X %08X %08X %08X %08X %08X\n", read32(CARD_SCMD), read32(CARD_SCMD_1), read32(CARD_SCMD_2), read32(CARD_SCMD_3), read32(CARD_SCMD_4), read32(CARD_SCONTROL) );

				Shutdown();
			} break;

			/* CARDOpen( char *FileName ) */
			case 0xC0:
			{
				char FileName[32];

				u32 FInfo = P2C(read32(CARD_SCMD_2));

				memcpy( FileName, (void*)0x17E0, 32 );
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDOpen( \"%s\", 0x%08x )", FileName, FInfo );
#endif
										
				CardOpenFile( (char*)FileName, (CARDFileInfo*)FInfo );

				while( read32(CARD_CONTROL) & 1 )
					clear32( CARD_CONTROL, 1 );
				
				while( (read32(CARD_SSTATUS) & 0x10) != 0x10 )
					set32( CARD_SSTATUS, 0x10 );
				
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif
			} break;
			case 0xC1:
			{
				u32 FileNo = read32(CARD_SCMD_1);
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDClose( %d )", FileNo );
#endif
				if( FileNo < 0 )
					write32( CARD_SRETURN, -128 );
				else
					write32( CARD_SRETURN, 0 );

				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32(CARD_SRETURN) );
#endif
			} break;
			case 0xC2:
			{
				char FileName[32];
				u32 Size	=	  read32(CARD_SCMD_2);
				u32 FInfo	= P2C(read32(CARD_SCMD_3));
				
				memcpy( FileName, (void*)0x17E0, 32 );
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDCreate( \"%s\", 0x%04x, 0x%08x )", FileName, Size, FInfo );
#endif

				CardCreateFile( (char*)FileName, Size, (CARDFileInfo*)FInfo );

				write32( 0x2FA0, read32(0x2FA0) + CARD_XFER_CREATE );
					
				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif
			} break;
			case 0xC3:
			{
				CARDStat CS;
				
#ifdef CARDDEBUG
			//	dbgprintf("MC:CARDGetState( %d, 0x%08x, ",  read32(CARD_SCMD_1), P2C(read32(CARD_SCMD_2)) );
#endif
					
				if( read32(CARD_SCMD_1) >= CARD_MAX_FILES )
				{
					EXIControl(1);
					dbgprintf("MC: Invalid file slot!:%d\n", read32(CARD_SCMD_1) );
					Shutdown();
				}

				f_lseek( &CardStat, sizeof(CARDStat) * read32(CARD_SCMD_1) );
				f_read( &CardStat, &CS, sizeof(CARDStat), &read );
				
				if( CS.length == 0 )
				{
#ifdef CARDDEBUG
				//	dbgprintf(")");
#endif
					write32( CARD_SRETURN, CARD_NO_FILE );
				} else {
					
#ifdef CARDDEBUG
				//	dbgprintf("\"%s\")", CS.fileName );
					dbgprintf("MC:CARDGetState( %d, 0x%08x, \"%s\"):0",  read32(CARD_SCMD_1), P2C(read32(CARD_SCMD_2)), CS.fileName );
#endif

					CardUpdateStats( &CS );
				
					memcpy( (void*)0x1780, &CS, sizeof(CARDStat) );
			
#ifdef CARDDEBUG	
					CARDStat *CSTAT = (CARDStat*)0x1780;
					
					dbgprintf("\nMC:Card Status:\n");
					dbgprintf("\tFilename:%.32s\n", CSTAT->fileName );
					dbgprintf("\tGameName:%.4s\n", CSTAT->gameName );
					dbgprintf("\tCompany :%.2s\n", CSTAT->company );
					dbgprintf("\tLength  :%d\n", CSTAT->length );
					dbgprintf("\tTime    :%d\n\n", CSTAT->time );
					
					dbgprintf("\tBannerFormat:%d\n", CSTAT->bannerFormat );
					dbgprintf("\tIconAddress :0x%04X\n", CSTAT->iconAddr );
					dbgprintf("\tIconFormat  :0x%02X\n", CSTAT->iconFormat );
					dbgprintf("\tIconSpeed   :0x%02X\n", CSTAT->iconSpeed );
					dbgprintf("\tComntAddress:0x%04X\n\n", CSTAT->commentAddr );
					
					dbgprintf("\tOffsetBanner :0x%04X\n", CSTAT->offsetBanner );
					dbgprintf("\tOffsetBnrTlt :0x%04X\n", CSTAT->offsetBannerTlut );

					for( i=0; i < CARD_ICON_MAX; ++i)
						dbgprintf("\tOffsetIcon[%d]:0x%04X\n", i, CSTAT->offsetIcon[i] );

					dbgprintf("\tOffsetIconTlt:0x%04X\n", CSTAT->offsetIconTlut );
					dbgprintf("\tOffsetData   :0x%04X\n", CSTAT->offsetData );
#endif
									
					write32( CARD_SRETURN, CARD_SUCCESS );
				}
				
				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
			//		dbgprintf("MC:CARDGetState( %d, 0x%08x, ):%d\n",  read32(CARD_SCMD_1), P2C(read32(CARD_SCMD_2)), read32( CARD_SRETURN) );
#endif
			} break;
			case 0xC4:
			{
				CARDStat CS;
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDSetState( %d, 0x%08x )",  read32(CARD_SCMD_1), P2C(read32(CARD_SCMD_2)) );
#endif

				if( read32(CARD_SCMD_1) >= CARD_MAX_FILES )
				{
					EXIControl(1);
					dbgprintf("\nMC: Invalid file slot!\n");
					Shutdown();
				}

				CARDStat *CStat = (CARDStat *) P2C(read32(CARD_SCMD_2));

				f_lseek( &CardStat, sizeof(CARDStat) * read32(CARD_SCMD_1) );
				f_read( &CardStat, &CS, sizeof(CARDStat), &read );

				if( CS.length == 0 )
				{
#ifdef CARDDEBUG
					dbgprintf(")");
#endif
					write32( CARD_SRETURN, CARD_NO_FILE );

				} else {

					CS.bannerFormat	= CStat->bannerFormat;
					CS.iconAddr		= CStat->iconAddr;
					CS.iconFormat	= CStat->iconFormat;
					CS.iconSpeed	= CStat->iconSpeed;
					CS.commentAddr	= CStat->commentAddr;
				
#ifdef CARDDEBUG
					dbgprintf("\nMC:Card Status:\n");
					dbgprintf("\tFilename:%.32s\n", CS.fileName );
					dbgprintf("\tGameName:%.4s\n", CS.gameName );
					dbgprintf("\tCompany :%.2s\n", CS.company );
					dbgprintf("\tLength  :%d\n", CS.length );
					dbgprintf("\tTime    :%d\n\n", CS.time );
					
					dbgprintf("\tBannerFormat:%d\n", CS.bannerFormat );
					dbgprintf("\tIconAddress :0x%04X\n", CS.iconAddr );
					dbgprintf("\tIconFormat  :0x%02X\n", CS.iconFormat );
					dbgprintf("\tIconSpeed   :0x%02X\n", CS.iconSpeed );
					dbgprintf("\tComntAddress:0x%04X\n\n", CS.commentAddr );
#endif

					CardUpdateStats( &CS );
					
#ifdef CARDDEBUG
					dbgprintf("\tOffsetBanner :0x%04X\n", CS.offsetBanner );
					dbgprintf("\tOffsetBnrTlt :0x%04X\n", CS.offsetBannerTlut );

					for( i=0; i < CARD_ICON_MAX; ++i)
						dbgprintf("\tOffsetIcon[%d]:0x%04X\n", i, CS.offsetIcon[i] );

					dbgprintf("\tOffsetIconTlt:0x%04X\n", CS.offsetIconTlut );
					dbgprintf("\tOffsetData   :0x%04X\n", CS.offsetData );
#endif
				
					f_lseek( &CardStat, sizeof(CARDStat) * read32(CARD_SCMD_1) );
					f_write( &CardStat, &CS, sizeof(CARDStat), &read );
					f_sync( &CardStat );

					write32( 0x2FA0, read32(0x2FA0) + CARD_XFER_SETSTATUS );
					
					write32( CARD_SRETURN, CARD_SUCCESS );
				}

				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":1\n");
#endif
			} break;
			/* CARDFastOpen( u32 FileNO, CARDFileInfo *CFInfo ) */			
			case 0xC5:
			{
				u32 FileNo	= read32(CARD_SCMD_1);
				u32 FInfo	= P2C(read32(CARD_SCMD_2));
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDFastOpen( %d, 0x%08X )", FileNo, FInfo );
#endif
				
				CardFastOpenFile( FileNo, (CARDFileInfo*)FInfo );

				while( read32(CARD_CONTROL) & 1 )
					clear32( CARD_CONTROL, 1 );
				
				while( (read32(CARD_SSTATUS) & 0x10) != 0x10 )
					set32( CARD_SSTATUS, 0x10 );
				
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif

			} break;
			case 0xC6:
			{
				char FileName[32];

				memcpy( FileName, (void*)0x17E0, 32 );
#ifdef CARDDEBUG
				dbgprintf("MC:CARDDelete( \"%s\" )", FileName );
#endif

				CardDeleteFile( (char*)FileName );

				write32( 0x2FA0, read32(0x2FA0) + CARD_XFER_DELETE );
					
				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif
			} break;
			case 0xC8:
			{
				u32 Buffer	= P2C(read32(CARD_SCMD_1));
				u32 Length	= read32(CARD_SCMD_2);
				u32 Offset	= read32(CARD_SCMD_3);
				u32 FileNo	= read32(CARD_SCMD_4);
				
#ifdef CARDDEBUG
				dbgprintf("MC:CARDWrite( %d, 0x%08x, 0x%04x, 0x%04x )", FileNo, Buffer, Offset, Length );
#endif

				if( FileNo >= CARD_MAX_FILES )
				{
					EXIControl(1);
					dbgprintf("\nMC: Invalid file slot!:%d\n", FileNo );
					Shutdown();
				}

				CardWriteFile( FileNo, (u8*)Buffer, Length, Offset );

				write32( 0x2FA0, read32(0x2FA0) + CARD_XFER_WRITE + Length );
					
				write32( CARD_SRETURN, 0 );

				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
				
#ifdef CARDDEBUG
				dbgprintf(":%u\n", read32(CARD_SRETURN) );
#endif
			} break;
			case 0xC9:
			{
				u32 Buffer	= P2C(read32(CARD_SCMD_1));
				u32 Length	= read32(CARD_SCMD_2);
				u32 Offset	= read32(CARD_SCMD_3);
				u32 FileNo	= read32(CARD_SCMD_4);
					
#ifdef CARDDEBUG
				dbgprintf("MC:CARDRead( %d, 0x%08x, 0x%04x, 0x%04x )", FileNo, Buffer, Offset, Length );
#endif

				if( FileNo >= CARD_MAX_FILES )
				{
					EXIControl(1);
					dbgprintf("\nMC: Invalid file slot!:%d\n", FileNo );
					Shutdown();
				}

				CardReadFile( FileNo, (u8*)Buffer, Length, Offset );

				write32( 0x2FA0, read32(0x2FA0) + Length );

				write32( CARD_SRETURN, 0 );

				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
				
#ifdef CARDDEBUG
				dbgprintf(":%u\n", read32(CARD_SRETURN) );
#endif
			} break;
			case 0xCA:
			{
				u32 FileNo = read32( CARD_SCMD_1 );
#ifdef CARDDEBUG
				dbgprintf("MC:CARDFastDelete( %u )", FileNo );
#endif
				CardFastDelete( FileNo );

				write32( 0x2FA0, read32(0x2FA0) + CARD_XFER_DELETE );
					
				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif
			} break;
			case 0xCB:
			{
				char NameSrc[32];
				char NameDst[32];

				memcpy( NameSrc, (void*)0x17C0, 32 );
				memcpy( NameDst, (void*)0x17E0, 32 );

#ifdef CARDDEBUG
				dbgprintf("MC:CARDRename( \"%s\", \"%s\" )", NameSrc, NameDst );
#endif
				CardRename( NameSrc, NameDst );

				while( read32(CARD_SCONTROL) & 1 )
					clear32( CARD_SCONTROL, 1 );

				set32( CARD_SSTATUS, 0x10 );
#ifdef CARDDEBUG
				dbgprintf(":%d\n", read32( CARD_SRETURN ) );
#endif
			} break;
		}
#ifdef ACTIVITYLED
		clear32( HW_GPIO_OUT, 1<<5 );
#endif
	}
}
