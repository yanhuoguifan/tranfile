#ifndef TRANFILE
#define TRANFILE
#define HOME "HOME"
#define MODE S_IRWXU|S_IXGRP|S_IRGRP|S_IXOTH
#include"http.h"
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/mman.h>

int fileinit(unsigned char identity,char*filepath);
int tranfile(void);
#endif
