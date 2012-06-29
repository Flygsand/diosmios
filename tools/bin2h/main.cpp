#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>

int main( int argc, char *argv[] )
{
	if( argc != 2 )
	{
		return 0;
	}

	FILE *in = fopen( argv[1], "rb" );
	if( in == NULL )
	{
		printf("couldn't open:\"%s\"\n", argv[1] );
		return -1;
	}

	fseek( in, 0, SEEK_END );
	unsigned int size = ftell(in);
	fseek( in, 0, 0 );

	int point=0;
	int slash=0;

	for( int i = strlen(argv[1]) - 1; i > 0; --i )
	{
		if( argv[1][i] == '.' && point == 0 )
			point = strlen(argv[1]) - i;

		if( argv[1][i] == '\\' || argv[1][i] == '/' )
		{
			slash = i + 1;
			break;
		}
	}

	char * str = new char[ strlen(argv[1]) - slash - point + 3 ]; // '.' 'h' '\0'

	memcpy( str, argv[1]+slash, strlen(argv[1]) - slash - point );
	
	str[ strlen(argv[1]) - slash - point + 0 ] = '.';
	str[ strlen(argv[1]) - slash - point + 1 ] = 'h';
	str[ strlen(argv[1]) - slash - point + 2 ] = '\0';

	printf("Using:\"%s\" for name\n", str );

	FILE *out = fopen( str, "wb" );
	
	str[ strlen(argv[1]) - slash - point ] = '\0';

	fprintf( out, "/*\n\tFilename    : %s\n", argv[1] );

	time_t t;
	time(&t);

	fprintf( out, "\tDate created: %s*/\n\n", ctime (&t)  );

	fprintf( out, "#define %s_size 0x%x\n\n", str, size );

	fprintf( out, "unsigned char %s[] = {\n\n\t", str );

	for( int i=0; i < size; ++i )
	{
		fprintf( out, "0x%02X", fgetc(in) );

		if( i+1 != size )
			fprintf( out, ", " );
		
		if( ((i+1) & 0xF) == 0 )
			fprintf( out, "\n" );

		if( ((i+1) & 0x3) == 0 )
			fprintf( out, "\t" );
	}

	fprintf( out, "\n};\n" );

	fclose( out );
	free( str );

	return 0;
}