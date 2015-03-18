// (c) 2015 Allen R. Belletti

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#ifdef LINUX
#include <stdint.h>
typedef uint8_t uchar_t;
#define NBBY 8
#endif

#include "defs.h"
#include "comp.h"
#include "lzjb.h"

extern int Debug;

int comp( void )
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
	int inSize;		// size to compress for next block, always COMPBLOCK
				// unless we've hit EOF and this is final block

	// allocate the necessary read buffer
	if( NULL == (rawData = malloc( READBLOCK )))
	{
		perror( "malloc rawData" );
		return 2;
	}

	// allocate sufficient output buffer to hold an entire OUTBLOCK plus
	// enough extra to hold the last compressed block worth of output
	if( NULL == (compData = malloc( OUTBLOCK + COMPBLOCK + 1 )))
	{
		perror( "malloc compData" );
		return 3;
	}

	compPtr = compData;		// next free byte in the compressed data buffer

	for( ;; ) {
		// FIRST PART: fetch a block of data
		// in an ideal world we'd use a separate thread for this
		// we're going to request an entire READBLOCK bytes worth of data.
		// Depending upon how our input is provided (redirect vs. pipe),
		// that request may or may not be satisfied in full.
	
		lenRead = read( STDIN_FILENO, rawData, READBLOCK );
		endData = rawData + lenRead;	// end of the raw data block, first free byte

		if( Debug )
			fprintf( stderr, "read %d bytes\n", lenRead );

		// hit EOF and done?
		if( lenRead == 0 ) { break; }

		if( lenRead % COMPBLOCK )
		// we've got a partial read, need to make it an even number of COMPBLOCKs
		// this could require more than one try, so we'll loop
		{
			int remaining = COMPBLOCK - (lenRead % COMPBLOCK );
			if( Debug >= 2 )
				fprintf( stderr, "need to fill %d bytes\n", remaining );

			do {
				if( -1 == (lenRead = read( STDIN_FILENO, endData, remaining )))
				{
					perror( "reading more" );
					return 7;
				}
				if( Debug >= 2 )
				{
					fprintf( stderr, "read %d more bytes\n", lenRead );
					fprintf( stderr, "errno = %d\n", errno );
				}
				remaining -= lenRead;
				endData += lenRead;	// end of the raw data block
			} while ( (lenRead != 0) && (remaining != 0 ));
			// end once we've reached a COMPBLOCK boundary or hit EOF
		}

		// SECOND PART: compress what we've got and send it along
		// data starts at rawData, runs through endData-1

		for( readPtr = rawData; readPtr < endData; readPtr += COMPBLOCK )
		{
			if( Debug >= 2 )
			{
				fprintf( stderr,
				  "endData - readPtr (data left in buffer to compress) = %d\n",
				  (int) (endData - readPtr) );
				fprintf( stderr,
				  "compPtr offset (how much output in buffer) = %d\n",
				  (int) (compPtr - compData) );
			}
			// if we have at least a full COMPBLOCK left to compress
			if( (endData - readPtr) >= COMPBLOCK)
			{
				inSize = COMPBLOCK;
			}
			else	// this is the final input block
			{
				inSize = endData - readPtr;
				if( Debug )
					fprintf( stderr,
					  "dealing with partial COMPBLOCK of %d\n", inSize );
			}
			// compress to compPtr+1 in order to leave *compPtr free for
			// the format flag
			lenComp = compress( readPtr, compPtr+1, inSize );
			if( Debug >= 2 )
				fprintf( stderr, "compressed %d bytes to %d bytes\n",
				  inSize, lenComp );
			if( lenComp > inSize )
			{
				// this should never, ever happen
				fprintf( stderr, "FAIL: lenComp=%d > inSize=%d!!!",
				  lenComp, inSize );
				return 6;
			}

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
				if( Debug )
					fprintf( stderr, "Writing %d bytes\n", OUTBLOCK );
				
				if( -1 == 
				  (lenWritten = write( STDOUT_FILENO, compData, OUTBLOCK )))
				{	
					perror( "write to STDOUT" );
					return 4;
				}
				else if( lenWritten != OUTBLOCK )
				{
					fprintf( stderr, "Short write!  Wrote %d, had %d\n",
					  lenWritten, OUTBLOCK );
					return 5;
				}
				// copy what's left to start of the buffer
				extra = compPtr - compData - OUTBLOCK;
				if( Debug >= 2 )
				{
					fprintf( stderr, "compData = %p, compPtr = %p\n",
					  compData, compPtr );
					fprintf( stderr, "%d extra bytes in output buffer\n",
					  extra );
				}
				memmove( compData, compData + OUTBLOCK, extra );
				compPtr -= OUTBLOCK;
				if( Debug >= 2 )
					fprintf( stderr, "compData = %p, compPtr = %p\n",
					  compData, compPtr );
			}
		}
	}

	// now write the remaining compressed data
	if( Debug )
		fprintf( stderr, "Writing %d bytes to stdout\n", (int) (compPtr - compData) );
	if( -1 == (lenWritten = write( STDOUT_FILENO, compData, compPtr - compData )))
	{
		perror( "write to STDOUT" );
		return 4;
	}
	else if( lenWritten != (compPtr - compData) )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n", lenWritten,
		  (int) (compPtr - compData) );
		return 5;
	}

	return 0;
}
