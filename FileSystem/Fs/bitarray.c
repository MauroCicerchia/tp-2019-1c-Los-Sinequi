#include"bitarray.h"

//para usar en kill filesystem
void ba_bitarrayDestroy(){
	msync(bitarray->bitarray, bitarrayfd, MS_SYNC);
	bitarray_destroy(bitarray);
}


void ba_create()
{
	int blocks = get_blocks_cuantity();

	if(blocks == 0) log_error(logger,"No puede haber 0 bloques");//no puede haber 0 bloques
	char *url = fs_getBitmapUrl();
	bitarrayfd = open(url,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	ftruncate(bitarrayfd,blocks/8 + 1);
	if(bitarrayfd == -1){
		log_error(logger,"error al abrir el bitarray, abortando sistema");
		close(bitarrayfd);
		free(url);
		exit(-1);
	}
	else{
		bitarrayContent = mmap(NULL,blocks/8 + 1, PROT_READ | PROT_WRITE, MAP_SHARED, bitarrayfd,0);
		bitarray = bitarray_create_with_mode(bitarrayContent, blocks/8 + 1 , LSB_FIRST);
		free(url);
	}
}


//int ba_exists(){
//	char *url = fs_getBitmapUrl();
//	if(access(url,F_OK) != -1){
//		free(url);
//		return 1;
//	}
//	free(url);
//	return 0;
//}

int get_blocks_cuantity()
{
	return metadataBlocks;

}

// si no hay bloques libres devuelve -1
//devuelve un boque libre y lo marca como usado en el bitarray
int ba_getNewBlock()
{
	int blocks = get_blocks_cuantity();
	int aux = lastBlockAssigned;
	while(blocks){//esto esta mal porque tiene que volver al ppio
		if(!bitarray_test_bit(bitarray,aux)){
			bitarray_set_bit(bitarray,aux);
			lastBlockAssigned = aux;
			return aux;
		}else{
			aux++;//vas al proximo bloque
			blocks--; //descartas una posibilidad
		}
	}
	return -1; // salio del while, por lo que no hay bloque libres/
}

//libera el bloque "block"
void ba_freeBlock(int block)
{
	bitarray_clean_bit(bitarray, block);
}





