#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"commons/string.h"
#include"commons/txt.h"

int fs_tableExists(char*);
int fs_create(char*,char*,int,int);
char *makeUrl(char *tabla,char *);
void makeDirectoriesAndFiles(char *,int);
void makeMetadataFile(char *);
void loadMetadata(char *,char *,int ,int);
