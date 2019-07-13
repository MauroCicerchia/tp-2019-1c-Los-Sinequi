#include"bitarray.h"

//para usar en kill filesystem
void ba_bitarrayDestroy(){
	msync(bitarray->bitarray, bitarrayfd, MS_SYNC);
	bitarray_destroy(bitarray);
}


void ba_create()
{
	pthread_mutex_lock(&MUTEX_BITARRAY);

	int blocks = get_blocks_cuantity();

	if(blocks == 0){
		log_error(logger,"No puede haber 0 bloques");
		pthread_mutex_unlock(&MUTEX_BITARRAY);
		return;
	}

	char *url = fs_getBitmapUrl();
	bitarrayfd = open(url,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	ftruncate(bitarrayfd,blocks/8 + 1);
	if(bitarrayfd == -1){
		log_error(logger,"error al abrir el bitarray, abortando sistema");
		close(bitarrayfd);
		free(url);
		pthread_mutex_unlock(&MUTEX_BITARRAY);
		return;
	}

	bitarrayContent = mmap(NULL,blocks/8 + 1, PROT_READ | PROT_WRITE, MAP_SHARED, bitarrayfd,0);
	bitarray = bitarray_create_with_mode(bitarrayContent, blocks/8 + 1 , LSB_FIRST);
	free(url);

	for(int i=0; i < blocks; i++){

		if(b_empty(i)){
			bitarray_clean_bit(bitarray,i);
		}else{
			bitarray_set_bit(bitarray,i);
		}
	}
	msync(bitarray->bitarray, bitarrayfd, MS_SYNC);
	pthread_mutex_unlock(&MUTEX_BITARRAY);
}


int get_blocks_cuantity()
{
	return metadataBlocks;

}

// si no hay bloques libres devuelve -1
//devuelve un boque libre y lo marca como usado en el bitarray
int ba_getNewBlock()
{
	if(!flagBloquesLibres){ //si no hay bloques libres ni busca
		return -1;
	}

	pthread_mutex_lock(&MUTEX_BITARRAY);

	int blocks = get_blocks_cuantity();
	int aux = lastBlockAssigned;
	int i;

	for(i = lastBlockAssigned; i < blocks; i++){
		if(!bitarray_test_bit(bitarray,aux)){
			bitarray_set_bit(bitarray,aux);
			lastBlockAssigned = aux;
			b_writeBlockAssigned(aux);
			msync(bitarray->bitarray, bitarrayfd, MS_SYNC);
			pthread_mutex_unlock(&MUTEX_BITARRAY);
			return aux;
		}
		else aux++;//vas al proximo bloque
	}

	aux = 0;
//	i=0;
	while(aux < lastBlockAssigned){
		if(!bitarray_test_bit(bitarray,aux)){
			bitarray_set_bit(bitarray,aux);
			lastBlockAssigned = aux;
			msync(bitarray->bitarray, bitarrayfd, MS_SYNC);
			pthread_mutex_unlock(&MUTEX_BITARRAY);
			return aux;
		}
		else aux++;
//		i++;
	}
	flagBloquesLibres = 0; // 0 si no hay libres, 1 si los hay
	pthread_mutex_unlock(&MUTEX_BITARRAY);
	return -1; // salio del while, por lo que no hay bloque libres/
}

//libera el bloque "block"
void ba_freeBlock(int block)
{
	pthread_mutex_lock(&MUTEX_BITARRAY);

	bitarray_clean_bit(bitarray,block);
	flagBloquesLibres = 1;

	pthread_mutex_unlock(&MUTEX_BITARRAY);
}





