#include"delayer.h"

void delayer()
{
	sem_wait(&MUTEX_RETARDTIME);
//	int rt = get_retard_time();
	int rt = retardTime;
	sem_post(&MUTEX_RETARDTIME);
	usleep(rt * 1000);
}
