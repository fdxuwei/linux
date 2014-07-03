#include <sys/socket.h>
#include <arpa/inet.h>

int server_tcp(unsigned port);
int connect_tcp(const char *ip, unsigned port);

