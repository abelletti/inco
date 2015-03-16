// (c) 2015 Allen R. Belletti

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "inco.h"

int Mode = DEFAULT;
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
			case '?':
				fprintf( stderr, "usage: %s: %s\n", argv[0], OPTIONS );
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
}
