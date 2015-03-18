// (c) 2015 Allen R. Belletti

size_t compress( void *s_start, void *d_start, size_t s_len );

size_t decompress( void *s_start, void *d_start, size_t s_len,
		   size_t d_len, size_t *s_used );
