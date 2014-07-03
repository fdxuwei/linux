#include "header.h"
#include "inet_helper.h"
#include "buffer.h"
#include <string.h>
#include <signal.h>

#define READ_SIZE 300

typedef struct
{
	BUFFER *rbuf;
}client_data_t;

typedef struct
{
	client_data_t clidata[FD_SETSIZE];
}client_list_t;


client_list_t *client_list_create()
{
	client_list_t *clp = (client_list_t *)malloc(sizeof(client_list_t));
	memset(clp->clidata, 0, sizeof(clp->clidata));
	return clp;
}

void client_list_release(client_list_t *clp)
{
	int n;
	if(NULL != clp)
	{
		for(n = 0; n < FD_SETSIZE; ++n)
		{
			if(NULL != clp->clidata[n].rbuf)
				buffer_release(clp->clidata[n].rbuf);
		}
		free(clp);
	}
}

void client_list_add(client_list_t *clp, int fd)
{
	clp->clidata[fd].rbuf = buffer_alloc();
}

void client_list_remove(client_list_t *clp, int fd)
{
	buffer_release(clp->clidata[fd].rbuf);
}

client_data_t *client_list_get(client_list_t *clp, int fd)
{
	return &(clp->clidata[fd]);
}

int main()
{
	int rlen, wlen;
	int ms, ss;
	int fd;
	int connnum = 0;
	fd_set fds, rfds, wfds;
	client_list_t *clilistp;

	/* ignore SIGPIPE */
	if(signal(SIGPIPE, SIG_IGN) < 0)
		err_sys("signal");
	/**/
	ms = server_tcp(8881);
	if(ms < 0)
		err_sys("server_tcp");

	clilistp = client_list_create();
	
	FD_ZERO(&fds);
	FD_SET(ms, &fds);
	memcpy(&rfds, &fds, sizeof(fds));
	memcpy(&wfds, &fds, sizeof(fds));
	while(select(FD_SETSIZE, &rfds, &wfds, NULL, NULL) > 0)
	{
		if(FD_ISSET(ms, &rfds))
		{
			ss = accept(ms, 0, NULL);
			if(ss > 0)
			{
				FD_SET(ss, &fds);
				client_list_add(clilistp, ss);
				connnum++;
			}
			else
				printf("accept: %s\n", strerror(errno));
			FD_CLR(ms, &rfds);
		}

		for(fd = 0; fd < FD_SETSIZE; ++fd)
		{
			// read fds
			if(FD_ISSET(fd, &rfds))
			{
				BUFFER *bf = client_list_get(clilistp, fd)->rbuf;
				if(buffer_prepare(bf, READ_SIZE) == 0
						&& (rlen = read(fd, buffer_wpos(bf), READ_SIZE)) > 0)
				{
					buffer_commit(bf, rlen);
				}
				else
				{
					/* an error occored*/
					close(fd);
					FD_CLR(fd, &fds);
					FD_CLR(fd, &wfds);
					client_list_remove(clilistp, fd);
					printf("closed: %s\n", strerror(errno));
				}
			}

			// write fds
			if(FD_ISSET(fd, &wfds))
			{
				BUFFER *bf = client_list_get(clilistp, fd)->rbuf;
				if(buffer_datasize(bf) > 0)
				{		
					if((wlen = write(fd, buffer_rpos(bf), buffer_datasize(bf))) > 0)
						buffer_consume(bf, wlen);
					else
					{
						close(fd);
						FD_CLR(fd, &fds);
						client_list_remove(clilistp, fd);
						printf("write: %s\n", strerror(errno));
					}
				}
			}
		}
		memcpy(&rfds, &fds, sizeof(fds));
		memcpy(&wfds, &fds, sizeof(fds));
	}
	printf("exit: %s\n", strerror(errno));
	return 0;
}

