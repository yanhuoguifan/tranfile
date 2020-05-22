#ifndef HTTP
#define HTTP "HTTP/1.1"
#define FILEMETA    (unsigned char)1
#define FILEDATA    (unsigned char)2
#define HTTPREQUEST (unsigned char)4
#define HTTPRETURN  (unsigned char)8
#define GET "GET"
#define BLANK " "
#define HOST "HOST"
#define COLON ": "
#define USER "USER-AGENT"
#define LINEFEED "\r\n"
#define BYTE     "bytes="
#define RANGE    "RANGE"
#define CONTENT_RANGE "CONTENT-RANGE"
#define CONTENT_LENGTH "CONTENT-LENGTH"
#define BYTES  "bytes "
#define EDITION "tranfile.1.0"
#define HTTPPARASE "?messagetype="
#include"i6tcpsocket.h"
#include<limits.h>
typedef struct filemeta
{
	struct timespec time; 
	mode_t mode;
	off_t filelength;
}filemeta;
typedef struct filedata
{
	void*filestr;
	unsigned long long start,relength;
}filedata;

typedef struct httpparase
{
	void*parase;
	char*filename;
	unsigned char type;
}httpparase;

httpparase*getparase(unsigned char parasetype);
int httpinit(void);
int parsinghttp(void);
int sendhttp(int succeedoffail);
#endif
