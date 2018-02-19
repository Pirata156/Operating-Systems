#include <stdlib.h>
#include <unistd.h>


ssize_t readln(int fildes, void *buf, size_t nbyte)
{
	int i = -1;
	ssize_t n;
	do {
		i++;
		n = read(fildes,&buf[i],1);
	} while((i < nbyte) && (n > 0) && (((char) buf[i]) != '\n'))
	n = i;
	exit(n);
}