#include <assert.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "header.h"
#include "inet_helper.h"

#define MAXFDNUM 10000000
#define VIP_FMT "192.168.32.%d"

#define false (0)
#define true (1)

typedef char bool;

static bool fdvec[MAXFDNUM] = {false};
static char server_ip[32] = "0.0.0.0";
static unsigned server_port = 8888;
static const int threadnum = 10;

static int connect_it(const char *ip, unsigned port, const char *bindip);
static void thread_connect(void *param);

int main(int ac, char *av[])
{
	pthread_t ts[threadnum];
	if(ac < 3)
	{
		printf("need server address!\n");
		return -1;
	}
	strncpy(server_ip, av[1], sizeof(server_ip));
	server_port = (unsigned)atoi(av[2]);
	//
	for(int i = 0; i < threadnum; ++i)
	{
		if(pthread_create(&ts[i], NULL, &thread_connect, i) < 0)
		{
			err_sys("create thread failed");
			return -1;
		}
	}

	for(int i = 0; i < threadnum; ++i)
	{
		if(pthread_join(ts[i], NULL) < 0)
		{
			err_sys("join thread failed");
			return -1;
		}
	}
	printf("all connected!");
	sleep(10);
	return 0;
}

static void thread_connect(void *param)
{
	int seq = (int)param;
	for(int i = 0; i < 10; ++i)
	{
		char local_ip[32];
		sprintf(local_ip, VIP_FMT, seq*10+i+1);
		for(int j = 0; j < 50000; ++j)
		{
			int s = connect_it(server_ip, server_port, local_ip);
			if(s < 0)
			{
				err_sys("connect failed");
				return;
			}
			fdvec[s] = true;
		}
	}
}

static int connect_it(const char *ip, unsigned port, const char *bindip)
{
	int sock;
	struct sockaddr_in sin;
	int noport = 1;
	int freebind = 1;
	int dontroute = 1;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		err_sys("create socket failed");
		return -1;
	}
//	if(setsockopt(sock, IPPROTO_IP, IP_BIND_ADDRESS_NO_PORT, &noport, sizeof(noport)) < 0)
//	{
//		close(sock);
//		return -1;
//	}
// 	if(setsockopt(sock, IPPROTO_IP, IP_FREEBIND, &freebind, sizeof(freebind)) < 0)
// 	{
// 		close(sock);
// 		err_sys("setsockopt failed");
// 		return -1;
// 	}
	if(setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, &freebind, sizeof(freebind)) < 0)
	{
		close(sock);
		err_sys("setsockopt failed");
		return -1;
	}
	if(bindip)
	{
		struct sockaddr_in bsin;
		bsin.sin_family = AF_INET;
		bsin.sin_port = htons(0);
		bsin.sin_addr.s_addr = inet_addr(bindip);
		if(bind(sock, (struct sockaddr*)&bsin, sizeof(bsin)) < 0)
		{
			printf("address : %s\n", bindip);
			err_sys("bind address failed");
			return -1;
		}
	}
	if(connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		printf("dest address : %s\n", ip);
		err_sys("connect failed");
		close(sock);
		return -1;
	}
	return sock;
}