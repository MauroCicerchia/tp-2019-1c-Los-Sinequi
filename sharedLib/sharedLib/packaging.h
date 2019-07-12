#include"query.h"
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/socket.h>

#ifndef PACKAGING_H
#define PACKAGING_H

typedef struct{
	int size;
	void *stream;
}t_buffer;

typedef struct{
	e_query queryType;
	t_buffer *buffer;

}t_package;

typedef enum {
	REQUEST_QUERY,
	REQUEST_JOURNAL,
	REQUEST_GOSSIP,
	REQUEST_VALUESIZE
}e_request_code;

typedef enum {
	RESPONSE_SUCCESS,
	RESPONSE_ERROR,
	RESPONSE_FULL
}e_response_code;


void *serialize_package(t_package*, int);
t_package *create_package(e_query);
void create_buffer(t_package*);
void add_to_package(t_package*, void*, int);
void send_package(t_package*, int);
void delete_package(t_package*);
void set_buffer(t_package*, char**);
void send_int(int, int);
void send_timestamp(int socket, uint64_t value);
void send_str(int, char*);
void send_q_type(int, e_query);
void send_cons_type(int, e_cons_type);
void send_req_code(int, e_request_code);
void send_res_code(int, e_response_code);
int recv_int(int);
uint64_t recv_timestamp(int socket);
char *recv_str(int);
e_query recv_q_type(int);
e_cons_type recv_cons_type(int);
e_request_code recv_req_code(int);
e_response_code recv_res_code(int);

#endif
