#include "sim.h"
#include "cache.h"

//store instructions in data memory
void init_data_memory()
{

}


void input_cfg(FILE * incfg)
{
	if(incfg==NULL)
	{printf("Error - incfg is NULL\n");exit(-99);}

	char ch[1000];

	//fscanf(incfg,"# Common config file for MIPs core");
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);
	fscanf(incfg,"%s",ch);


	fscanf(incfg,"%s",ch);

	bool perfect=false;int temp;

	if(strcmp(ch,"[perf_model/l1_icache]")==0)
	{
		cache_split=1;

		fscanf(incfg,"perfect =");fscanf(incfg,"%s",ch);
		
		if(strcmp(ch,"false")==0)
		{cache_writealloc[0]=1;}
		else if(strcmp(ch,"true")==0)
		{cache_writealloc[0]=1;}
		else
		{printf("Wrong input for perfect\n");}

		fscanf(incfg,"cache_size =");fscanf(incfg,"%d",cache_isize);
		fscanf(incfg,"associativity =");fscanf(incfg,"%d",cache_assoc[0]);
		fscanf(incfg,"replacement_policy =");fscanf(incfg,"%s",ch);

		fscanf(incfg,"writethrough =");fscanf(incfg,"%d",temp);cache_writeback[0]=1-temp;
		fscanf(incfg,"cache_block_size =");fscanf(incfg,"%d",cache_block_size[0]);


	}
	else if(strcmp(ch,"[perf_model/l1_dcache]")==0)
	{
		cache_split=1;

		fscanf(incfg,"perfect =");fscanf(incfg,"%s",ch);
		
		if(strcmp(ch,"false")==0)
		{cache_writealloc[1]=1;}
		else if(strcmp(ch,"true")==0)
		{cache_writealloc[1]=1;}

		fscanf(incfg,"cache_size =");fscanf(incfg,"%d",cache_dsize);
		fscanf(incfg,"associativity =");fscanf(incfg,"%d",cache_assoc[1]);
		fscanf(incfg,"replacement_policy =");fscanf(incfg,"%s",ch);

		fscanf(incfg,"writethrough =");fscanf(incfg,"%d",temp);cache_writeback[1]=1-temp;
		fscanf(incfg,"cache_block_size =");fscanf(incfg,"%d",cache_block_size[1]);

	}
	else if(strcmp(ch,"[perf_model/core]")==0)
	{
		fscanf(incfg,"frequency =");fscanf(incfg,"%f",fghz);
	}
	else if(strcmp(ch,"[perf_model/dram]")==0)
	{
		fscanf(incfg,"latency =");fscanf(incfg,"%d",latency);
	}
	else
	{
		printf("Error - unrecognized string %s\n",ch);
	}
	
}