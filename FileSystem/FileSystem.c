#include "FileSystem.h"

/*GLOBALES*/
t_list *memtable;

t_list *sysTables;

char *absoluto, *port, *ip;

t_log *logger;

int fd,wd,tmpNo,valueSize, exitFlag,dumpTime,retardTime;

pthread_t tApi,tDump,tListenCfg, tLisentClient;

t_config *config;
t_config *metadataCfg;

int metadataBlocks;
int metadataSizeBlocks;


t_bitarray *bitarray;
int bitarrayfd;
char *bitarrayContent;

int lastBlockAssigned;
int flagBloquesLibres;


pthread_mutex_t MUTEX_MEMTABLE, MUTEX_RETARDTIME, MUTEX_DUMPTIME, MUTEX_BITARRAY, MUTEX_CONFIG,MUTEX_ELSOLUCIONADOR,MUTEX_LISTACTIVETABLES;
sem_t MAX_CLIENTS;
/*GLOBALES*/

int main(int argc, char **argv)
{

	init_FileSystem();

	pthread_create(&tListenCfg,NULL,threadConfigModify,NULL);
	pthread_detach(tListenCfg);

	pthread_create(&tLisentClient,NULL,threadListenToClient,NULL);
	pthread_detach(tLisentClient);

	pthread_create(&tDump,NULL,threadDump,NULL);
	pthread_detach(tDump);

	start_Api();

	kill_FileSystem();

	return 0;
}

void init_FileSystem()
{
	system ("clear");

	pthread_mutex_init(&MUTEX_LISTACTIVETABLES,NULL);
	pthread_mutex_init(&MUTEX_ELSOLUCIONADOR,NULL);
	pthread_mutex_init(&MUTEX_CONFIG,NULL);
	pthread_mutex_init(&MUTEX_MEMTABLE,NULL); //inicio los semaforos
	pthread_mutex_init(&MUTEX_DUMPTIME,NULL);
	pthread_mutex_init(&MUTEX_RETARDTIME,NULL);
	pthread_mutex_init(&MUTEX_BITARRAY,NULL);
//	pthread_mutex_init(&MUTEX_STR_ARRAY,NULL);
	sem_init(&MAX_CLIENTS,1,1); //Max connections

	logger = NULL;
	iniciar_logger(&logger); //arranco logger

	log_info(logger,"[Lissandra]: Iniciando Lissandra FS");
	log_info(logger,"[Lissandra]: Creando estructuras...");

	memtable = list_create(); //creo memtable
	sysTables = list_create();
	log_info(logger,"[Lissandra]: Leyendo variables...");
	fd = inotify_init(); //arranco monitoreo en el archivo de config
	wd = inotify_add_watch(fd,"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Config/",IN_MODIFY);

	pthread_mutex_lock(&MUTEX_CONFIG);
		load_config();
		absoluto = string_duplicate(get_fs_route());
		ip = string_duplicate(get_ip());
		port = string_duplicate(get_port());
		valueSize = get_valueSize();
		dumpTime = get_dump_time();
		retardTime = get_retard_time();
		config_destroy(config);
	pthread_mutex_unlock(&MUTEX_CONFIG);

	t_config *metadata = load_lfsMetadata();
	metadataBlocks = get_blocks_cuantityy(metadata);
	metadataSizeBlocks = get_size_of_blocks(metadata);
	config_destroy(metadata); //leo metadata del fs
	log_info(logger,"[Lissandra]: Seteando hilos...");
	tmpNo = -1;
	fs_setActualTmps(); //me fijo cuantos temporales hay al iniciar el sistema
	tmpNo++; // =0 no hay tmps =6 de 0-5 tmps

	flagBloquesLibres = 1; //hay bloques libres
	lastBlockAssigned = 0; //inicio como ultimo bloque asignado el primero

	if(!b_blocksCreated()){
		b_create();
	}

	ba_create(); //levanto el bitarray

	fs_setActiveTables(); //cargo a memoria todas las tablas activas en "systables"

	activeTable *x;
	for(int i = 0; i < list_size(sysTables);i++){ //creo todos los hilos de compactacion para las tablas
		x = list_get(sysTables,i);
		threadForCompact(string_duplicate(x->name));
	}

	log_info(logger,"[Lissandra]: Lissandra FS Listo!");
//	compact(list_get(sysTables,0));
}


void kill_FileSystem()
{
	log_info(logger,"[Lissandra]: Terminando FS");
	log_info(logger, "[Lissandra]: Iniciando DUMP de seguridad...");

	pthread_mutex_lock(&MUTEX_MEMTABLE);
	dump(); //dump de cierre
	pthread_mutex_unlock(&MUTEX_MEMTABLE);
	log_info(logger, "[Lissandra]: Dump dump terminado!");

	log_info(logger, "[Lissandra]: Destruyendo MEMTABLE...");
	list_destroy(memtable); //mato memtable
	log_info(logger, "[Lissandra]: Destruida con exito!");

	log_info(logger, "[Lissandra]: Liberando lista de tablas activas...");
	void free_activeTable(void *pivot){
		free(((activeTable*)pivot)->name);
		pthread_mutex_destroy(&((activeTable*)pivot)->MUTEX_TABLE_PART);
		pthread_mutex_destroy(&((activeTable*)pivot)->MUTEX_DROP_TABLE);
		free(pivot);
	}

	list_destroy_and_destroy_elements(sysTables, free_activeTable);
	log_info(logger, "[Lissandra]: Liberada!");


	log_info(logger, "[Lissandra]: Destruyendo Bitarray...");
	ba_bitarrayDestroy(); //mato el bitarray
	log_info(logger, "[Lissandra]: Destruido!");

	pthread_mutex_destroy(&MUTEX_LISTACTIVETABLES);
	pthread_mutex_destroy(&MUTEX_ELSOLUCIONADOR);
	pthread_mutex_destroy(&MUTEX_CONFIG);
	pthread_mutex_destroy(&MUTEX_MEMTABLE);//mato semaforos
	pthread_mutex_destroy(&MUTEX_DUMPTIME);
	pthread_mutex_destroy(&MUTEX_RETARDTIME);
	pthread_mutex_destroy(&MUTEX_BITARRAY);
	sem_destroy(&MAX_CLIENTS);

	log_info(logger, "[Lissandra]: Termina FS.");

	log_destroy(logger); //mato logger

	free(absoluto);
	//chau fs :)
}

void *threadConfigModify()
{
	log_info(logger, "[Monitor]: Inicia monitoreo de cambios en .config");
	char *buff = NULL;
	while(1){
		buff = malloc(200);
		read(fd,buff,200);
		free(buff);
		log_info(logger, "[Monitor]: Se podrujo un cambio en .config");
		log_info(logger, "[Monitor]:Actualizando valores...");

		load_config();

		pthread_mutex_lock(&MUTEX_DUMPTIME);
		dumpTime = get_dump_time();
		pthread_mutex_unlock(&MUTEX_DUMPTIME);

		pthread_mutex_lock(&MUTEX_RETARDTIME);
		retardTime = get_retard_time();
		pthread_mutex_unlock(&MUTEX_RETARDTIME);

		config_destroy(config);

		log_info(logger, "[Monitor]: Valores actualizados");
	}
	return NULL;
}

void *threadDump()
{
	int dt;
	while(1){
		pthread_mutex_lock(&MUTEX_DUMPTIME);
//		dt = get_dump_time();
		dt=dumpTime;
		pthread_mutex_unlock(&MUTEX_DUMPTIME);

		usleep(dt * 1000);

		log_info(logger, "[DUMP]: Iniciando Dump");
		pthread_mutex_lock(&MUTEX_MEMTABLE);
		dump();
		pthread_mutex_unlock(&MUTEX_MEMTABLE);
		log_info(logger, "[DUMP]: Fin Dump");
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
//	pthread_mutex_lock(&MUTEX_CONFIG);
//	load_config();
	int dt = config_get_int_value(config,"TIEMPO_DUMP");
//	config_destroy(config);
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	log_error(logger,"**%d**",dt);
	return dt;
}
int get_retard_time(){
//	pthread_mutex_lock(&MUTEX_CONFIG);
//	load_config();
	int r = config_get_int_value(config,"RETARDO");
//	config_destroy(config);
//	pthread_mutex_unlock(&MUTEX_CONFIG);
	return r;
}
char *get_fs_route(){
	return config_get_string_value(config,"PUNTO_MONTAJE");
}
int get_valueSize(){
	return config_get_int_value(config,"TAMAÑO_VALUE");
}
char *get_port(){
	return config_get_string_value(config,"PUERTO_ESCUCHA");
}
char *get_ip(){
	return config_get_string_value(config,"IP");
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
	free(url);
		if(metadata == NULL){
			log_error(logger,"No se pudo abrir el archivo de Metadata del FS");
			return NULL;
		}
		return metadata;
}

void threadForCompact(char *tableName){
	pthread_t tCompact;
	pthread_create(&tCompact,NULL,(void*)threadCompact,tableName);
	pthread_detach(tCompact);
}

void *threadListenToClient(){
	listen_client();
	return NULL;
}


