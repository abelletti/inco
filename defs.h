#define DEBUG 0

#define FLAG_UNCOMPRESSED 0
#define FLAG_LZJB 1

// chunk size to read
#define INBLOCK 1048576
// must be an even multiple of COMPBLOCK
#define READBLOCK 1048576
// output buffer size, we know it won't grow faster than compressor's COMPBLOCK
#define OUTBLOCK 1048576
#define COMPBLOCK 16384

#define OPTIONS "cdDh"
#define DEFAULT 0
#define COMPRESS 1
#define DECOMPRESS 2

#define COMPNAME "comp"
#define DECOMPNAME "decomp"
