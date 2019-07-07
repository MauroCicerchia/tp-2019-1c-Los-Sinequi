#include<unistd.h>
#include<semaphore.h>

extern sem_t MUTEX_RETARDTIME;
extern int retardTime;

#ifndef DELAYER_H
#define DELAYER_H

void delayer();

#endif
