#include"init.h"
static int parse(int argc,char**argv)//parse commomd line parameter
{
	extern char*optarg;
      	extern int optind,opterr,optopt;
	char ip[BLOCK],port[BLOCK],filepath[PATH_MAX];
	int ret,s=0,r=0,p=0,i=0;
	while((ret= getopt(argc,argv,PARSEMODULE))!=-1)
	{
		switch(ret)
		{
			case('s'):
			++s;
			strcpy(filepath,optarg);
		        break;
			case('r'):
			++r;
			strcpy(filepath,optarg);
			break;
			case('i'):
			strcpy(ip,optarg);
			++i;
			break;
			case('p'):
			strcpy(port,optarg);
			++p;
			break;
			case(':'):fprintf(stderr,"the %c need parameter\n",optopt);
                        return FAIL;
			break;
			case('?'):fprintf(stderr,"you can't input %c\n",optopt);
                        return FAIL;
			break;
		}
	}
	if(((s==1&&r==0)||(s==0&&r==1))&&((p==1&&i==0)||(i==1&&p==1)))
	{
	  if(i==1)
          {
		  i=CCONNECT;
	  }
	  else
	  {
		  i=CACCEPT;
	  }
	  if(s==1)
	  {
		  s=HTTPRETURN;
	  }
	  else
	  {
		  s=HTTPREQUEST;
	  }
        return initsocket((unsigned char)i,ip,port)||httpinit()||fileinit(s,filepath);
	}
        fprintf(stderr,"you should input parase :one(-s or -r) and one(-p or -i)\n"); 
        return FAIL;
}

inline int init(int argc,char**args)//init all
{
     return parse(argc,args);
}
