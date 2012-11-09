#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define UINT_MAX ((unsigned int)0xffffffff)

#define CARD_DEBUG 1

#define CHEATHOOK 1
//#define DEBUGGER 1
//#define DEBUGGERWAIT 1
//#define ACTIVITYLED 1
//#define CARDMODE 1
//#define CARDDEBUG 1
#define REALNAND 1
#define PADHOOK 1

#define CONFIG_VERSION	0x00000002
#define DM_VERSION		0x00020005

#define	DI_SUCCESS	1
#define	DI_ERROR	2
#define	DI_FATAL	64

//#define DEBUG		0
#define false		0
#define true		1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef int bool;
typedef unsigned int sec_t;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;
typedef volatile signed long long vs64;

typedef s32 size_t;

typedef u32 u_int32_t;

typedef s32(*ipccallback)(s32 result,void *usrdata);

#define NULL ((void *)0)

#define ALIGNED(x) __attribute__((aligned(x)))

#endif

