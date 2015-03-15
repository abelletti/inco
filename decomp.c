#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FLAG_UNCOMPRESSED 0
#define FLAG_LZJB 1

#define DEBUG 1

// chunk size to read
#define INBLOCK 1048576	
// output buffer size, we know it won't grow faster than compressor's COMPBLOCK
#define OUTBLOCK 1048576
#define COMPBLOCK 16384

int main( int argc, char *argv[] )
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

	// leave room for partial COMPBLOCK plus flag so we can always read full INBLOCK
	if( NULL == (compData = malloc( INBLOCK + COMPBLOCK + 1 )))
	{
		perror( "malloc compData" );
		exit( 2 );
	}

	// enough output buffer space to always finish one more COMPBLOCK
	if( NULL == (outData = malloc( OUTBLOCK + COMPBLOCK )))
	{
		perror( "malloc outData" );
		exit( 3 );
	}

	outEnd = outData + OUTBLOCK;
	outPtr = outData;
	compPtr = compData;

	lenRead = read( STDIN_FILENO, compPtr, INBLOCK );
#if DEBUG
	fprintf( stderr, "DC: read %d bytes\n", lenRead );
#endif
	if( lenRead == 0 )
	{	// no input means no output
		exit( 0 );
	}
	lenRemaining = lenRead;

	while( lenRemaining > 0 )	
	{
		if( *(uchar_t *)compPtr == FLAG_LZJB )
		{
			lenDecomp = decompress( compPtr+1, outPtr, lenRemaining-1, COMPBLOCK,
			  &lenUsed );
#if DEBUG
			fprintf( stderr, "DC: expanded to %d bytes\n", lenDecomp );
			fprintf( stderr, "DC: used %d compressed bytes\n", lenUsed );
#endif
			compPtr += (lenUsed+1);
			outPtr += lenDecomp;
			lenRemaining -= (lenUsed+1);
		}
		else if( *(uchar_t *)compPtr == FLAG_UNCOMPRESSED )
		{
			int size = lenRemaining - 1;
			if( size > COMPBLOCK )
				size = COMPBLOCK;
			memcpy( outPtr, compPtr+1, size );
#if DEBUG
			fprintf( stderr, "DC: copied %d bytes as-is\n", size );
#endif
			compPtr += (size+1);
			outPtr += size;
			lenRemaining -= (size+1);
		}
		else
		{
			fprintf( stderr, "ALERT!  Unknown format %d encountered\n",
			  *(uchar_t *)compPtr );
			exit( 1 );
		}

		// much of this logic isn't necessary given that we know we'll expand to
		// COMPBLOCK each time; keeping for flexibility, for now

		if( (outEnd - outPtr) < COMPBLOCK )
		{	// we may not have enough room for another expanded block, gotta flush
			lenWritten = write( STDOUT_FILENO, outData, OUTBLOCK);
#if DEBUG
			fprintf( stderr, "DC: Wrote %d bytes\n", lenWritten );
#endif
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
			// now move what's left to beginning of next OUTBLOCK
#if DEBUG
			fprintf( stderr, "DC: outData = %x, outPtr = %x, outPtr-outEnd = %d\n",
				outData, outPtr, (outEnd - outPtr));
#endif
			memmove( outData, outPtr, (outEnd - outPtr));
			outPtr -= OUTBLOCK;
#if DEBUG
			fprintf( stderr, "DC: outData = %x, outPtr = %x, outPtr-outEnd = %d\n",
				outData, outPtr, (outEnd - outPtr));
#endif
		}

		if( lenRemaining < (COMPBLOCK+1) )
		// we might not have enough to decompress a block, try reading more
		{
#if DEBUG
			fprintf( stderr, "DC: lenRemaining = %d\n", lenRemaining );
#endif
			// first must move the remainder to start of buffer
			memmove( compData, compPtr, lenRemaining );
			compPtr = compData;

			lenRead = read( STDIN_FILENO, compPtr + lenRemaining, INBLOCK );
#if DEBUG
			fprintf( stderr, "DC: read %d bytes\n", lenRead );
#endif
			lenRemaining += lenRead;
		}
#if DEBUG
		fprintf( stderr, "DC: lenRemaining = %d\n", lenRemaining );
#endif
	}

	// now we must flush whatever is left in the output buffer
	lenWritten = write( STDOUT_FILENO, outData, (outPtr - outData));
	if( lenWritten == -1 )
	{
		perror( "final write to STDOUT" );
		exit( 6 );
	}
	else if( lenWritten != (outPtr - outData) )
	{
		fprintf( stderr, "Short write!  Wrote %d, had %d\n", lenWritten,
		  (outPtr - outData) );
		exit( 7 );
	}

	return 0;
}

