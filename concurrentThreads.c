#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t game_thread, time_thread;
int game_return, time_return;

struct socketThread {
	pthread_t thread;
	int inUse;
};

struct socketThread socketThreads[5];


void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

void *PlayGame ()
{
	int input1, input2;
   	printf("enter the first integer:\n");
   	scanf("%d",&input1);
	printf("enter the second integer:\n");
	scanf("%d", &input2);
	//waitFor(5);
	printf("sum of %d and %d is: %d", input1, input2, input1 + input2);
	pthread_mutex_unlock(&myMutex);
}




int main()
{
	int threadCount;

//	while (1) {
		int i;
		for (i = 0; i < 5; ++i) {
			if (!socketThreads[i].inUse) {
				pthread_mutex_lock(&myMutex);
				socketThreads[i].inUse = 1;
				game_return = pthread_create(&socketThreads[i].thread, NULL, PlayGame, NULL);
			}
			else {
				threadCount++;
				//printf("Thread %d is in use\n", i);
			}
		}

		pthread_join(socketThreads[i].thread,NULL);
		socketThreads[i].inUse = 0;
//	}

	printf("Program over\n");
	return 0;
}
