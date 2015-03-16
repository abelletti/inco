#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK 4096
#define INFILE "comp1"

int main( int argc, char *argv[] )
{
	int infile;
	void *dataBlock;
	void *compBlock;
	int lenRead;
	int lenDecomp;
	int lenUsed;
	int lenWritten;

	if( NULL == (dataBlock = malloc( BLOCK )))
	{
		perror( "malloc dataBlock" );
		exit( 2 );
	}

	if( NULL == (compBlock = malloc( BLOCK )))
	{
		perror( "malloc compBlock" );
		exit( 3 );
	}

	if( -1 == (infile = open( INFILE, O_RDONLY )))
	{
		perror( "open" );
		exit( 1 );
	}

	lenRead = read( infile, compBlock, BLOCK );
	fprintf( stderr, "read %d bytes\n", lenRead );

	lenDecomp = decompress( compBlock, dataBlock, lenRead, 2048, &lenUsed );
	fprintf( stderr, "expanded to %d bytes\n", lenDecomp );
	fprintf( stderr, "used %d compressed bytes\n", lenUsed );

	if( -1 == (lenWritten = write( STDOUT_FILENO, dataBlock, lenDecomp )))
	{
		perror( "write to STDOUT" );
		exit( 4 );
	}
	else if( lenWritten != lenDecomp )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n",
		  lenWritten, lenDecomp );
		exit( 5 );
	}

	close( infile );

	return 0;
}

