#include "header.h"
#include "buffer.h"

int main()
{
	int n;
	size_t len;
	BUFFER *bf = buffer_alloc();
	for(n = 0; n < 10000; ++n)
	{
	printf("%d: ", n);
	fflush(NULL);
	if(buffer_prepare(bf, 5) < 0)
	{
		printf("prepare failed, datasize=%d\n", buffer_datasize(bf));
		break;
	}
	memcpy(buffer_wpos(bf), "xuwei", 5);
	buffer_commit(bf, 5);
	if(buffer_prepare(bf, 100) < 0)
	{
		printf("prepare failed, datasize=%d\n", buffer_datasize(bf));
		break;
	}
	memcpy(buffer_wpos(bf), " is a great man in china!", 24);
	buffer_commit(bf, 24);
	if(buffer_datasize(bf) >= 10)
	{
		write(STDOUT_FILENO, buffer_rpos(bf), 10);
		buffer_consume(bf, 10);
	}
	write(STDOUT_FILENO, buffer_rpos(bf), buffer_datasize(bf));
	buffer_consume(bf, buffer_datasize(bf));
	printf("\n");
	}
	buffer_release(bf);
	return 0;
}

