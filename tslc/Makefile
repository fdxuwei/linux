CC=gcc
CFLAGS=-I../inet_helper -I../buffer -I../ -g -std=c99 -w
LDFLAGS=-L../inet_helper -L../buffer
LDLIBS=-linet_helper -lbuffer -pthread
EXEC= server client
all: ${EXEC}
			      
clean:
	rm -f ${EXEC} *.o               
