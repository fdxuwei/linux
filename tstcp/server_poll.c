#include "header.h"
#include "inet_helper.h"
#include "buffer.h"
#include <string.h>
#include <signal.h>
#include <poll.h>

#define READSIZE 300
#define MAXCLIENTNUM 2048

typedef struct
{
	BUFFER *rbuf;
}client_data_t;

typedef struct
{
	client_data_t clidata[MAXCLIENTNUM];
	struct pollfd fds[MAXCLIENTNUM];
	int clinum;
}client_list_t;

client_list_t *client_list_create()
{
	client_list_t *clp = (client_list_t *)malloc(sizeof(client_list_t));
	memset(clp->clidata, 0, sizeof(clp->clidata));
	clp->clinum = 0;
	return clp;
}

void client_list_release(client_list_t *clp)
{
	int n;
	if(NULL != clp)
	{
		for(n = 0; n < MAXCLIENTNUM; ++n)
		{
			if(NULL != clp->clidata[n].rbuf)
				buffer_release(clp->clidata[n].rbuf);
		}
		free(clp);
	}
}

void client_list_add(client_list_t *clp, int fd)
{
	clp->clidata[clp->clinum].rbuf = buffer_alloc();
	clp->fds[clp->clinum].fd = fd;
	clp->fds[clp->clinum].events = POLLIN | POLLOUT;
	++clp->clinum;
}

void client_list_remove(client_list_t *clp, int index)
{
	buffer_release(clp->clidata[index].rbuf);
	clp->clidata[index] = clp->clidata[clp->clinum-1];

	close(clp->fds[index].fd);
	clp->fds[index] = clp->fds[clp->clinum-1];
	--clp->clinum;
}

client_data_t *client_list_get(client_list_t *clp, int index)
{
	return &(clp->clidata[index]);
}

int main()
{
	int rlen, wlen;
	int ms, ss;
	int index;
	int connnum = 0;
	client_list_t *clsp;

	/* ignore SIGPIPE */
	if(signal(SIGPIPE, SIG_IGN) < 0)
		err_sys("signal");
	/**/
	ms = server_tcp(8881);
	if(ms < 0)
		err_sys("server_tcp");

	clsp = client_list_create();
	client_list_add(clsp, ms);
	while(poll(clsp->fds, clsp->clinum, -1) > 0)
	{
		if(clsp->fds[0].revents & POLLIN)
		{
			ss = accept(ms, 0, NULL);
			if(ss > 0)
			{
				client_list_add(clsp, ss);
				connnum++;
			}
			else
				printf("accept: %s\n", strerror(errno));
		}

		for(index = 1; index < clsp->clinum; ++index)
		{
			// read fds
			if(clsp->fds[index].revents & POLLIN)
			{
				BUFFER *bf = client_list_get(clsp, index)->rbuf;
				if((buffer_prepare(bf, READSIZE) == 0)
						&& (rlen = read(clsp->fds[index].fd, buffer_wpos(bf), READSIZE)) > 0)
				{
					buffer_commit(bf, rlen);
				}
				else
				{
					printf("closed: %s\n", strerror(errno));
					client_list_remove(clsp, index);
					--index;
				}
			}
		}

		/* write fd  */
		for(index = 1; index < clsp->clinum; ++index)
		{
			if(clsp->fds[index].revents & POLLOUT)
			{
				BUFFER *bf = client_list_get(clsp, index)->rbuf;
				if(buffer_datasize(bf) > 0)
				{		
					wlen = write(clsp->fds[index].fd, buffer_rpos(bf), buffer_datasize(bf));
					if(wlen > 0)
						buffer_consume(bf, wlen);
					else
					{
						client_list_remove(clsp, index);
						--index;
						printf("write: %s\n", strerror(errno));
					}
				}
			}
		}
	}
	printf("exit: %s\n", strerror(errno));
	return 0;
}

