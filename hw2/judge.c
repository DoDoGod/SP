/*b03902042 宋子維*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct player{
	int id;
	int score;
	int rank;
} Player;

void handle_ret(char* ret, int* num)
{
	puts(ret);
	char buf[512];
	int check[4] = {0};
	int j = 0;
	for (int i = 0; ret[i] != '\0'; i++) {
		buf[j++] = ret[i]; 
		if (ret[i] == '\n') {
			char player;
			int number, r;
			buf[j++] = '\0';
			sscanf(buf, "%c %d %d", &player, &r, &number);
			if (num[player-'A'] != 0)
				num[player - 'A'] = number;
			check[player-'A'] = number;
			j = 0;
		}
	}
	
	memcpy(num, check, sizeof(num));

	sprintf(ret, "%d %d %d %d\n", num[0], num[1], num[2], num[3]);
	return;
}

void handle_score(int* num, Player* p)
{
	int S[16] = {0};
	for (int i = 0; i < 4; i++) {
		S[num[i]]++;
	}
	for (int i = 0; i < 3; i++) {
		if (S[2*i+1] == 1) {
			for (int j = 0; j < 4; j++) {
				if (num[j] == 2*i+1) {
					p[j].score += num[j];
				}
			}
		}
	}
	return;
}

void bubble_sort(Player* p)
{
	Player tmp;
	for (int i = 3; i > 0; i++)
		for (int j = 0; j < i; j++) {
			if (p[j].score < p[j+1].score) {
				tmp = p[j];
				p[j] = p[j+1];
				p[j+1] = tmp;
			}
		}
}

int main(int argc, char *argv[])
{
	int host_id = atoi(argv[1]);
	// variables for FIFO name
	char response[512];
	char mess[4][512];
	// create response FIFO
	sprintf(response, "judge%d.FIFO", host_id);
	for(int i = 0; i < 4; i++)
		sprintf(mess[i], "judge%d_%c.FIFO", host_id, 'A'+i);
	// random key
	srand(time(NULL));

	while(1){
		// read from pipe of bidding system(stdin)
		char p_buf[512];
		read(0, p_buf, 512);
		Player p[4];
		sscanf(p_buf, "%d %d %d %d\n", &p[0].id, &p[1].id, &p[2].id, &p[3].id);
		if(p[0].id == -1 && p[1].id == -1 && p[2].id == -1 && p[3].id == -1)
			break;
		mkfifo(response, 0777);
		// fork to execute player
		int FIFO_W[4];
		pid_t pid;
		for(int i = 0; i < 4; i++){
			mkfifo(mess[i], 0777);
			int random = rand() % 65536;
			if((pid = fork()) == 0){
				char player_index[4];
				char random_str[16];
				sprintf(random_str, "%d", random);
				sprintf(player_index, "%c", 'A'+i);
				execlp("./player", "./player", argv[1], player_index, random_str, NULL);
			}
			else{
				FIFO_W[i] = open(mess[i], O_WRONLY);
			}
		}
		// open read FIFO
		int FIFO_R = open(response, O_RDONLY);
		int cnt = 0, already_r = 0;
		int num[4] = {-1, -1, -1, -1};

		char ret[512];

		while(cnt < 20){
			if (cnt != 0) {
				for (int i = 0; i < 4; i++)
					write(FIFO_W[i], ret, strlen(ret));
			}
			sleep(3);
			read(FIFO_R, ret, 512);
			handle_ret(ret, num);
			handle_score(num, p);
			cnt++;
		}
		// wait for all children 
		for(int i = 0; i < 4; i++){
			wait(NULL);
		}
		// close and remove FIFO
		close(FIFO_R);
		unlink(response);
		for(int i = 0; i < 4; i++){
			close(FIFO_W[i]);
			unlink(mess[i]);
		}
		// deal with output
	
		bubble_sort(p);

		Player rank[16];
		int j = 1, pre = -1, k = 0;
		for (int i = 0; i < 4; i++) {
			if (pre == p[i].score)
				k++;
			else {
				j += k;
				k = 1;
			}
			rank[i].id = p[i].id;
			rank[i].rank = j;
			pre = p[i].score;
		}
	
		// write into pipe of bidding_system(stdout)
		char output[512];
		sprintf(output, "%d %d\n%d %d\n%d %d\n%d %d\n",
						rank[0].id, rank[0].rank, rank[1].id, rank[1].rank, 
						rank[2].id, rank[2].rank, rank[3].id, rank[3].rank);
		write(1, output, strlen(output));
		fsync(1);
	}

	return 0;
}
