#ifndef SOCKET
#define SOCKET
#define SUCCEED 0
#define FAIL -1
#define BLOCK 4096
#define CCONNECT 0
#define CACCEPT  1
#define RBLOCK   0
#define RNBLOCK  1
#define FDEND   -2
#define BROKENNET -3
#include<netdb.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<stdio.h>

int initsocket(unsigned char connecttype,char* ip,char*port);//init listen fd
int readfd(char*str,int *length,unsigned char block);//read str from peer
int writefd(char*str,int *length);//write str to peer
void getpeerip(char **ip);

#endif
