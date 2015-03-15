#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define DEBUG 1

#define FLAG_UNCOMPRESSED 0
#define FLAG_LZJB 1

// must be an even multiple of COMPBLOCK
#define READBLOCK 1048576
#define OUTBLOCK 1048576
#define COMPBLOCK 16384

int main( int argc, char *argv[] )
{
	void *rawData;
	void *endData;
	void *readPtr;
	void *compData;
	void *compPtr;
	
	int lenRead;
	int lenComp;
	int lenWritten;
	uchar_t	format;		// what is format of following block?
	int inSize;		// size to compress for next block

	// need extra room to be sure we complete an OUTBLOCK
	if( NULL == (compData = malloc( OUTBLOCK + COMPBLOCK + 1 )))
	{
		perror( "malloc compData" );
		exit( 3 );
	}

	if( NULL == (rawData = malloc( READBLOCK )))
	{
		perror( "malloc rawData" );
		exit( 2 );
	}


	compPtr = compData;		// position in the compressed data buffer
	for( ;; ) {
		lenRead = read( STDIN_FILENO, rawData, READBLOCK );
		endData = rawData + lenRead;	// end of the raw data block
#if DEBUG
		fprintf( stderr, "read %d bytes\n", lenRead );
#endif
		// are we done?	
		if( lenRead == 0 ) { break; }

		if( lenRead % COMPBLOCK )
		// we've got a partial read, need to make it an even number of COMPBLOCKs
		{
			int remaining = COMPBLOCK - (lenRead % COMPBLOCK );
#if DEBUG
			fprintf( stderr, "need to fill %d bytes\n", remaining );
#endif
			do {
				lenRead = read( STDIN_FILENO, endData, remaining );
#if DEBUG
			fprintf( stderr, "read %d more bytes\n", lenRead );
			fprintf( stderr, "errno = %d\n", errno );
			perror( "reading more" );
#endif
				remaining -= lenRead;
				endData += lenRead;	// end of the raw data block
			} while ( (lenRead != 0) && (remaining != 0 ));
		}


		for( readPtr = rawData; readPtr < endData; readPtr += COMPBLOCK )
		{
#if DEBUG
			fprintf( stderr, "endData - readPtr = %d\n", (endData - readPtr ));
			fprintf( stderr, "compPtr offset = %d\n", compPtr - compData );
#endif
			// if we have at least a full COMPBLOCK left to compress
			if( (endData - readPtr) >= COMPBLOCK)
			{
				inSize = COMPBLOCK;
			}
			else	// this is the final input block
			{
				inSize = endData - readPtr;
#if DEBUG
			fprintf( stderr, "dealing with partial COMPBLOCK of %d\n", inSize );
#endif
			}
			lenComp = compress( readPtr, compPtr+1, inSize );
#if DEBUG
			fprintf( stderr, "compressed %d bytes to %d bytes\n", inSize, lenComp );
#endif
			if( lenComp == inSize )
			// we got nothing out of that
			{
				format = FLAG_UNCOMPRESSED;
			}
			else
			{
				format = FLAG_LZJB;
			}
			*(uchar_t *)compPtr = format;
			compPtr += (lenComp + 1);

			if( (compPtr - compData) >= OUTBLOCK)
			// we've got a full output block to write!
			{
				int extra;	// bytes past OUTBLOCK in buffer
#if DEBUG
				fprintf( stderr, "Writing %d bytes\n", OUTBLOCK );
#endif
				lenWritten = write( STDOUT_FILENO, compData, OUTBLOCK );	

				if( lenWritten == -1 )
				{	
					perror( "write to STDOUT" );
					exit( 4 );
				}
				else if( lenWritten != OUTBLOCK )
				{
					fprintf( stderr, "Short write!  Wrote %d, had %d\n",
					  lenWritten, OUTBLOCK );
					exit( 5 );
				}
				// copy what's left to start of the buffer
				extra = compPtr - compData - OUTBLOCK;
#if DEBUG
				fprintf( stderr, "compData = %x, compPtr = %x\n", compData, compPtr );
				fprintf( stderr, "%d extra bytes in output buffer\n", extra );
#endif
				memmove( compData, compData + OUTBLOCK, extra );
				compPtr -= OUTBLOCK;
#if DEBUG
				fprintf( stderr, "compData = %x, compPtr = %x\n", compData, compPtr );
#endif
			}
		}
	}

	// now write the remaining compressed data
#if DEBUG
	fprintf( stderr, "Writing %d bytes to stdout\n", (compPtr - compData));
#endif
	if( -1 == (lenWritten = write( STDOUT_FILENO, compData, compPtr - compData )))
	{
		perror( "write to STDOUT" );
		exit( 4 );
	}
	else if( lenWritten != (compPtr - compData) )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n", lenWritten,
		  (compPtr - compData) );
		exit( 5 );
	}

	exit( 0 );
}
