#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

static void err_sys(const char *arg)
{
	printf("%s: %s\n", arg, strerror(errno));
	exit(1);
}

