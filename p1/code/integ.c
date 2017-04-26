#include "main.h"
#include "cache.h"


void write_to_cache(int u,int idx)
{
	c[u][idx]=write_data;
}

void read_from_cache(int u,int idx)
{
	read_data=c[u][idx];
}

void write_to_MM(unsigned addr)
{
	if(0x10010000<=addr && addr<0x10010000+67108864)
	{MMd[addr-0x10010000]=write_data;}
	else if(0x00400000<=addr && addr<0x00400000+65536)
	{MMi[addr-0x00400000]=write_data;}
	else
	{//printf("Error - memory address is out of range\n");
	}

}

void read_from_MM(unsigned addr)
{
	if(0x10010000<=addr && addr<0x10010000+67108864)
	{read_data=MMd[addr-0x10010000];}
	else if(0x00400000<=addr && addr<0x00400000+65536)
	{read_data=MMi[addr-0x00400000];}
	else
	{//printf("Error - memory address is out of range\n");
	}

}

//these two are for transfers between cache and MM 
void block_write(int u,int mini)
{
	//block write to MM from here 
	unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
	pre=pre|(indx<<blockbits[u]); 
	for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
	{
		write_data=c[u][i];
		write_to_MM(pre|j);
	} 
	//block write to MM till here
}

void block_read(int u,int mini)
{
	//block read from MM from here
	unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
	pre=pre|(indx<<blockbits[u]); 
	for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
	{
		read_from_MM(pre|j);
		c[u][i]=read_data;
	}
	//block read from MM till here
}
