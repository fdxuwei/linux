#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <sys/types.h>

typedef void BUFFER;

/* allocate a buffer.
   on success, return the BUFFER object, otherwise return NULL.  */
BUFFER *buffer_alloc();

/* release BUFFER object, bf must be a value returned by buffer_alloc.  */
void buffer_release(BUFFER *bf);

/* get read position of the buffer for read. */
const void *buffer_rpos(BUFFER *bf);

/* get write potition of buffer for write. */
void *buffer_wpos(BUFFER *bf);

/* prepare size bytes of empty space for write
   on success, return 0, on error, return -1. */
int buffer_prepare(BUFFER *bf, size_t size);

/* consume size bytes of data in the buffer.
   it moves read pointer forward by size bytes.
  it often be called after a read by buffer_rpos. */
void buffer_consume(BUFFER *bf, size_t size);

/* commit size bytes of data to the buffer.
 it moves write pointer forward by size bytes.
 it often be called after a write by buffer_wpos. */
void buffer_commit(BUFFER *bf, size_t size);

/* return the size of data in the buffer */
size_t buffer_datasize(BUFFER *bf);

#endif

