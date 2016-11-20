#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

int number[] = {1, 3, 5};

int main(int argc, char *argv[])
{
	int host_id = atoi(argv[1]);
	int random_key = atoi(argv[3]);
	char player_index;
	sscanf(argv[2], "%c", &player_index);
	// FIFO name
	char player_FIFO[512], host_FIFO[512];
	sprintf(player_FIFO, "judge%d_%c.FIFO", host_id, player_index);
	sprintf(host_FIFO, "judge%d.FIFO", host_id);
	//open FIFO
	int FIFO_R = open(player_FIFO, O_RDONLY);
	int FIFO_W = open(host_FIFO, O_WRONLY);

	int cnt = 0;

	srand(time(NULL));

	char ret[512];
	while(cnt < 20){
		if (cnt != 0) {
			while (1) {
				fd_set R;
				FD_ZERO(&R);
				FD_SET(FIFO_R, &R);
				select(FIFO_R+1, &R, NULL, NULL, NULL);
				if (FD_ISSET(FIFO_R, &R)) {
					if (read(FIFO_R, ret, 512) > 0) {
						break;				
					}
				}
				fprintf(stderr, "fucking block read???\n");
			}
		}
		int r = (cnt+(player_index-'A')) % 3;
		sprintf(ret, "%c %d %d\n\0", player_index, random_key, number[r]);
		
		while (1) {
			fd_set W;
			FD_ZERO(&W);
			FD_SET(FIFO_W, &W);
			select(FIFO_W+1, NULL, &W, NULL, NULL);
			if (FD_ISSET(FIFO_W, &W)) {
				write(FIFO_W, ret, strlen(ret));
				break;
			}	
			fprintf(stderr, "fucking block write???\n");
		}
		cnt++;
	}
	close(FIFO_R);
	close(FIFO_W);
	fprintf(stderr, "Player %c dies\n", player_index);
	return 0;
}
