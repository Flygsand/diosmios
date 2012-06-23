#include "DVD.h"

DVDConfig *DICfg = (DVDConfig *)NULL;
u32 read;
static char GamePath[256];

extern FIL GameFile;
extern u32 FSTMode;
extern u32 DOLMaxOff;
extern u32 DOLOffset;

u8 HardDriveConnected;
FATFS fatfs;

static u8 *FSTable ALIGNED(32);
u32 ApploaderSize=0;
u32 dolOffset=0;
u32 FSTableSize=0;
u32 FSTableOffset=0;

u32 FCEntry=0;
FileCache FC[FILECACHE_MAX];
u32 FCState[FILECACHE_MAX];

void DVDInit( void )
{
	int i=0;
	s32 fres = FR_DISK_ERR;
	int MountFail=0;
	HardDriveConnected = 0;

	while(!HardDriveConnected)
	{
		while(1)
		{
			fres = f_mount(0, &fatfs );
			dbgprintf( "DIP:f_mount():%d\n", fres );
			if( fres == FR_OK )
				break;
			else
				MountFail++;

			if( MountFail == 10 )
			{
				dbgprintf( "DIP:too much fail! looping now!\n");
				while(1);
			}

			udelay(500000);
		}

		//try to open a file, it doesn't have to exist, just testing if FS works
		FIL f;
		fres = f_open( &f, "/randmb.in", FA_READ|FA_OPEN_EXISTING );
		switch(fres)
		{
			case FR_OK:
				f_close( &f );
			case FR_NO_PATH:
			case FR_NO_FILE:
			{
				HardDriveConnected = 1;
				fres = FR_OK;
			} break;
			default:
			case FR_DISK_ERR:
			{
				dbgprintf( "DIP: Disk error\n", fres );
				while(1);
			} break;
		}
	}

	if( fres != FR_OK )
	{
		dbgprintf( "Could not find any USB device!");
	}

	return;
}
s32 DVDSelectGame( void )
{
	char *str  = (char*)malloca( 256, 32 );

	if( ConfigGetConfig(DML_CFG_GAME_PATH) )
	{
		sprintf( str, "%s", ConfigGetGamePath() );

	} else {
		dbgprintf("No game path was supplied!\n");
		free(str);
		return -1;
	}

	s32 fres = f_open( &GameFile, str, FA_READ );
	if( fres != FR_OK )
	{
		sprintf( GamePath, "%s", str );

		//Try to switch to FST mode
		if( !FSTInit() )
		{
			dbgprintf("Failed to open:\"%s\" fres:%d\n", str, fres );
			free(str);
			return -2;
		}

	} else {
	
		f_lseek( &GameFile, 0 );
		f_read( &GameFile, (void*)0, 0x20, &read );

		f_lseek( &GameFile, 0 );
		f_read( &GameFile, str, 0x400, &read );
	
		dbgprintf("DIP:Loading game %.6s: %s\n", str, (char *)(str+0x20));

		f_lseek( &GameFile, 0x420 );
		f_read( &GameFile, str, 0x40, &read );
	}
	
	free( str );

	return DI_SUCCESS;
}
u32 FSTInit( void )
{
	char Path[256];
	FIL fd;
	u32 read;
	
	sprintf( Path, "%ssys/boot.bin", GamePath );
	if( f_open( &fd, Path, FA_READ ) != FR_OK )
	{
		dbgprintf( "DIP:[%s] Failed to open!\n", Path );
		return 0;

	} else {

		u8 *rbuf = (u8*)malloc( 0x100 );
		
		f_lseek( &fd, 0 );
		f_read( &fd, rbuf, 0x100, &read );

		dbgprintf("DIP:Loading game %.6s: %s\n", rbuf, (char *)(rbuf+0x20));

		//Read DOL/FST offset/sizes for later usage
		f_lseek( &fd, 0x0420 );
		f_read( &fd, rbuf, 0x20, &read );

		dolOffset		= *(u32*)(rbuf);
		FSTableOffset	= *(u32*)(rbuf+4);
		FSTableSize		= *(u32*)(rbuf+8);

		free( rbuf );

		dbgprintf( "DIP:FSTableOffset:%08X\n", FSTableOffset );
		dbgprintf( "DIP:FSTableSize:  %08X\n", FSTableSize );
		dbgprintf( "DIP:DolOffset:    %08X\n", dolOffset );	

		FSTMode = 1;

		f_close( &fd );
	}

	//Init cache
	u32 count = 0;
	for( count=0; count < FILECACHE_MAX; ++count )
	{
		FCState[count] = 0xdeadbeef;
	}

	return 1;
}

void Asciify( char *str )
{
	int i=0;
	for( i=0; i < strlen(str); i++ )
		if( str[i] < 0x20 || str[i] > 0x7F )
			str[i] = '_';
}
void FSTRead( char *Buffer, u32 Length, u32 Offset )
{
	char Path[256];
	FIL fd;
	u32 read;
	int i,j;
	
	if( Offset >= FSTableOffset+FSTableSize ) {
				
		//Get FSTTable offset from low memory, must be set by apploader
		if( FSTable == NULL )
		{
			FSTable	= (u8*)((*(vu32*)0x38) & 0x7FFFFFFF);
			//dbgprintf("DIP:FSTOffset:  %08X\n", (u32)FSTable );
		}
		
		//try cache first!
		for( i=0; i < FILECACHE_MAX; ++i )
		{
			if( FCState[i] == 0xdeadbeef )
				continue;

			if( Offset >= FC[i].Offset )
			{
				u64 nOffset = Offset - FC[i].Offset;
				if( nOffset < FC[i].Size )
				{
					//dbgprintf("DIP:[Cache:%02d][%08X:%05X]\n", i, (u32)(nOffset>>2), Length );
					f_lseek( &(FC[i].File), nOffset );
					f_read( &(FC[i].File), Buffer, ((Length)+31)&(~31), &read );
					return;
				}
			}
		}

		//The fun part!

		u32 Entries = *(u32*)(FSTable+0x08);
		char *NameOff = (char*)(FSTable + Entries * 0x0C);
		FEntry *fe = (FEntry*)(FSTable);

		u32 Entry[16];
		u32 LEntry[16];
		u32 level=0;

		for( i=1; i < Entries; ++i )
		{
			if( level )
			{
				while( LEntry[level-1] == i )
				{
					//printf("[%03X]leaving :\"%s\" Level:%d\n", i, buffer + NameOff + swap24( fe[Entry[level-1]].NameOffset ), level );
					level--;
				}
			}

			if( fe[i].Type )
			{
				//Skip empty folders
				if( fe[i].NextOffset == i+1 )
					continue;

				//printf("[%03X]Entering:\"%s\" Level:%d leave:%04X\n", i, buffer + NameOff + swap24( fe[i].NameOffset ), level, swap32( fe[i].NextOffset ) );
				Entry[level] = i;
				LEntry[level++] = fe[i].NextOffset;
				if( level > 15 )	// something is wrong!
					break;
			} else {

				if( Offset >= fe[i].FileOffset )
				{
					u32 nOffset = (Offset - fe[i].FileOffset);
					if( nOffset < fe[i].FileLength )
					{
					//	dbgprintf("DIP:Offset:%08X FOffset:%08X Dif:%08X Flen:%08X nOffset:%08X\n", Offset, fe[i].FileOffset, Offset-fe[i].FileOffset, fe[i].FileLength, nOffset );

						//Do not remove!
						memset( Path, 0, 256 );					
						sprintf( Path, "%sroot/", GamePath );

						for( j=0; j<level; ++j )
						{
							if( j )
								Path[strlen(Path)] = '/';
							memcpy( Path+strlen(Path), NameOff + fe[Entry[j]].NameOffset, strlen(NameOff + fe[Entry[j]].NameOffset ) );
						}
						if( level )
							Path[strlen(Path)] = '/';
						memcpy( Path+strlen(Path), NameOff + fe[i].NameOffset, strlen(NameOff + fe[i].NameOffset) );
						
						if( FCEntry >= FILECACHE_MAX )
							FCEntry = 0;

						if( FCState[FCEntry] != 0xdeadbeef )
						{
							f_close( &(FC[FCEntry].File) );
							FCState[FCEntry] = 0xdeadbeef;
						}

						Asciify( Path );

						f_open( &(FC[FCEntry].File), Path, FA_READ );

						FC[FCEntry].Size	= fe[i].FileLength;
						FC[FCEntry].Offset	= fe[i].FileOffset;
						FCState[FCEntry]	= 0x23;

						f_lseek( &(FC[FCEntry].File), nOffset );
						f_read( &(FC[FCEntry].File), Buffer, Length, &read );

						FCEntry++;
					}
				}
			}
		}

	} else if ( Offset >= FSTableOffset ) {
		
		Offset -= FSTableOffset;
		
		sprintf( Path, "%ssys/fst.bin", GamePath );
		if( f_open( &fd, Path, FA_READ ) != FR_OK )
		{
			dbgprintf( "DIP:[%s] Failed to open!\n", Path );
			return;
		} else {
			//dbgprintf( "DIP:[fst.bin] Offset:%08X Size:%08X\n", Offset, Length );
			
			f_lseek( &fd, Offset );
			f_read( &fd, Buffer, Length, &read );
			f_close( &fd );
						
			if( FSTable == NULL )
			{
				FSTable	= (u8*)Buffer;
			}

			return;
		}

	} else if ( Offset >= dolOffset ) {
		
		Offset -= dolOffset;
		
		sprintf( Path, "%ssys/main.dol", GamePath );
		if( f_open( &fd, Path, FA_READ ) != FR_OK )
		{
			dbgprintf( "DIP:[%s] Failed to open!\n", Path );
			return;
		} else {
			//dbgprintf( "DIP:[main.dol] Offset:%08X Size:%08X\n", Offset, Length );
			
			f_lseek( &fd, Offset );
			f_read( &fd, Buffer, Length, &read );
			f_close( &fd );

			return;
		}

	} else if ( Offset >= 0x2440 ) {
		
		Offset -= 0x2440;
		
		sprintf( Path, "%ssys/apploader.img", GamePath );
		if( f_open( &fd, Path, FA_READ ) != FR_OK )
		{
			dbgprintf( "DIP:[%s] Failed to open!\n", Path );
			return;
		} else {
			//dbgprintf( "DIP:[apploader.img] Offset:%08X Size:%08X\n", Offset, Length );
			
			f_lseek( &fd, Offset );
			f_read( &fd, Buffer, Length, &read );
			f_close( &fd );

			return;
		}

	} else if ( Offset >= 0x440 ) {

		Offset -= 0x440;
		
		sprintf( Path, "%ssys/bi2.bin", GamePath );
		if( f_open( &fd, Path, FA_READ ) != FR_OK )
		{
			dbgprintf( "DIP:[%s] Failed to open!\n", Path );
			return;
		} else {
			//dbgprintf( "DIP:[bi2.bin] Offset:%08X Size:%08X\n", Offset, Length );
			
			f_lseek( &fd, Offset );
			f_read( &fd, Buffer, Length, &read );

			f_close( &fd );

			return;
		}

	} else {
		sprintf( Path, "%ssys/boot.bin", GamePath );
		if( f_open( &fd, Path, FA_READ ) != FR_OK )
		{
			dbgprintf( "DIP:[%s] Failed to open!\n", Path );
			return;
		} else {
			//dbgprintf( "DIP:[boot.bin] Offset:%08X Size:%08X\n", Offset, Length );
			
			f_lseek( &fd, Offset );
			f_read( &fd, Buffer, Length, &read );

			f_close( &fd );

			return;
		}
	}
}
