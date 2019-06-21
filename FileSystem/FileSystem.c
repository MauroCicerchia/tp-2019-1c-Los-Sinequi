#include "FileSystem.h"

/*GLOBALES*/
t_list *memtable;

char *absoluto;

t_log *logger;

int fd,wd,dumpTime,retardTime,tmpNo,port,valueSize;

pthread_t tApi,tDump,tListenCfg;

t_config *config;
t_config *metadataCfg;

int metadataBlocks;
int metadataSizeBlocks;


t_bitarray *bitarray;
int bitarrayfd;
char *bitarrayContent;

int lastBlockAssigned;
int flagBloquesLibres;

sem_t MUTEX_MEMTABLE, MUTEX_RETARDTIME, MUTEX_DUMPTIME, MUTEX_BITARRAY;
/*GLOBALES*/

int main(int argc, char **argv)
{

	init_FileSystem();

	pthread_create(&tListenCfg,NULL,threadConfigModify,NULL);
	pthread_detach(tListenCfg);

//	pthread_create(&tDump,NULL,threadDump,NULL);
//	pthread_detach(tDump);

	start_Api();

	kill_FileSystem();

	return 0;
}

void init_FileSystem()
{
	sem_init(&MUTEX_MEMTABLE,1,1); //inicio los semaforos
	sem_init(&MUTEX_DUMPTIME,1,1);
	sem_init(&MUTEX_RETARDTIME,1,1);
	sem_init(&MUTEX_BITARRAY,1,1);

	memtable = list_create(); //creo memtable


	logger = NULL;
	iniciar_logger(&logger); //arranco logger

	fd = inotify_init(); //arranco monitoreo en el archivo de config
	wd = inotify_add_watch(fd,"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Config",IN_MODIFY);

	load_config();
	dumpTime = get_dump_time();
	retardTime = get_retard_time();
	absoluto = string_duplicate(get_fs_route());
	port = get_port();
	valueSize = get_valueSize();
	config_destroy(config);

	t_config *metadata = load_lfsMetadata();
	metadataBlocks = get_blocks_cuantityy(metadata);
	metadataSizeBlocks = get_size_of_blocks(metadata);
	config_destroy(metadata); //leo metadata del fs

	tmpNo = 0;
	fs_setActualTmps(); //me fijo cuantos temporales hay al iniciar el sistema
	flagBloquesLibres = 1; //hay bloques libre
	lastBlockAssigned = 0; //inicio como ultimo bloque asignado el primero
	printf("%d",tmpNo);
//	if(!b_blocksCreated()){ //
//		b_create();
//	}





	ba_create(); //levanto el bitarray
}


void kill_FileSystem()
{
	log_info(logger, "----------------------------------------");
	log_info(logger, "Dump de seguridad");

	sem_wait(&MUTEX_MEMTABLE);
	dump(); //dump de cierre
	sem_post(&MUTEX_MEMTABLE);

	log_info(logger, "----------------------------------------");

	list_destroy(memtable); //mato memtable

	ba_bitarrayDestroy(); //mato el bitarray

	sem_destroy(&MUTEX_MEMTABLE);//mato semaforos
	sem_destroy(&MUTEX_DUMPTIME);
	sem_destroy(&MUTEX_RETARDTIME);
	sem_destroy(&MUTEX_BITARRAY);

	log_info(logger, "Fin FileSystem");
	log_info(logger, "----------------------------------------");

	log_destroy(logger); //mato logger
	//chau fs :)
}

void *threadConfigModify(){
	log_info(logger, "----------------------------------------");
	log_info(logger, "Inicia monitoreo de cambios en .config");
	log_info(logger, "----------------------------------------");
	char *buff = NULL;
	while(1){
		buff = malloc(200);
		read(fd,buff,200);
		free(buff);
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
int get_valueSize(){
	return config_get_int_value(config,"TAMAÑO_VALUE");
}
int get_port(){
	return config_get_int_value(config,"PUERTO_ESCUCHA");
}

int get_blocks_cuantityy(t_config *metadata)
{
	return config_get_int_value(metadata,"BLOCKS");
}
int get_size_of_blocks(t_config *metadata){
	return config_get_int_value(metadata,"BLOCK_SIZE");
}


t_config *load_lfsMetadata()
{
	char *url = fs_getlfsMetadataUrl();
	t_config *metadata = config_create(url);
		if(metadata == NULL){
			log_error(logger,"No se pudo abrir el archivo de Metadata del FS");
			return NULL;
		}
		return metadata;
}

