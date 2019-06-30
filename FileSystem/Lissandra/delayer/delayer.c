#include"delayer.h"

void delayer()
{
	sem_wait(&MUTEX_RETARDTIME);
	int rt = retardTime;
	sem_post(&MUTEX_RETARDTIME);
	sleep(rt/1000);
}
