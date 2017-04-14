#include "cache.h"

void Add_Break(int pc)
{
	if(pc<0x00400000)
	{fprintf(stderr,"Error - Value of PC given is less than 0x00400000\n");fflush(stderr);}
	else
	{
		isbreak[(pc-0x00400000)/4]=1;
	}

}

void Delete_Break(int pc)
{
	if(pc<0x00400000)
	{fprintf(stderr,"Error - Value of PC given is less than 0x00400000\n");fflush(stderr);}
	else
	{
		isbreak[(pc-0x00400000)/4]=0;
	}
}

int find(int currr)
{
	if(isbreak[currr]==1)
	{return 1;} 
	else
	{return 0;}
}