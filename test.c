#include"init.h"
int main(int argc,char**args)
{
	if(init(argc,args)==SUCCEED)
	{
		httpparase* parase=getparase(FILEMETA);
		parase->type|=HTTPREQUEST;
		sendhttp(SUCCEED);
	}
	return 0;
}
