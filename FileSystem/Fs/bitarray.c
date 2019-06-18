#include"bitarray.h"

void ba_loadBitarray(){
	int f = open(fs_getBitmapUrl(), O_RDWR);
	struct stat dunno;
	fstat(f,&dunno);
	bitarray = (t_bitarray*) mmap(NULL,dunno.st_size, PROT_WRITE, MAP_SHARED, f, 0);
}

//GUARDARLO EN MEMORIA
void ba_bitarrayDestroy(){
	msync((void*)bitarray,bitarray->size, MS_SYNC);

}


void ba_create()
{
	int blocks = get_blocks_cuantity();

	if(blocks == 0) log_error(logger,"No puede haber 0 bloques");//no puede haber 0 bloques

	if(blocks%8 == 0){
		char *nullBitarray = calloc(blocks/8, sizeof(char)*(blocks/8));
		bitarray = bitarray_create_with_mode(nullBitarray, sizeof(nullBitarray), LSB_FIRST);
	}else{
		char *nullBitarray = calloc(blocks/8+1, ((sizeof(char)/8)*blocks)+1);
		bitarray = bitarray_create_with_mode(nullBitarray, sizeof(nullBitarray), LSB_FIRST);
	}

	char *url = fs_getBitmapUrl();
	FILE *f = fopen(url,"w+");
	txt_write_in_file(f,(char*)bitarray);
	fclose(f);

//	bitarray_destroy(bitarray);

//	fs_createBlocks(blocks);
}


int ba_exists(){
	char *url = fs_getBitmapUrl();
	if(access(url,F_OK) != -1)return 1;
	return 0;
}

int get_blocks_cuantity()
{
	return config_get_int_value(lfsMetadata,"BLOCKS");

}

void *ba_load_lfsMetadata()
{
	char *url = fs_getlfsMetadataUrl();
	lfsMetadata = config_create(url);
		if(lfsMetadata == NULL){
			log_error(logger,"No se pudo abrir el archivo de Metadata del FS");
			return NULL;
		}
		return "no soy null :)";
}

// si no hay bloques libres devuelve -1
//devuelve un boque libre y lo marca como usado en el bitarray
int ba_getNewBlock()
{
	int blocks = get_blocks_cuantity();
	int aux = lastBlockAssigned;
	while(blocks){//maximo loop = cantidad de blocks del fs  //esto esta mal porque tiene que volver al ppio
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





