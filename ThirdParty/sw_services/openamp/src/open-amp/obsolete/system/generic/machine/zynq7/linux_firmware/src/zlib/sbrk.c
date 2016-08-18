
#define HEAP_SIZE   1024 * 512

extern unsigned int _heap_start;

static void *heap_end = 0;
static int bytes_used = 0;

void *_sbrk(int nbytes)
{
	void *prev_heap_end = (void *)-1;	/* Out of heap space */

	/* Check if it's first call to _sbrk() */
	if (heap_end == 0) {
		heap_end = &_heap_start;
	}

	/* Validate request */
	if (((bytes_used + nbytes) <= HEAP_SIZE)
	    && ((bytes_used + nbytes) >= 0)) {
		/* Request is in range, grant it */
		prev_heap_end = heap_end;

		/* Increment the heap_end pointer and bytes_used */
		heap_end += nbytes;
		bytes_used += nbytes;
	}

	return (prev_heap_end);
}

void *malloc(int nbytes)
{
	return _sbrk(nbytes);
}

void free(void *ptr)
{

}
