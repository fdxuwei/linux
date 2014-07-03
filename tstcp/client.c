#include "header.h"
#include "inet_helper.h"

int main()
{
	int s;
	int i;
	for(i = 0; i < 1; i++)
	{
	s = connect_tcp("127.0.0.1", 8881);
	if(s < 0)
		err_sys("connect_tcp");
	printf("connect success\n");
	}
	sleep(10);
	return 0;
}

