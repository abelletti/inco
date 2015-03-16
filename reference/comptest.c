#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK 4096
#define INFILE "sample1"

int main( int argc, char *argv[] )
{
	int infile;
	void *dataBlock;
	void *compBlock;
	int lenRead;
	int lenComp;
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

	lenRead = read( infile, dataBlock, BLOCK );
	fprintf( stderr, "read %d bytes\n", lenRead );

	lenComp = compress( dataBlock, compBlock, BLOCK );
	fprintf( stderr, "compressed to %d bytes\n", lenComp );

	if( -1 == (lenWritten = write( STDOUT_FILENO, compBlock, lenComp )))
	{
		perror( "write to STDOUT" );
		exit( 4 );
	}
	else if( lenWritten != lenComp )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n",
		  lenComp, lenWritten );
		exit( 5 );
	}

	close( infile );

	return 0;
}

