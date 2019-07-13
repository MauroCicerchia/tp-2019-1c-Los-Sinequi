#include"delayer.h"

void delayer()
{
	pthread_mutex_lock(&MUTEX_RETARDTIME);
//	int rt = get_retard_time();
	int rt = retardTime;
	pthread_mutex_unlock(&MUTEX_RETARDTIME);
	usleep(rt * 1000);
}
