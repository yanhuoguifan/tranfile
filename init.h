#ifndef INIT
#define INIT
#define PARSEMODULE ":s:r:i:p:"
#define SUCCEED 0
#define FAIL -1
#define BLOCK 4096
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/resource.h>
#include<signal.h>
#include<syslog.h>
#include"i6tcpsocket.h"
#include"http.h"
#include"tranfile.h"

int init(int argc,char**args);//init all

#endif
