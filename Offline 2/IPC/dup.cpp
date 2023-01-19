#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<time.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

using namespace std;

#define CYCLIST_NUMBER 10  // input: cyclist number (N)
#define SERVICEMAN_NUMBER 3  // input: serviceman number (S)
#define PAYROOM_CAPACITY 2 // input: payment room capacity (C)
#define GLOVAR_NUMBER 2
#define ADDITIONAL_LOCKS 3

int in_service=0, in_waiting=0;

sem_t payroom_booths;
pthread_mutex_t servicing_rooms[SERVICEMAN_NUMBER], global_variables[GLOVAR_NUMBER], locks[ADDITIONAL_LOCKS];

void* visit_cycle_repairing_shop(void* arg) {
	/* assigning id to local id & freeing allocated memory & assigning NULL to avoid dangling pointer */
	int id = *((int*)arg);
	// cout<<"current id:"<<id<<endl;
	delete (int*)arg;
	arg = NULL;
	
	/* part1: taking services in servicing_rooms */
	pthread_mutex_lock(&locks[0]);  // down(L)
	pthread_mutex_lock(&locks[1]);  // down(N)
	pthread_mutex_lock(&servicing_rooms[0]);  // down(M)

	pthread_mutex_lock(&global_variables[0]);  // mutex-block: in_service
	in_service++;
	if(in_service == 1) {
		pthread_mutex_lock(&locks[2]);  // down(mutex-LEAVE)
	}
	pthread_mutex_unlock(&global_variables[0]);

	printf("%d started taking service from serviceman %d\n", id, 1);
	fflush(stdout);
	pthread_mutex_unlock(&locks[1]);  // up(N)
	usleep((rand()%1001)*1000);
	printf("%d finished taking service from serviceman %d\n", id, 1);
	fflush(stdout);

	if(SERVICEMAN_NUMBER != 1) {
		pthread_mutex_lock(&servicing_rooms[1]);
	} 
	pthread_mutex_unlock(&servicing_rooms[0]);  // up(M)
	pthread_mutex_unlock(&locks[0]);  // up(L)

	for(int i=1; i<SERVICEMAN_NUMBER; i++) {
		printf("%d started taking service from serviceman %d\n", id, i+1);
		fflush(stdout);
		usleep((rand()%1001)*1000);
		printf("%d finished taking service from serviceman %d\n", id, i+1);
		fflush(stdout);

		if(i != SERVICEMAN_NUMBER-1) {
			pthread_mutex_lock(&servicing_rooms[i+1]);
		} 
		pthread_mutex_unlock(&servicing_rooms[i]);
	}
	pthread_mutex_lock(&global_variables[0]);  // mutex-block: in_service
	in_service--;
	if(in_service == 0) {
		pthread_mutex_unlock(&locks[2]);  // up(mutex-LEAVE)
	}
	pthread_mutex_unlock(&global_variables[0]);

	/* part2: paying service bill in payment_room */
	sem_wait(&payroom_booths);
	printf("%d started paying the service bill\n", id);
	fflush(stdout);
	usleep((rand()%1001)*1000);

	pthread_mutex_lock(&global_variables[1]);  // mutex-block: in_waiting
	if(in_waiting == 0) {
		pthread_mutex_lock(&locks[1]);  // down(N)
	}
	in_waiting++;
	pthread_mutex_unlock(&global_variables[1]);

	printf("%d finished paying the service bill\n", id);
	fflush(stdout);
	sem_post(&payroom_booths);

	/* part3: departing from cycle_repairing_shop */
	pthread_mutex_lock(&servicing_rooms[0]);  // down(M)
	pthread_mutex_lock(&locks[2]);  // down(mutex-LEAVE)

	usleep((rand()%1001)*1000);
	printf("%d has departed\n", id);
	fflush(stdout);

	pthread_mutex_lock(&global_variables[1]);  // mutex-block: in_waiting
	if(in_waiting == 1) {
		pthread_mutex_unlock(&locks[1]);  // up(N)
	}
	in_waiting--;
	pthread_mutex_unlock(&global_variables[1]);

	pthread_mutex_unlock(&locks[2]);  // up(mutex-LEAVE)
	pthread_mutex_unlock(&servicing_rooms[0]);  // up(M)
}

int main(int argc, char* argv[]) {
	int return_value;
	srand(time(0));

	/* initializing semaphore and locks(mutex) */

	return_value = sem_init(&payroom_booths, 0, PAYROOM_CAPACITY);
	if(return_value != 0) {
		printf("payroom_booths semaphore initialization failed\n");
		fflush(stdout);
	}
	for(int i=0; i<SERVICEMAN_NUMBER; i++) {
		return_value = pthread_mutex_init(&servicing_rooms[i], NULL);
		if(return_value != 0) {
			printf("servicing_rooms[%d] mutex initialization failed\n", i);
			fflush(stdout);
		}
	}
	for(int i=0; i<GLOVAR_NUMBER; i++) {
		/* 0: in_service & 1: in_waiting */
		return_value = pthread_mutex_init(&global_variables[i], NULL);
		if(return_value != 0) {
			printf("global_variables[%d] mutex initialization failed\n", i);
			fflush(stdout);
		}
	}
	for(int i=0; i<ADDITIONAL_LOCKS; i++) {
		/* 0: mutex-L & 1: mutex-N & 2: mutex-LEAVE */
		return_value = pthread_mutex_init(&locks[i], NULL);
		if(return_value != 0) {
			printf("locks[%d] mutex initialization failed\n", i);
			fflush(stdout);
		}
	}

	/* creating cyclist threads */

	pthread_t cyclists[CYCLIST_NUMBER];
	for(int i=0; i<CYCLIST_NUMBER; i++) {
		int* id = new int(i+1);

		return_value = pthread_create(&cyclists[i], NULL, visit_cycle_repairing_shop, (void*)id);
		if(return_value != 0) {
			printf("cyclists[%d] thread creation failed\n", i);
			fflush(stdout);
		}
	}

	/* waiting for cyclist threads to finish */

	for(int i=0; i<CYCLIST_NUMBER; i++) {
		return_value = pthread_join(cyclists[i], NULL);
		if(return_value != 0) {
			printf("cyclists[%d] thread join failed\n", i);
			fflush(stdout);
		}
	}

	/* uninitializing semaphores and locks(mutex) */

	return_value = sem_destroy(&payroom_booths);
	if(return_value != 0) {
		printf("payroom_booths semaphore uninitialization failed\n");
		fflush(stdout);
	}
	for(int i=0; i<SERVICEMAN_NUMBER; i++) {
		return_value = pthread_mutex_destroy(&servicing_rooms[i]);
		if(return_value != 0) {
			printf("servicing_rooms[%d] mutex uninitialization failed\n", i);
			fflush(stdout);
		}
	}
	for(int i=0; i<GLOVAR_NUMBER; i++) {
		return_value = pthread_mutex_destroy(&global_variables[i]);
		if(return_value != 0) {
			printf("global_variables[%d] mutex uninitialization failed\n", i);
			fflush(stdout);
		}
	}
	for(int i=0; i<ADDITIONAL_LOCKS; i++) {
		return_value = pthread_mutex_destroy(&locks[i]);
		if(return_value != 0) {
			printf("locks[%d] mutex uninitialization failed\n", i);
			fflush(stdout);
		}
	}

	return 0;
}