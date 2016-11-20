/*b03902042 宋子維*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

const char* END_SIG = "-1 -1 -1 -1\n";
const int r2s[] = {3, 2, 1, 0};
char combination[5000][32];
static int com = 0;

typedef struct player{
	int id;
	int score;
	int rank;
} Player;

void generator(int *arr, int n, int index, int *data, int cur)
{
	if(index == 4){
		sprintf(combination[com], "%d %d %d %d\n", data[0], data[1], data[2], data[3]);
		com++;
		return;
	}

	if(cur >= n)
		return;

	data[index] = arr[cur];
	generator(arr, n, index+1, data, cur+1);
	generator(arr, n, index, data, cur+1);
}

int cmp(const void* a, const void* b)
{
	Player* p1 = (Player*) a;
	Player* p2 = (Player*) b;
	if (p1->score != p2->score) return p1->score > p2->score;
	else return p1->id < p2->id;
}

void handle_rank(const char *buffer, Player *p)
{
	Player t[4];
	sscanf(buffer, "%d %d\n%d %d\n%d %d\n%d %d\n", 
					&t[0].id, &t[0].rank, &t[1].id, &t[1].rank,
					&t[2].id, &t[2].rank, &t[3].id, &t[3].rank);

	for(int i = 0; i < 4; i++)
		p[t[i].id-1].score += r2s[t[i].rank-1];
}

int main(int argc, char *argv[])
{
	int host_num = atoi(argv[1]);
	int player_num = atoi(argv[2]);
	int pfd[16][2], pfd2[16][2]; // pfd: write for parent, pfd2: read for parent
	// initialize score of players
	Player p[32];
	for(int i = 0; i < player_num; i++){
		p[i].id = i;
		p[i].score = 0;
	}
	// fork child process to execute host
	int max_fdw = 0, max_fdr;
	pid_t pid;
	fd_set MASTER_R, MASTER_W;
	FD_ZERO(&MASTER_R);
	FD_ZERO(&MASTER_W);

	for(int i = 0; i < host_num; i++){
		pipe(pfd[i]);
		pipe(pfd2[i]);
		if((pid = fork()) == 0){
			dup2(pfd[i][0], 0);
			close(pfd[i][0]);
			dup2(pfd2[i][1], 1);
			close(pfd2[i][1]);
			
			close(pfd[i][1]);
			close(pfd2[i][0]);

			char host_temp[1024];
			sprintf(host_temp, "%d", i+1);
			execlp("./judge", "./judge", host_temp, NULL);
		}
		else{
			close(pfd[i][0]);
			close(pfd2[i][1]);
			// find maximum fd;
			FD_SET(pfd[i][1], &MASTER_W);
			if(max_fdw < pfd[i][1])
				max_fdw = pfd[i][1];
			if(max_fdr < pfd2[i][0])
				max_fdr = pfd2[i][0];
		}
	}
	// generate all combinations
	int total_read = 0, total_write = 0, total_com = 1;

	for(int i = 0; i < 4; i++)
		total_com *= (player_num - i);
	total_com /= 24;

	int arr[32];
	for(int i = 0; i < player_num; i++)
		arr[i] = i+1;

	int data[4];
	generator(arr, player_num, 0, data, 0);
	// i/o multiplexing on pipe
	fd_set read_fd, write_fd;

	while(total_read < total_com){
		write_fd = MASTER_W;
		select(max_fdw + 1, NULL, &write_fd, NULL, NULL);
		// parent can write into pipe
		for(int i = 0; i < host_num && total_write < total_com; i++){
			if(FD_ISSET(pfd[i][1], &write_fd)){
				write(pfd[i][1], combination[total_write], strlen(combination[total_write]));
				fsync(pfd[i][1]);
				FD_CLR(pfd[i][1], &MASTER_W);
				FD_SET(pfd2[i][0], &MASTER_R);
				total_write++;
			}
		}
		read_fd = MASTER_R;
		select(max_fdr + 1, &read_fd, NULL, NULL, NULL);
		// parent can read from pipe
		for(int i = 0; i < host_num && total_read < total_com; i++){
			if(FD_ISSET(pfd2[i][0], &read_fd)){
				char buffer[1024];
				if(read(pfd2[i][0], buffer, 1024) > 0){
					handle_rank(buffer, p);
					FD_SET(pfd[i][1], &MASTER_W);
					FD_CLR(pfd2[i][0], &MASTER_R);
					total_read++;
				}
			}
		}
	}

	for(int i = 0; i < host_num; i++){
		write(pfd[i][1], END_SIG, strlen(END_SIG));
		fsync(pfd[i][1]);
		close(pfd[i][1]);
		close(pfd2[i][0]);
	}

	qsort(p, player_num, sizeof(Player), cmp);

	for(int i = 0; i < player_num; i++)
		printf("%d %d\n", p[i].id, p[i].score);

	for(int i = 0; i < host_num; i++)
		wait(NULL);

	return 0;
}
