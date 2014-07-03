//#include "buffer.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAX(x,y) ((x)>(y)?(x):(y))

typedef struct 
{
	char *data;
	size_t size;
	size_t readpos;
	size_t writepos;
}BUFFER;

static int bf_realloc(BUFFER *bf, size_t newsize);

BUFFER *buffer_alloc()
{
	BUFFER *pbf = (BUFFER*)malloc(sizeof(BUFFER));
	pbf->size = 400; /* initial size*/
	pbf->data = (char*)malloc(pbf->size);
	pbf->readpos = 0;
	pbf->writepos = 0;
	return pbf;
}

int bf_realloc(BUFFER *bf, size_t newsize)
{
	if(newsize > 5000)
		return -1;
	if(newsize <= bf->size)
		return 0;
	/* multiply 5/4 to ensure there is any other space after next fill */
	bf->size = MAX(bf->size*2, newsize*5/4);
	bf->data = (char*)realloc(bf->data, bf->size);

	return 0;
}

void buffer_release(BUFFER *bf)
{
	if(NULL != bf)
	{
		if(NULL != bf->data)
			free(bf->data);
		free(bf);
	}
}

const void *buffer_rpos(BUFFER *bf)
{
	return (bf->data + bf->readpos);
}

void *buffer_wpos(BUFFER *bf)
{
	return (bf->data + bf->writepos);
}

int buffer_prepare(BUFFER *bf, size_t size)
{
	size_t emptysz;
	size_t writablesz;
	writablesz = bf->size - bf->writepos;
	if(writablesz >= size)
		return 0;
	emptysz = bf->size + bf->readpos - bf->writepos;
	if((emptysz < size) && (bf_realloc(bf, bf->size + size - emptysz) < 0))
		return -1;
	writablesz = bf->size - bf->writepos;
	if(writablesz < size)
	{
		memmove(bf->data, bf->data+bf->readpos, bf->writepos-bf->readpos);
		bf->writepos = bf->writepos - bf->readpos;
		bf->readpos = 0;
	}
	return 0;
}

void buffer_consume(BUFFER *bf, size_t size)
{
	bf->readpos = bf->readpos + size;
}

void buffer_commit(BUFFER *bf, size_t size)
{
	bf->writepos = bf->writepos + size;
}

size_t buffer_datasize(BUFFER *bf)
{
	return (bf->writepos - bf->readpos);
}
