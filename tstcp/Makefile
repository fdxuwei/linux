CC=gcc
CFLAGS=-I../inet_helper -I../buffer -I../ -g
LDFLAGS=-L../inet_helper -L../buffer
LDLIBS=-linet_helper -lbuffer
EXEC= server_select server_poll server_epoll client
all: ${EXEC}
			      
clean:
	rm -f ${EXEC} *.o               
