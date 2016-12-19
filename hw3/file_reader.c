#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LEN 1024

int main(int argc, char ** argv)
{
#ifdef SLEEP
	sleep(10);
#endif
	char filename[MAX_LEN];

	read(0, filename, MAX_LEN);

	int stat;
	if ((stat = access(filename, F_OK)) == -1) {
		printf("file %s does not exist.\n", filename);
		exit(2);
	}
	else if ((stat = access(filename, R_OK)) == -1) {
		printf("file %s read permission denied.\n", filename);
		exit(1);
	}
	else {
		int fd = open(filename, O_RDONLY);
		int nbytes;
		char buf[MAX_LEN];
		while (1) {
			if ((nbytes = read(fd, buf, MAX_LEN)) == -1) {
				perror("read failed");
				exit(11);
			}
			if (nbytes == 0) break;
			write(1, buf, nbytes);
		}
	}

	return 0;	
}
