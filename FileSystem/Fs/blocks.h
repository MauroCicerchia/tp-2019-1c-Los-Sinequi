#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>

#include<commons/config.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<sharedLib/console.h>
#include"bitarray.h"

extern t_config *lfsMetadata;

#ifndef BLOCKS_H
#define BLOCKS_H
void b_loadPartitionsFiles(char *tableUrl, int parts);   //le asigna el size y un bloque a cada bloque de particion
void b_assignSizeAndBlock(char *partUrl) ;  //le asigna un bloque y el size a esa url
t_list *b_getListOfInserts(char *partUrl); //trae todas los inserts de esa url, que es la particion.bin o el .tmp
#endif
