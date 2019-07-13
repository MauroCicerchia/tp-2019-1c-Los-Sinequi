#include<unistd.h>
#include<semaphore.h>

extern int retardTime;

extern sem_t MUTEX_RETARDTIME;

#ifndef DELAYER_H
#define DELAYER_H

void delayer();

#endif
