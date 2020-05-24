#include"tranfile.h"
typedef struct information
{
	char filepath[PATH_MAX],cpfilepath[PATH_MAX];
	mode_t mode;
	off_t filestart;
	struct timespec time;
}information;
static off_t conflengthstart;
static off_t conflengthlast;
static httpparase*parase;
static filedata*data;
static filemeta*meta;
static information alldata;
static void*confbuf;
static int confd;
static char *confpath;
int getfilepath(int fd)
{
	char temp[100];
	int ret;
	snprintf(temp,sizeof(temp),"/proc/self/fd/%d",fd);
	if((ret=readlink(temp,alldata.filepath,PATH_MAX))==FAIL)
	{
		fprintf(stderr,"getfilepath fail,errno:%d\n",errno);
		return FAIL;
	}
        alldata.filepath[ret]='\0';
	return SUCCEED;
}
int fileinit(unsigned char identity,char*filepath)
{
	int fd,ret;
        void*filebuf;
	parase=getparase(FILEDATA);
	data=(filedata*)parase->parase;
	parase=getparase(FILEMETA);
	meta=(filemeta*)parase->parase;
	if(identity==HTTPRETURN)
	{
	      parase->filename=alldata.filepath;
	      parase->type|=HTTPRETURN;
	      parase->type&=(~HTTPREQUEST);
	      if((fd=open(filepath,O_RDONLY))==FAIL)
	      {
		      fprintf(stderr,"open %s path fail,errno:%d\n",filepath,errno);
		      return FAIL;
	      }
	      if(getfilepath(fd)==FAIL)
	      {
		      return FAIL;
	      }
	      struct stat buf;
              if(stat(filepath,&buf)==FAIL)
	      {
		      fprintf(stderr,"stat %s path fail,errno:%d\n",filepath,errno);
	      }
	      meta->time=buf.st_ctim;
	      meta->mode=buf.st_mode;
	      meta->filelength=buf.st_size;
	      if((filebuf=mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED)
	      {
		      fprintf(stderr,"mmap %s path fail,errno:%d\n",alldata.filepath,errno);
		      return FAIL;
	      }
	}
	else
	{
	      char tempconfpath[PATH_MAX],*confname,hasfile;
	      information*tempdate;
	      parase->type|=FILEMETA|HTTPREQUEST;
	      parase->type&=(~FILEDATA)&(~HTTPRETURN);
	      if((ret=sendhttp(SUCCEED))!=SUCCEED||(ret=parsinghttp())!=SUCCEED)
	      {
		      return ret;
	      }
	      strcpy(alldata.cpfilepath,parase->filename);
	      alldata.mode=meta->mode;
	      alldata.time=meta->time;
	      hasfile=access(filepath,F_OK);
	      if((fd=open(filepath,O_RDWR|O_CREAT,alldata.mode))==FAIL)
	      {
		      if(hasfile==FAIL)
		      {
		           fprintf(stderr,"create %s file fail,errno:%d\n",filepath,errno);
		      }
		      else
		      {
		           fprintf(stderr,"open %s file fail,errno:%d\n",filepath,errno);
		      }
		      return FAIL;
	      }
	      if(getfilepath(fd)==FAIL)
	      {
		      return FAIL;
	      }
	      if((confpath=getenv(HOME))==NULL)
	      {
		      fprintf(stderr,"getenv HOME fail,errno:%d\n",errno);
		      return FAIL;
	      }		      
	      sprintf(tempconfpath,"%s%s",confpath,"/./tranfile");
              if(access(tempconfpath,F_OK)==FAIL)
	      {
		      if(mkdir(tempconfpath,MODE)==FAIL)
		      {
			      fprintf(stderr,"mkdir %s path fail,errno:%d\n",tempconfpath,errno);
			      return FAIL;
		      }
	      }
	      if((confname=strrchr(alldata.filepath,'/'))==NULL)
	      {
		      fprintf(stderr,"look for filename from %s path errno\n",alldata.filepath);
		      return FAIL;
	      }
	      if((confpath=(char*)malloc(sizeof(char)*(strlen(tempconfpath)+1+strlen(confname))))==NULL)
	      {
		      fprintf(stderr,"malloc fail,errno:%d\n",errno);
		      return FAIL;
	      }
	      sprintf(confpath,"%s%s",tempconfpath,confname);
	      if((confd=open(confpath,O_CREAT|O_RDWR,MODE))==FAIL)
	      {
		      fprintf(stderr,"open/create %s file fail,errno:%d\n",confpath,errno);
		      return FAIL;
	      }
	      conflengthstart=0;
	      if((conflengthlast=lseek(confd,0,SEEK_END))==FAIL)
	      {
		      fprintf(stderr,"lseek %s file to end fail,errno:%d\n",confpath,errno);
                      return FAIL;
	      }
	      if(conflengthlast>=sizeof(alldata))
	      {
                   if((confbuf=mmap(NULL,conflengthlast,PROT_READ|PROT_WRITE,MAP_SHARED,confd,0))==MAP_FAILED)
		   {
                         fprintf(stderr,"mmap %s from 0 to %d fail,errno:%d\n",confpath,conflengthlast-1,errno);
			 return FAIL;
		   }
	      }
	      while(conflengthstart<conflengthlast)
	      {
		      tempdate=(information*)((char*)confbuf+conflengthstart);
                      if(strcmp(tempdate->filepath,alldata.filepath)==SUCCEED)
		      {
                            if(hasfile==FAIL)
			    {
                                   alldata.filestart=0;
				   memcpy(tempdate,&alldata,sizeof(alldata));
				   break;
			    }
			   if(strcmp(tempdate->cpfilepath,alldata.cpfilepath)!=SUCCEED) 
			   {
				   fprintf(stderr,"%s file already exist\n",alldata.filepath);
				   return FAIL;
			   }
			   if(alldata.time.tv_sec!=tempdate->time.tv_sec||alldata.time.tv_nsec!=tempdate->time.tv_nsec||alldata.mode!=tempdate->mode)
			   {
                                   alldata.filestart=0;
				   memcpy(tempdate,&alldata,sizeof(alldata));
				   if(ftruncate(fd,0)!=SUCCEED||lseek(fd,0,SEEK_SET)!=0)
				   {
					   fprintf(stderr,"empty %s file fail,errno:%d\n",alldata.filepath,errno);
					   return FAIL;
				   }
				   break;
			   }
		      }
		      conflengthstart+=sizeof(alldata);
	      }
	      if(conflengthstart>=conflengthlast)
	      {
		      if(hasfile==SUCCEED)
		      {
			  fprintf(stderr,"%s file already exist\n",alldata.filepath);
			  return FAIL;
		      }
		      if(confbuf!=NULL&&munmap(confbuf,conflengthlast)==FAIL)
		      {
			  fprintf(stderr,"mummap %s file from 0 to %d fail,errno:%d\n",confpath,conflengthlast-1,errno);
		      }
		      conflengthlast+=sizeof(alldata);
		      if(ftruncate(confd,conflengthlast)==FAIL)
		      {
                          fprintf(stderr,"ftruncate %s file fail,errno:%d\n",confpath,errno);
			  return FAIL;
		      }
                      if((confbuf=mmap(NULL,conflengthlast,PROT_READ|PROT_WRITE,MAP_SHARED,confd,0))==MAP_FAILED)
		      {
                         fprintf(stderr,"mmap %s from 0 to %d fail,errno:%d\n",confpath,conflengthlast-1,errno);
			 return FAIL;
		      }
		      alldata.filestart=0;
		      memcpy(((char*)confbuf+conflengthstart),&alldata,sizeof(alldata));
	      }
	      if(ftruncate(fd,meta->filelength)==FAIL)
	      {
		      fprintf(stderr,"ftruncate %s file fail,errno:%d\n",alldata.filepath,errno);
		      return FAIL;
              }
	      if((filebuf=mmap(NULL,meta->filelength,PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED)
	      {
		      fprintf(stderr,"mmap %s from 0 to %d fail,errno:%d\n",alldata.filepath,meta->filelength,errno);
	              return FAIL;
	      }
	}
	data->filestr=filebuf;
	return SUCCEED;
}
static int sendfile(void)
{
        int ret;
	while(1)
	{
		if((ret=parsinghttp())!=SUCCEED)
		{
			if(ret==FDEND)
			{
				return SUCCEED;
			}
			else if(ret==BROKENNET)
			{
                                return BROKENNET;
			}
		}
		if((ret=sendhttp(ret))!=SUCCEED)
		{
			if(ret==FDEND)
			{
				return SUCCEED;
			}
			else
			{
				return ret;
			}
		}
	}
}
static int recefile(void)
{
       int ret;
       information*confdata=(information*)((char*)confbuf+conflengthstart);
       data->start=confdata->filestart;
       parase->type|=FILEDATA;
       parase->type&=(~FILEMETA);
       while(1)
       {
	     if(data->start+BLOCK<meta->filelength)
	     {
		     data->relength=data->start-1+BLOCK;
	     }
	     else
	     {
		     data->relength=meta->filelength-1;
	     }
	     if((ret=sendhttp(SUCCEED))!=SUCCEED||(ret=parsinghttp())!=SUCCEED)
	     {
		     return ret;
	     }
             data->start=data->relength+1;
	     if(data->start==meta->filelength)
	     {
		     if(conflengthstart+sizeof(alldata)<conflengthlast)
		     {
                            memcpy(confdata,(char*)confbuf+conflengthlast-sizeof(alldata),sizeof(alldata));
		     }
		     conflengthlast-=sizeof(alldata);
                     if(ftruncate(confd,conflengthlast)==FAIL||lseek(confd,conflengthlast,SEEK_SET)==FAIL)
		     {
			     fprintf(stderr,"narrow %s file fail,errno:%d\n",confpath,errno);
			     return FAIL;
		     }
                     return SUCCEED;
	     }
	     confdata->filestart=data->start;
       }
}
inline int tranfile(void)
{
	if(parase->type&HTTPRETURN)
	{
		return sendfile();
	}
	else
	{
		return recefile();
	}
}
