#ifndef _DRIVE_
#define _DRIVE_

#include "global.h"
#include "HW.h"
#include "memory.h"

void DVDLowReset( void );
u32 DVDEnableAudioStreaming( u32 Enable );
u32 DVDLowGetError( void );
u32 DVDLowStopMotor( void );
u32 LowReadDiscID( void *data );
u32 LowRead( void *data, u32 Offset, u32 Length );
u32 DVDLowSeek( void );

#endif
