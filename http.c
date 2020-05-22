#include"http.h"
static httpparase*parase;
static filemeta*meta;
static filedata*data;

inline httpparase*getparase(unsigned char parasetype)
{
	if(parase==NULL||meta==NULL||data==NULL)
	{
		return NULL;
	}
	parase->type=parasetype;
	if(parasetype==FILEMETA)
	{
		parase->parase=(void*)meta;
	}
	else
	{
		parase->parase=(void*)data;
	}
	return parase;
}

inline int httpinit(void)
{
	parase=(httpparase*)malloc(sizeof(httpparase));
	meta=(filemeta*)malloc(sizeof(filemeta));
	data=(filedata*)malloc(sizeof(filedata));
	if(parase==NULL||meta==NULL||data==NULL)
	{
		fprintf(stderr,"http init fail\n");
		return FAIL;
	}
	parase->filename=NULL;
        return SUCCEED;
}
int parsinghttp(void)
{
	unsigned int i,length,alllength=0,t,f;
	int ret;
	char httphead[BLOCK];
	while(1)
	{
		length=100;
		if((ret=readfd(httphead+alllength,&length,RNBLOCK))!=SUCCEED)
		{
			return ret;
		}
	for(i=alllength,t=alllength+length;i<t;++i)
	{
		if(httphead[i]=='\r'&&httphead[i+1]=='\n'&&httphead[i+2]=='\r'&&httphead[i+3]=='\n')
		{
	         alllength+=length;
		 length=i-1;
		 goto headend;
		}
	}
	alllength+=length;
	}
headend:
	if(parase->type&HTTPRETURN)
	{
           if(memcmp(httphead,"GET ",strlen("GET "))!=0)
	   {
		   return FAIL;
	   }
           t=4;
	   for(i=t;i<=length;++i)
	   {
                if(httphead[i]=='?')
		{
			break;
		}
	   }
	   if(i==length)
	   {
		   return FAIL;
	   }
           if(!((i==1+t)&&memcmp(httphead+t,"/",1)==0)&&!(i==t+strlen(parase->filename)&&memcmp(httphead+t,parase->filename,i-t)==0))
	   {
	           return FAIL;
	   }
	   t=strlen(HTTPPARASE);
           if(memcmp(httphead+i,HTTPPARASE,t)!=0)
	   {
		   return FAIL;
	   }
	   i+=t;
	   if(httphead[i]==FILEDATA +'0')
	   {
		   char *field[14]={BLANK,HTTP,LINEFEED,HOST,COLON,NULL,LINEFEED,USER,COLON,EDITION,LINEFEED,RANGE,COLON,BYTE};
		   ++i;
		   for(t=0;t<5;++t)
		   {
			   f=strlen(field[t]);
			   if(memcmp(httphead+i,field[t],f)!=0)
			   {
				   return FAIL;
			   }
                           i+=f;
		   }
		   f=strlen(LINEFEED);
		   for(;i<=length;++i)
		   {
			   if(memcmp(httphead+i,LINEFEED,f)==0)
			   {
				   break;
			   }
		   }
		   if(i>length)
		   {
			   return FAIL;
		   }
		   i+=f;
		   for(t=7;t<14;++t)
		   {
			   f=strlen(field[t]);
			   if(memcmp(httphead+i,field[t],f)!=0)
			   {
				   return FAIL;
			   }
			   i+=f;
		   }
		   httphead[length+1]='\0';
		   if(sscanf(httphead+i,"%llu-%llu",&(data->start),&(data->relength))!=2)
		   {
			   return FAIL;
		   }
		   parase->type&=(~FILEMETA);
		   parase->type|=FILEDATA;
		   return SUCCEED;
	   }
	   else if(httphead[i++]==FILEMETA +'0')
	   {
                   unsigned int f;
		   char *field[10]={BLANK,HTTP,LINEFEED,HOST,COLON,NULL,LINEFEED,USER,COLON,EDITION};
		   for(t=0;t<5;++t)
		   {
			   f=strlen(field[t]);
			   if(memcmp(httphead+i,field[t],f)!=0)
			   {
				   return FAIL;
			   }
                           i+=f;
		   }
		   f=strlen(LINEFEED);
		   for(;i<=length;++i)
		   {
			   if(memcmp(httphead+i,LINEFEED,f)==0)
			   {
				   break;
			   }
		   }
		   if(i>length)
		   {
			   return FAIL;
		   }
		   for(t=6;t<10;++t)
		   {
			   f=strlen(field[t]);
			   if(memcmp(httphead+i,field[t],f)!=0)
			   {
				   return FAIL;
			   }
			   i+=f;
		   }
		   parase->type&=(~FILEDATA);
		   parase->type|=FILEMETA;
		   return SUCCEED;
	   }
	   else
           {
		   return FAIL;
	   }
	}
	else
	{
	      f=strlen(HTTP);
              if(memcmp(httphead,HTTP,f)!=0)
	      {
		      return FAIL;
	      }
	      i=f;
	      f=strlen(BLANK);
              if(memcmp(httphead+i,BLANK,f)!=0)
	      {
		      return FAIL;
	      }
	      i+=f;
              if(memcmp(httphead+i,"200",3)==0)
	      {
		  i+=3;
		  unsigned long long contentlength;
                  if(parase->type&FILEMETA)
		  {
                   char *field[15]={BLANK,"OK",LINEFEED,CONTENT_LENGTH,COLON};
		  for(t=0;t<5;++t)
		  {
			   f=strlen(field[t]);
			   if(memcmp(httphead+i,field[t],f)!=0)
			   {
				   return FAIL;
			   }
                           i+=f;
		  }
			  httphead[length+1]='\0';
                          if(sscanf(httphead+i,"%llu",&contentlength)!=1)
			  {
				  return FAIL;
			  }
			  length+=5;
			  if(alllength>length+contentlength)
			  {
				  return FAIL;
			  }
			  else
			  {
				  unsigned int readlength=contentlength-alllength+length;
				  if((ret=readfd(httphead+alllength,&readlength,RBLOCK))!=SUCCEED)
				  {
					  return ret;
				  }
				  char filename[PATH_MAX];
				  httphead[alllength+readlength]='\0';
				  if(sscanf(httphead+length,"%s"BLANK"%ld"BLANK"%u"BLANK"%ld:%ld",filename,&(meta->filelength),&(meta->mode),&(meta->time.tv_sec),&(meta->time.tv_nsec))!=5)
				  {
					  return FAIL;
				  }
                                  free(parase->filename);
				  f=strlen(filename);
				  parase->filename=(char*)malloc(sizeof(char)*(f+1));
				  if(parase->filename==NULL)
				  {
					  return FAIL;
				  }
				  strncpy(parase->filename,filename,f+1);
                                  parase->type&=(~FILEDATA);
			          parase->type|=FILEMETA;
				  return SUCCEED;
			  }
		  }
		  else
		  {
			    unsigned long long contentlength;
			    off_t filelength;
			    httphead[length+1]='\0';
                            if(sscanf(httphead+i,BLANK"OK"LINEFEED CONTENT_RANGE COLON BYTES"%llu-%llu/%ld"LINEFEED CONTENT_LENGTH COLON "%llu",&(data->start),&(data->relength),&filelength,&contentlength)!=4)
			    {
				    return FAIL;
			    }
			    if(filelength!=meta->filelength)
			    {
				    return FAIL;
			    }
			  length+=5;
			  if(alllength>length+contentlength)
			  {
				  return FAIL;
			  }
		          unsigned int readlength=contentlength-alllength+length;
			  if(alllength>length)
			  {
                          memcpy((char*)(data->filestr)+data->start,httphead+length,alllength-length+1);
			  }
                          parase->type&=(~FILEMETA);
			  parase->type|=FILEDATA;
			  return readfd((char*)(data->filestr)+data->start+alllength-length,&readlength,RBLOCK);
		  }
	      }
	      else
	      {
		      return FAIL;
	      }
	}
}
int sendhttp(int succeedorfail)
{
  char httphead[BLOCK],*head=httphead;
  unsigned int length,i;
  if(parase->type&HTTPREQUEST)
  {
       char *empyt="/",*field[22]={GET,BLANK,parase->filename,HTTPPARASE,NULL,BLANK,HTTP,LINEFEED,HOST,COLON,NULL,LINEFEED,USER,COLON,EDITION,LINEFEED,RANGE,COLON,BYTE,NULL,LINEFEED,LINEFEED};
       if(field[2]==NULL)
       {
	       field[2]=empyt;
       }
       getpeerip(&(field[10]));
       if(parase->type&FILEMETA)
       {
               field[4]="1";
	       for(i=0;i<16;++i)
	       {
                   length=strlen(field[i]);
                   strncpy(head,field[i],length);
                   head+=length;
	       }
               length=strlen(LINEFEED);
               strncpy(head,LINEFEED,length);
               head+=length;
               length=head-httphead;
               return writefd(httphead,&length);
       }
       else
       {
               char contentlength[40];
	       field[4]="2";
	       field[19]=contentlength;
	       sprintf(field[19],"%llu-%llu",data->start,data->relength);
	       for(i=0;i<22;++i)
	       {
                   length=strlen(field[i]);
                   strncpy(head,field[i],length);
                   head+=length;
	       }
               length=head-httphead;
               return writefd(httphead,&length);
       }
  }
  else
  {
     if(succeedorfail==FAIL)
     {
       char*field[7]={HTTP,BLANK,"400",BLANK,"Bad Request",LINEFEED,LINEFEED};
	       for(i=0;i<7;++i)
	       {
                   length=strlen(field[i]);
                   strncpy(head,field[i],length);
                   head+=length;
	       }
       length=head-httphead;
       return writefd(httphead,&length);
     }
     else
     {
	     if(parase->type&FILEMETA)
	     {
       unsigned long long contentlength=0;
       char filelength[20],filemode[10],filectime[40],content_length[20],*field[18]={HTTP,BLANK,"200",BLANK,"OK",LINEFEED,CONTENT_LENGTH,COLON,content_length,LINEFEED,LINEFEED,parase->filename,BLANK,filelength,BLANK,filemode,BLANK,filectime};
       sprintf(filelength,"%ld",meta->filelength);
       sprintf(filemode,"%u",meta->mode);
       sprintf(filectime,"%ld",meta->time.tv_sec);
       sprintf(filectime+strlen(filectime),":%ld",meta->time.tv_nsec);
       for(i=11;i<18;++i)
       {
	       contentlength+=strlen(field[i]);
       }
       sprintf(content_length,"%llu",contentlength);
       for(i=0;i<18;++i)
       {
               length=strlen(field[i]);
               strncpy(head,field[i],length);
               head+=length;
       }
       length=head-httphead;
       return writefd(httphead,&length);
	     }
	     else
	     {
       char filelength[40],content_length[20],*field[16]={HTTP,BLANK,"200",BLANK,"OK",LINEFEED,CONTENT_RANGE,COLON,BYTES,filelength,LINEFEED,CONTENT_LENGTH,COLON,content_length,LINEFEED,LINEFEED};
       sprintf(content_length,"%llu",data->relength-data->start+1);
       sprintf(filelength,"%llu-%llu/%ld",data->start,data->relength,meta->filelength);
       for(i=0;i<16;++i)
       {
               length=strlen(field[i]);
               strncpy(head,field[i],length);
               head+=length;
       }
       length=head-httphead;
       if(writefd(httphead,&length)==SUCCEED)
       {
	       length=data->relength-data->start+1;
	       return writefd((char*)(data->filestr)+data->start,&length);
       }
       else
       {
	       return FAIL;
       }
	     }
     }
  }
}
