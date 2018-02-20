#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


ssize_t readln(int fildes, void *buf, size_t nbyte)
{
	char* buff = buf;
	size_t i = -1;
	ssize_t n;
	do {
		i++;
		n = read(fildes,buff+i,1);
	} while((i < nbyte) && (n > 0) && (buff[i] != '\n'));
	n = i;
	exit(n);
}

int main(int argc,char** argv)
{
	char buffer[100];
	int fd,n;
	
	fd = open("texto.txt", O_RDONLY);
	do {
		n = readln(fd, buffer, 100);
	
		printf("A linha que li foi [%s]\n",buffer);
	
	} while (n > 0);
	close(fd);
	
	return 0;
}