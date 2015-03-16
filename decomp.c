// (c) 2015 Allen R. Belletti

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "inco.h"

int decomp( void )
{
	void *compData;
	void *compPtr;
	void *outData;
	void *outPtr;
	void *outEnd;		// end of the output buffer
	int lenRead;
	int lenRemaining;
	int lenDecomp;
	int lenUsed;
	int lenWritten;
	int available;		// bytes of compressed data available in compData

	// leave room for partial COMPBLOCK plus flag so we can always read full INBLOCK
	if( NULL == (compData = malloc( INBLOCK + COMPBLOCK + 1 )))
	{
		perror( "malloc compData" );
		return 2;
	}

	// enough output buffer space to always finish one more COMPBLOCK
	// we know they'll all decompress to a COMPBLOCK in size, which
	// should divide evenly into OUTBLOCK, but to be flexible for future
	// changes, we'll do it this way.
	if( NULL == (outData = malloc( OUTBLOCK + COMPBLOCK )))
	{
		perror( "malloc outData" );
		return 3;
	}

	outEnd = outData + OUTBLOCK; // first byte past end of outData buffer
	outPtr = outData; // where to write next decompressed COMPBLOCK in outData
	compPtr = compData;
	available = 0;

	for( ;; ) {
		// now we'll start reading compressed data from STDIN
		// Trying to read an INBLOCK but will settle for at least
		// a COMPBLOCK+1 since that guarantees enough for a
		// successful decompress (in the longest case, which will
		// be a flag byte followed by an uncompressed block

		do {
			if( -1 == (lenRead = read( STDIN_FILENO, compPtr, INBLOCK - available)))
			{
				// something has gone badly wrong
				perror( "read from STDIN" );
				return 4;
			}
#if DEBUG
			fprintf( stderr, "DC: read %d when requesting %d\n", lenRead,
			  INBLOCK - available );
#endif
			if( !lenRead )
			{
				// EOF, we're done reading!
				break;
			}
			available += lenRead;
			compPtr += lenRead;
		} while( available < (COMPBLOCK + 1));

		if( !available )
			break;	// we've come back to read more but gotten nothing; EOF
		
		// when we get here, compData is guaranteed to point to at least
		// COMPBLOCK+1 bytes of data UNLESS we've hit EOF after a small read
		// available counts how many bytes of compressed data we have
		// those bytes start at compData for now

		compPtr = compData;
		do {
#if DEBUG
			fprintf( stderr, "DC: coming into decompress, available=%d\n",
			  available );
#endif
			if( *(uchar_t *)compPtr == FLAG_LZJB )
			{
				lenDecomp = decompress( compPtr+1, outPtr, available-1,
				  COMPBLOCK, &lenUsed );
#if DEBUG
				fprintf( stderr, "DC: expanded to %d bytes\n", lenDecomp );
				fprintf( stderr, "DC: used %d compressed bytes\n", lenUsed );
#endif
				compPtr += (lenUsed+1);
				outPtr += lenDecomp;
				available -= (lenUsed+1);
			}
			else if( *(uchar_t *)compPtr == FLAG_UNCOMPRESSED )
			{
				int size = available - 1;
				if( size > COMPBLOCK )
					size = COMPBLOCK;
				memcpy( outPtr, compPtr+1, size );
#if DEBUG
				fprintf( stderr, "DC: copied %d bytes as-is\n", size );
#endif
				compPtr += (size+1);
				outPtr += size;
				available -= (size+1);
			}
			else
			{
				fprintf( stderr, "FAIL: unknown format 0x%02x encountered\n",
				  *(uchar_t *)compPtr );
				return 1;
			}
#if DEBUG
			fprintf( stderr, "available = %d\n", available );
#endif
			// now, is outData getting full enough that we need to flush?

			if( (outEnd - outPtr) < COMPBLOCK )
			{	// we may not have enough room for another expanded block, gotta flush
				lenWritten = write( STDOUT_FILENO, outData, OUTBLOCK);
#if DEBUG
				fprintf( stderr, "DC: Wrote %d bytes\n", lenWritten );
#endif
				if( lenWritten == -1 )
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
				// now move what's left to beginning of next OUTBLOCK
#if DEBUG
				fprintf( stderr,
				  "DC: outData = %x, outPtr = %x, outPtr-outEnd = %d\n",
				  outData, outPtr, (outEnd - outPtr));
#endif
				memmove( outData, outPtr, (outEnd - outPtr));
				outPtr -= OUTBLOCK;
#if DEBUG
				fprintf( stderr,
				  "DC: outData = %x, outPtr = %x, outPtr-outEnd = %d\n",
				  outData, outPtr, (outEnd - outPtr));
#endif
			}
		} while( available >= (COMPBLOCK +1 ));

		// unless available==0, move that data back to start of compData
		memmove( compData, compPtr, available );
		compPtr = compData + available;
	}

	// done reading and decompressing but may have something left in the
	// output buffer; write it.

	if( -1 == (lenWritten = write( STDOUT_FILENO, outData, (outPtr - outData))))
	{
		perror( "final write to STDOUT" );
		return 6;
	}
	else if( lenWritten != (outPtr - outData) )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n", lenWritten,
		  (outPtr - outData) );
		return 7;
	}

	return 0;
}
