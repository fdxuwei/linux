#include "header.h"
#include "inet_helper.h"
#include "buffer.h"
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>

#define READSIZE 200
#define MAXCLIENTNUM 12000
#define MAXEVENTS 1000

typedef struct
{
	BUFFER *rbuf;
}client_data_t;

typedef struct
{
	client_data_t clidata[MAXCLIENTNUM];
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
	clp->clidata[fd].rbuf = buffer_alloc();
}

void client_list_remove(client_list_t *clp, int fd)
{
	close(fd);
	buffer_release(clp->clidata[fd].rbuf);
	clp->clidata[fd].rbuf = NULL;
}

client_data_t *client_list_get(client_list_t *clp, int fd)
{
	return &(clp->clidata[fd]);
}

int run = 1;

void sig_int(int signo)
{
	run = 0;
}

int main()
{
	int rlen, wlen;
	int ms, ss;
	int connnum = 0;
	int epollfd;
	struct epoll_event ev, evs[MAXEVENTS];
	int evnum, n;
	client_list_t *clsp;

	clsp = client_list_create();
	epollfd = epoll_create(1);
	if(epollfd < 0)
		err_sys("epoll_create");
	/* ignore SIGPIPE */
	if(signal(SIGPIPE, SIG_IGN) < 0)
		err_sys("signal");
	if(signal(SIGINT, sig_int) < 0)
		err_sys("signal");
	/**/
	ms = server_tcp(8881);
	if(ms < 0)
		err_sys("server_tcp");
	
	if(fcntl(ms, F_SETFL, O_NONBLOCK) < 0)
		err_sys("fcntl");

	ev.events = EPOLLIN ;
	ev.data.fd = ms;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, ms, &ev) < 0)
		err_sys("epoll_ctl");
	while((evnum = epoll_wait(epollfd, evs, MAXEVENTS, -1)) > 0)
	{
		if(!run)
			break;
		for(n = 0; n < evnum; ++n)
		{
			if(evs[n].data.fd == ms)
			{
				while((ss = accept(ms, 0, NULL)) > 0)
				{
					if(fcntl(ss, F_SETFL, O_NONBLOCK) < 0)
						err_sys("fcntl");
					ev.data.fd = ss;
					ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
					if(epoll_ctl(epollfd, EPOLL_CTL_ADD, ss, &ev) == 0)
					{
						client_list_add(clsp, ss);
						connnum++;
/*						printf("new connection, %d\n", connnum);*/
					}
					else
						err_sys("epoll_ctl");
				}
				if((ss < 0) && (errno != EAGAIN))
					err_sys("accept");
				evs[n].events = 0;
			}
			if(evs[n].events & EPOLLIN)
			{
				BUFFER *bf = client_list_get(clsp, evs[n].data.fd)->rbuf;
				while(buffer_prepare(bf, READSIZE) == 0)
				{
					rlen = read(evs[n].data.fd, buffer_wpos(bf), READSIZE);
					if(rlen > 0)
						buffer_commit(bf, rlen);
					else if(rlen < 0 && errno == EAGAIN)
						break;
					else
					{
						/*  rlen>0: buffer full; rlen=0, client closed. */
					
						/* close and remove the socket */
						if(epoll_ctl(epollfd, EPOLL_CTL_DEL, evs[n].data.fd, NULL) < 0)
							err_sys("epoll_ctl");
						printf("read: %s\n", strerror(errno));
						client_list_remove(clsp, evs[n].data.fd);
						evs[n].events = 0; /* to skip next test of EPOLLOUT */
						break;
					}
				}
				
			}
			if(evs[n].events & EPOLLOUT)
			{
				BUFFER *bf = client_list_get(clsp, evs[n].data.fd)->rbuf;
				while(buffer_datasize(bf) > 0)
				{
					wlen = write(evs[n].data.fd, buffer_rpos(bf), buffer_datasize(bf));
					if(wlen > 0)	
						buffer_consume(bf, wlen);
					else if(wlen < 0 && errno == EAGAIN)
						break;
					else
					{
						/* wlen==0: client closed;  */
						if(epoll_ctl(epollfd, EPOLL_CTL_DEL, evs[n].data.fd, NULL) < 0)
							err_sys("epoll_ctl");
						client_list_remove(clsp, evs[n].data.fd);
						printf("write: %s\n", strerror(errno));
						break;
					}
				}
			}
		}

	}

	for(n = 0; n < MAXCLIENTNUM; ++n)
	{
		BUFFER *bf = client_list_get(clsp, n)->rbuf;
		if(NULL != bf && buffer_datasize(bf) != 0)
		{
			printf("%u, ", buffer_datasize(bf));
		}
	}

	client_list_release(clsp);
	close(epollfd);
	err_sys("exit");
	return 0;
}

