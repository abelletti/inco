// (c) 2015 Allen R. Belletti

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "comp.h"
#include "decomp.h"
#include "inco.h"

int Mode = DEFAULT;
int Debug = 0;
char *myName;		// what name were we invoked with?

int main( int argc, char* argv[] )
{
	int opt;

	myName = basename( argv[0] );

	while( -1 != (opt = getopt( argc, argv, OPTIONS )))
	{
		switch( opt )
		{
			case 'c':
				Mode = COMPRESS;
				break;
			case 'd': 
				Mode = DECOMPRESS;
				break;
			case 'D':
				Debug++;
				break;
			case '?':
			case 'h':
				showHelp();
				exit( 1 );
		}
	}

	switch( Mode )
	{
		case COMPRESS:
			exit( comp() );
		case DECOMPRESS:
			exit( decomp() );
		case DEFAULT:
			// we'll take compress in the case of an unknown name
			if( strcmp( myName, DECOMPNAME ))
				exit( comp() );
			else
				exit( decomp() );
	}
	exit( 0 ); // unreachable
}


void showHelp( void )
{
	fprintf( stderr, "usage: %s -%s\n", myName, OPTIONS );
	fprintf( stderr, "  -c : Compression mode\n" );
	fprintf( stderr, "  -d : Decompression mode\n" );
	fprintf( stderr, "  -D : Send debug output to stderr\n" );
	fprintf( stderr, "  -DD : Generate additional debug output\n" );
	fprintf( stderr, "  -h : This listing of options\n" );
}
