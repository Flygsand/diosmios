#include <stdarg.h>
#include "string.h"

char * strstr ( const char *str1, const char *str2);
int vsprintf(char *buf, const char *fmt, va_list args);
int _sprintf( char *buf, const char *fmt, ... );
int dbgprintf( const char *fmt, ...);
void hexdump(void *d, int len);
