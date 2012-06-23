#include "Config.h"

DML_CFG *DMLCfg;

void ConfigInit( DML_CFG *Cfg )
{
	DMLCfg = (DML_CFG*)0xFFFE4200;

	memset32( DMLCfg, 0, sizeof(DML_CFG) );

	//If a loader supplied any options we use them otherwise use the code defines
	if( Cfg->Magicbytes == 0xD1050CF6 && Cfg->Version == CONFIG_VERSION )
	{
		memcpy( DMLCfg, Cfg, sizeof( DML_CFG ) );

	} else {

		dbgprintf("No config found in RAM\n");
		dbgprintf("Version:%08X\n", DMLCfg->Version );
		dbgprintf("Config:%08X\n", DMLCfg->Config );
		
		DMLCfg->Config = 0;
#ifdef CHEATHOOK
		DMLCfg->Config |= DML_CFG_CHEATS;
#endif
#ifdef DEBUGGER
		DMLCfg->Config |= DML_CFG_DEBUGGER;
#endif
#ifdef DEBUGGERWAIT
		DMLCfg->Config |= DML_CFG_DEBUGWAIT;
#endif
#ifdef CARDMODE
		DMLCfg->Config |= DML_CFG_NMM;
#endif
#ifdef CARDDEBUG
		DMLCfg->Config |= DML_CFG_NMM_DEBUG;
#endif
#ifdef ACTIVITYLED
		DMLCfg->Config |= DML_CFG_ACTIVITY_LED;
#endif
#ifdef PADHOOK
		DMLCfg->Config |= DML_CFG_PADHOOK;
#endif
		DMLCfg->VideoMode	= DML_VID_DML_AUTO;
		DMLCfg->Version		= CONFIG_VERSION;
		DMLCfg->Magicbytes	= 0xD1050CF6;
	}

	//Check if a memcard is inserted in Slot A
	if( (read32(0xD806800) & 0x1000) == 0x1000 )
	{
		DMLCfg->Config &= ~DML_CFG_NMM;		// disable NMM
	}

	dbgprintf("Config:%08X\n", DMLCfg->Config );
}

inline u32 ConfigGetConfig( u32 Config )
{
	return !!(DMLCfg->Config & Config);
}
u32 ConfigGetVideMode( void )
{
	return DMLCfg->VideoMode;
}

char *ConfigGetGamePath( void )
{
	return DMLCfg->GamePath;
}
char *ConfigGetCheatPath( void )
{
	return DMLCfg->CheatPath;
}
