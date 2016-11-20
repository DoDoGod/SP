/*b03902042 宋子維*/
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
			read(FIFO_R, ret, 512);
		}
		int r = rand() % 3;
		sprintf(ret, "%c %d %d\n", player_index, random_key, r);
		write(FIFO_W, ret, strlen(ret));
	}

	close(FIFO_R);
	close(FIFO_W);
	
	return 0;
}
