#include"i6tcpsocket.h"
static int fd=-1;
static int peerfd=-1;
static char peerip[INET6_ADDRSTRLEN];

int initsocket(unsigned char connecttype,char* ip,char*port)
{
  struct addrinfo hint,*result,*savef;
  memset(&hint,0,sizeof(hint));
  hint.ai_family=AF_INET6;
  hint.ai_socktype=SOCK_STREAM;
  if(connecttype==CACCEPT)
  {
  ip=NULL;
  hint.ai_flags=AI_PASSIVE|AI_V4MAPPED;
  }
  else
  {
  hint.ai_flags=AI_V4MAPPED;
  }
  if(getaddrinfo(ip,port,&hint,&result))
  {
	 if(ip)
	 {
	  fprintf(stderr,"ip:%s,port:%s getaddrinfo fail\n",ip,port);
	 }
	 else
	 {
	  fprintf(stderr,"port:%s getaddrinfo fail\n",port);
	 }
          return FAIL;
  }
  savef=result;
  if(connecttype==CACCEPT)
  {
  if((fd=socket(AF_INET6,SOCK_STREAM,0))==-1)
  {
         fprintf(stderr,"socket fail\n");
         return FAIL;
  }
  const int on=1; 
  if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)
  {
         fprintf(stderr,"set socket fail\n");
         return FAIL;
  }
  do
  {
     if(!bind(fd,result->ai_addr,result->ai_addrlen))
     {
          break;
     }
  }while((result=result->ai_next));
  if(result==NULL)
  {
	  return FAIL;
  }
  if(listen(fd,1)<0)
  {
         fprintf(stderr,"listen fail\n");
         return FAIL;
  }
  if((peerfd=accept(fd,NULL,NULL))==-1)
  {
         fprintf(stderr,"accept fail\n");
         return FAIL;
  }
  }
  else
  {
      unsigned int i=1;
      while(i<=32)
     {
      if((fd=socket(AF_INET6,SOCK_STREAM,0))==-1)
      {
	 if(i==32)
	 {
                 fprintf(stderr,"socket fail\n");
		 return FAIL;
	 }
	 sleep(i);
	 i<<=1;
         continue;
      }
      if(connect(fd,result->ai_addr,result->ai_addrlen)==-1)
      {
	 if(i==32)
	 {
                 fprintf(stderr,"connect fail\n");
		 return FAIL;
	 }
	 sleep(i);
	 i<<=1;
	 continue;
      }
      else
      {
         peerfd=fd;
	 break;
      }
     }
  }
  freeaddrinfo(savef);
  if(ip!=NULL)
  {
  strncpy(peerip,ip,strlen(ip)+1);
  }
  return SUCCEED;
}

int readfd(char*str,int *length,unsigned char block)
{
	unsigned int nread,outtime=1,wread=*length;
	while(wread>0)
	{
		if((nread=read(peerfd,str,wread))<=0)
		{
                     if(nread<0)
		     {
		      if(errno==EINTR)
		      {
			     continue;
		      }
		      else if(errno==ECONNABORTED)
		      {
			     if(outtime==1)
			     {
			      fprintf(stderr,"the net broken\n");
			     }
			     else if(outtime>=128)
			     {
				     *length-=wread;
				     return BROKENNET;
			     }
			     sleep(outtime);
			     outtime*=2;
		      }
                      else
		      {
			      fprintf(stderr,"read return fail and errno is %d\n",errno);
			      *length-=wread;
			      return FAIL;
		      }
		     }
		     else
		     {
			     *length-=wread;
			     return FDEND;
		     }
		}
		else
		{
			wread-=nread;
			if(block==RNBLOCK)
			{
				*length-=wread;
				return SUCCEED;
			}
			str+=nread;
		}
	}
	return SUCCEED;
}
int writefd(char*str,int *length)
{
	unsigned int nwrite,outtime=1,wwrite=*length;
	while(wwrite>0)
	{
		if((nwrite=write(peerfd,str,wwrite))<=0)
		{
                     if(nwrite<0)
		     {
		      if(errno==EINTR)
		      {
			     continue;
		      }
		      else if(errno==ECONNABORTED)
		      {
			     if(outtime==1)
			     {
			      fprintf(stderr,"the net broken\n");
			     }
			     else if(outtime>=128)
			     {
			             *length-=wwrite;
				     return BROKENNET;
			     }
			     sleep(outtime);
			     outtime*=2;
		      }
                      else
		      {
			      fprintf(stderr,"read return fail and errno is %d\n",errno);
			      *length-=wwrite;
			      return FAIL;
		      }
		     }
		     else
		     {
			     *length-=wwrite;
			     return FDEND;
		     }
		}
		else
		{
			wwrite-=nwrite;
			str+=nwrite;
		}
	}
	return SUCCEED;
}
inline void getpeerip(char **ip)
{
       *ip=peerip;
}
