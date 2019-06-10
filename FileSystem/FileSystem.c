#include "FileSystem.h"

t_list *memtable;
char *absoluto;
t_log *logger;
int fd,wd,dumpTime,retardTime;
pthread_t tApi,tDump,tListenCfg;
t_config *config;
sem_t MUTEX_MEMTABLE, MUTEX_RETARDTIME, MUTEX_DUMPTIME;

int main(int argc, char **argv)
{
	init_FileSystem();

	pthread_create(&tListenCfg,NULL,threadConfigModify,NULL);
	pthread_detach(tListenCfg);

	pthread_create(&tDump,NULL,threadDump,NULL);
	pthread_detach(tDump);

	start_Api();

	kill_FileSystem();

	return 0;
}

void init_FileSystem()
{
	memtable = list_create();

	absoluto = string_new();

	logger = NULL;
	iniciar_logger(&logger);

	fd = inotify_init();
	wd = inotify_add_watch(fd,"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Config",IN_MODIFY);

	load_config();

	dumpTime = get_dump_time();
	retardTime = get_retard_time();
	absoluto = string_new();
	char *x = get_fs_route();
	string_append(&absoluto,x);
	free(x);
//	config_destroy(config);

	sem_init(&MUTEX_MEMTABLE,1,1);
	sem_init(&MUTEX_DUMPTIME,1,1);
	sem_init(&MUTEX_RETARDTIME,1,1);
}


void kill_FileSystem()
{
	log_info(logger, "----------------------------------------");
	log_info(logger, "Dump de seguridad");

	sem_wait(&MUTEX_MEMTABLE);
	dump();
	sem_post(&MUTEX_MEMTABLE);

	log_info(logger, "----------------------------------------");

	list_destroy(memtable);

	sem_destroy(&MUTEX_MEMTABLE);
	sem_destroy(&MUTEX_DUMPTIME);
	sem_destroy(&MUTEX_RETARDTIME);

	log_info(logger, "Fin FileSystem");
	log_info(logger, "----------------------------------------");
	log_destroy(logger);
}

void *threadConfigModify(){
	log_info(logger, "----------------------------------------");
	log_info(logger, "Inicia monitoreo de cambios en .config");
	log_info(logger, "----------------------------------------");

	while(1){
		read(fd,NULL,200);
		log_info(logger, "----------------------------------------");
		log_info(logger, "Se podrujo un cambio en el .config");
		log_info(logger, "Actualizando valores...");

		load_config();

		sem_wait(&MUTEX_DUMPTIME);
		dumpTime = get_dump_time();
		sem_post(&MUTEX_DUMPTIME);

		sem_wait(&MUTEX_RETARDTIME);
		retardTime = get_retard_time();
		sem_post(&MUTEX_RETARDTIME);

		config_destroy(config);

		log_info(logger, "Valores actualizados y disponibles para su uso");
		log_info(logger, "----------------------------------------");
	}
	return NULL;
}

void *threadDump(){
	int dt;
	while(1){
		sem_wait(&MUTEX_DUMPTIME);
		dt = dumpTime;
		sem_post(&MUTEX_DUMPTIME);
		sleep(dt/1000);
		log_info(logger, "----------------------------------------");
		log_info(logger, "Iniciando Dump");
		sem_wait(&MUTEX_MEMTABLE);
		dump();
		sem_post(&MUTEX_MEMTABLE);
		log_info(logger, "Fin Dump");
		log_info(logger, "----------------------------------------");
	}
	return NULL;
}
void iniciar_logger(t_log **logger){
	*logger = log_create("FileSystem.log", "FileSystem", 1, LOG_LEVEL_INFO);
}

void load_config(){
	config = config_create("/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Config/.config");
	if(config == NULL){
		log_error(logger,"No se pudo abrir el archivo de configuracion");
	}
}

int get_dump_time(){
	return config_get_int_value(config,"TIEMPO_DUMP");
}
int get_retard_time(){
	return config_get_int_value(config,"RETARDO");
}
char *get_fs_route(){
	return config_get_string_value(config,"PUNTO_MONTAJE");
}
