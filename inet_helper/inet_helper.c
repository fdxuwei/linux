#include "inet_helper.h"

#include <stdio.h>

int server_tcp(unsigned port)
{
	int sock;
	int reuseaddr = 1;
	int addr_reuse = 1;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock < 0)
		return -1;

	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &addr_reuse, sizeof(addr_reuse)) < 0)
	{
		close(socket);
		return -1;
	}

	if(bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		close(sock);
		return -1;
	}
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0)
	    return -1;
	if(listen(sock, 1000) < 0)
	{
		close(sock);
		return -1;
	}
	return sock;
}

int connect_tcp(const char *ip, unsigned port)
{
	int sock;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		return -1;
	if(connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		close(sock);
		return -1;
	}
	return sock;
}
