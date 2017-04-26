#include "cache.h"
#include "main.h"

/* cache configuration parameters */
int cache_split = 0;
int cache_usize = DEFAULT_CACHE_SIZE;
int cache_isize = DEFAULT_CACHE_SIZE; 
int cache_dsize = DEFAULT_CACHE_SIZE;
int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
int cache_assoc = DEFAULT_CACHE_ASSOC;
int cache_writeback = DEFAULT_CACHE_WRITEBACK;
int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;


static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(int param,int value)
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/

//sets default values of cache parameters
void set_default()
{
	cache_split = 0;
	cache_usize = DEFAULT_CACHE_SIZE;
	cache_isize = DEFAULT_CACHE_SIZE; 
	cache_dsize = DEFAULT_CACHE_SIZE;
	cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
	words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
	cache_assoc = DEFAULT_CACHE_ASSOC;
	cache_writeback = DEFAULT_CACHE_WRITEBACK;
	cache_writealloc = DEFAULT_CACHE_WRITEALLOC;
} 

//allocates space to all variables 
void init_vars()
{


	c=(char**)calloc((2),sizeof(char*));
	tag=(int**)calloc((2),sizeof(int*));
	lru=(int**)calloc((2),sizeof(int*));
	v=(bool**)calloc((2),sizeof(bool*));
	dirty=(bool**)calloc((2),sizeof(bool*));


	int sizei,sized;
	if(cache_split==0)
	{sizei=sized=cache_usize+2*cache_block_size*cache_assoc+10;}
	else if(cache_split==1)
	{sizei=cache_isize+2*cache_block_size*cache_assoc+10;
	sized=cache_dsize+2*cache_block_size*cache_assoc+10;}
	else
	{printf("Error - cache_split has invalid value:%d\n",cache_split);} 


	c[0]=(char*)calloc((sizei),sizeof(char));
	c[1]=(char*)calloc((sized),sizeof(char));
	tag[0]=(int*)calloc((sizei),sizeof(int));
	tag[1]=(int*)calloc((sized),sizeof(int));
	lru[0]=(int*)calloc((sizei),sizeof(int));
	lru[1]=(int*)calloc((sized),sizeof(int));
	v[0]=(bool*)calloc((sizei),sizeof(bool));
	v[1]=(bool*)calloc((sized),sizeof(bool));
	dirty[0]=(bool*)calloc((sizei),sizeof(bool));
	dirty[1]=(bool*)calloc((sized),sizeof(bool));

	MMi=(char*)calloc((700100),sizeof(char));
	MMd=(char*)calloc((70000100),sizeof(char));


	numiref=numdref=0;
	numimiss=numdmiss=0;
	numireplace=numdreplace=0;
	demand_fetches[0]=copies_back[0]=0;
	demand_fetches[1]=copies_back[1]=0;

	curr[0]=curr[1]=1;

}

//initialises the number of tagbits,indexbits,blockbits
void init_cache()
{

	if(cache_split==0 && cache_usize % (cache_assoc*cache_block_size)!=0)
	{
		fprintf(stderr,"Error - cache_usize is not divisible by assocciativity*block_size\n");
		fflush(stderr);
	}
	else if(cache_split==1 && cache_isize % (cache_assoc*cache_block_size)!=0)
	{
		fprintf(stderr,"Error - cache_isize is not divisible by assocciativity*block_size\n");
		fflush(stderr);
	}
	else if(cache_split==1 && cache_usize % (cache_assoc*cache_block_size)!=0)
	{
		fprintf(stderr,"Error - cache_usize is not divisible by assocciativity*block_size\n");
		fflush(stderr);
	}


	if(cache_split==0)//unified cache
	{
		blockbits[0]=LOG2(cache_block_size);

		setsize[0]=cache_assoc*cache_block_size;
		numsets[0]=cache_usize/(setsize[0]);

		indxbits[0]=LOG2(numsets[0]);
		tagbits[0]=32-(blockbits[0]+indxbits[0]);

		//printf("cache_usize=%d setsize[0]=%d\n",cache_usize,setsize[0]);

	}
	else if(cache_split==1)//split cache
	{
		//for I
		blockbits[0]=LOG2(cache_block_size);

		setsize[0]=cache_assoc*cache_block_size;
		numsets[0]=cache_isize/(setsize[0]);

		indxbits[0]=LOG2(numsets[0]);
		tagbits[0]=32-(blockbits[0]+indxbits[0]);


		//for D	
		blockbits[1]=LOG2(cache_block_size);

		setsize[1]=cache_assoc*cache_block_size;
		numsets[1]=cache_dsize/(setsize[1]);

		indxbits[1]=LOG2(numsets[1]);
		tagbits[1]=32-(blockbits[1]+indxbits[1]);


	}

	//	words_per_block = cache_block_size / 4;

	// printf("%d %d %d \n",tagbits[u],indxbits[u],blockbits[u]);
}
/************************************************************/


/*
global variables:- read_data,write_data
these are interface between processor and cache and memory.
*/

void WB_WA(unsigned addr,unsigned access_type,int u)
{
	// v[i] and tag[i] are valid only for i which are multiples of cache_block_size 
	int hit=0;
	for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
	{
		if(v[u][i]==true && tag[u][i]==atag)//hit
		{
			hit=1;

			if(access_type==0)// d rd hit
			{
				//read from c[u][i+bo];
				//read_from_cache(u,i+bo);
				lru[u][i]=curr[u];
			}
			else if(access_type==1)// d wr hit 
			{
				//write in c[u][i+bo];
				//write_to_cache(u,i+bo);
				lru[u][i]=curr[u];			
				dirty[u][i]=true;
			}
			else if(access_type==2)// i rd hit 
			{
				//read from c[u][i+bo];
				//read_from_cache(u,i+bo);
				lru[u][i]=curr[u];
				int mem=c[u][i+bo];
				mem=mem+1;
			}

			break;//have found the memory location
		}
	}


  if(hit==0)//miss
  {

    //choose which block to remove
    unsigned minlru=curr[u]+10,mini=(indx+1)*setsize[u];
    for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
    {
      if(lru[u][i]<minlru)
      {minlru=lru[u][i];mini=i;}
    }


      if(v[u][mini]==true)
      {    

        if(access_type==0 || access_type==1)
        {++numdreplace;}
        else if(access_type==2)
        {++numireplace;}     
      }  

    //write to main memory the block from [mini] to [mini+cache_block_size-1]        
    {
      if(dirty[u][mini]==true)
      {
        //block has to be written to MM


        if(access_type==0 || access_type==1)
        {copies_back[u]+=words_per_block;}
        else if(access_type==2)
        {copies_back[u]+=words_per_block;}     


	/*	//block write to MM from here 
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//write_data=c[u][i];
			//write_to_MM(pre|j);
		} 
		//block write to MM till here
	*/
		block_write(u,mini);

      }

    }

    //read the block to cache from main memory
    {

	/*	//block read from MM from here
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//read_from_MM(pre|j);
			//c[u][i]=read_data;
		}
		//block read from MM till here
	*/
		block_read(u,mini);


		tag[u][mini]=atag;
		v[u][mini]=true;
		//dirty[u][i]=true;
		lru[u][mini]=curr[u];

      if(access_type==0 || access_type==1)
      {demand_fetches[u]+=words_per_block;}
      else if(access_type==2)
      {demand_fetches[u]+=words_per_block;}

    }


      if(access_type==0)// d rd miss
      {

		//read from c[u][mini+bo]
		//read_from_cache(u,mini+bo);

        ++numdmiss;

        dirty[u][mini]=false;
        v[u][mini]=true;

      }
      else if(access_type==1)// d wr miss 
      {

		//write to cache at c[u][mini+bo]
		//write_to_cache(u,mini+bo);

		++numdmiss;

		dirty[u][mini]=true;
        v[u][mini]=true;

      }
      else if(access_type==2)// i rd miss
      {

		//read from c[u][mini+bo]
		//read_from_cache(u,mini+bo);

		++numimiss;

		dirty[u][mini]=false;
		v[u][mini]=true;

      }



	}

}

void WB_WNA(unsigned addr,unsigned access_type,int u)
{
	// v[i] and tag[i] are valid only for i which are multiples of cache_block_size 
	int hit=0;
  for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
  {
    if(v[u][i]==true && tag[u][i]==atag)//hit
    {
      hit=1;
      
		if(access_type==0)// d rd hit
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];
		}
		else if(access_type==1)// d wr hit 
		{
			//write in c[u][i+bo];
			//write_to_cache(u,i+bo);
			lru[u][i]=curr[u];
			dirty[u][i]=true;

		}
		else if(access_type==2)// i rd hit 
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];
		}

        break;//have found the memory location
    }
  }


  if(hit==0)//miss
  {

    //choose which block to remove
    unsigned minlru=curr[u]+10,mini=0;
    for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
    {
      if(lru[u][i]<minlru)
      {minlru=lru[u][i];mini=i;}
    }


		if(v[u][mini]==true)
		{    

		if(access_type==0 /*|| access_type==1*/)
		{++numdreplace;}
		else if(access_type==2)
		{++numireplace;}     
		}  


      if(access_type==0 || access_type==2)
      {
    //write to main memory the block from [mini] to [mini+cache_block_size-1]        
    {
      if(dirty[u][mini]==true)
      {
        //block has to be written to MM



        if(access_type==0 /*|| access_type==1*/)
        {copies_back[u]+=words_per_block;}
        else if(access_type==2)
        {copies_back[u]+=words_per_block;}     


	/*	//block write to MM from here 
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//write_data=c[u][i];
			//write_to_MM(pre|j);
		} 
		//block write to MM till here
	*/
		block_write(u,mini);


      }

    }

    //write the block to cache from main memory
    {
      for(int i=mini ; i<(mini+cache_block_size) ; ++i)
      {c[u][i]=1;/*=from memory*/

		tag[u][i]=atag;
		v[u][i]=true;
		//dirty[u][i]=true;
		lru[u][i]=curr[u];}

      if(access_type==0 || access_type==1)
      {demand_fetches[u]+=words_per_block;}
      else if(access_type==2)
      {demand_fetches[u]+=words_per_block;}

    }

	}

      if(access_type==0)// d rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);

        ++numdmiss;

        dirty[u][mini]=false;
        v[u][mini]=true;

      }
      else if(access_type==1)// d wr miss 
      {

		//write to MM
		//write_to_MM(addr);

        copies_back[u]+=1;

        ++numdmiss;
      }
      else if(access_type==2)// i rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);

        ++numimiss;

        dirty[u][mini]=false;
        v[u][mini]=true;

      }



  }

}

void WT_WA(unsigned addr,unsigned access_type,int u)
{
	// v[i] and tag[i] are valid only for i which are multiples of cache_block_size 
	int hit=0;
  for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
  {
    if(v[u][i]==true && tag[u][i]==atag)//hit
    {
      hit=1;
      
		if(access_type==0)// d rd hit
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];
		}
		else if(access_type==1)// d wr hit 
		{
			//write in c[u][i+bo];
			//write_to_cache(u,i+bo);
			lru[u][i]=curr[u];

			//write to MM
			//write_to_MM(addr);

		dirty[u][i]=false;
		copies_back[u]+=1;

		}
		else if(access_type==2)// i rd hit 
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];
		}

        break;//have found the memory location
    }
  }


  if(hit==0)//miss
  {

    //choose which block to remove
    unsigned minlru=curr[u]+10,mini=0;
    for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
    {
      if(lru[u][i]<minlru)
      {minlru=lru[u][i];mini=i;}
    }


      if(v[u][mini]==true)
      {    

        if(access_type==0 || access_type==1)
        {++numdreplace;}
        else if(access_type==2)
        {++numireplace;}     
      }  

    //write to main memory the block from [mini] to [mini+cache_block_size-1]        
    /*{
      if(dirty[u][mini]==true && access_type==1)
      {
        //block has to be written to MM



        if(access_type==0 || access_type==1)
        {copies_back[u]+=words_per_block;}
        else if(access_type==2)
        {copies_back[u]+=words_per_block;}     


		//block write to MM from here 
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//write_data=c[u][i];
			//write_to_MM(pre|j);
		} 
		//block write to MM till here
	 //
		block_write(u,mini);


      }

    }*/


    //read the block to cache from main memory
    {

	/*	//block read from MM from here
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//read_from_MM(pre|j);
			//c[u][i]=read_data;
		}
		//block read from MM till here
	*/
		block_read(u,mini);


		tag[u][mini]=atag;
		v[u][mini]=true;
		//dirty[u][i]=true;
		lru[u][mini]=curr[u];

      if(access_type==0 || access_type==1)
      {demand_fetches[u]+=words_per_block;}
      else if(access_type==2)
      {demand_fetches[u]+=words_per_block;}

    }


      if(access_type==0)// d rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);
	
        ++numdmiss;
        dirty[u][mini]=false;

        v[u][mini]=true;

      }
      else if(access_type==1)// d wr miss 
      {

		//write byte to cache at c[u][mini+bo];
		//write_to_cache(u,mini+bo);
    	
        
        //write byte to MM;
        //write_to_MM(addr);

		++numdmiss;
        
        dirty[u][mini]=false;
        copies_back[u]+=1;
		//demand_fetches[u]+=1;
        v[u][mini]=true;
        

      }
      else if(access_type==2)// i rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);

        ++numimiss;

        dirty[u][mini]=false;
        v[u][mini]=true;

      }



  }

}


void WT_WNA(unsigned addr,unsigned access_type,int u)
{
	// v[i] and tag[i] are valid only for i which are multiples of cache_block_size 
	int hit=0;
  for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
  {
    if(v[u][i]==true && tag[u][i]==atag)//hit
    {
      hit=1;
      
		if(access_type==0)// d rd hit
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];
		}
		else if(access_type==1)// d wr hit 
		{
			//write in c[u][i+bo];
			//write_to_cache(u,i+bo);
			lru[u][i]=curr[u];

			//write to MM
			//write_to_MM(addr);

		dirty[u][i]=false;
		copies_back[u]+=1;

		}
		else if(access_type==2)// i rd hit 
		{
			//read from c[u][i+bo];
			//read_from_cache(u,i+bo);
			lru[u][i]=curr[u];			
		}

        break;//have found the memory location
    }
  }


  if(hit==0)//miss
  {

    //choose which block to remove
    unsigned minlru=curr[u]+10,mini=0;
    for(int i=(indx+1)*setsize[u] ; i<end ; i+=cache_block_size)
    {
      if(lru[u][i]<minlru)
      {minlru=lru[u][i];mini=i;}
    }


      if(v[u][mini]==true)
      {    

        if(access_type==0 || access_type==1)
        {++numdreplace;}
        else if(access_type==2)
        {++numireplace;}     
      }  

    //write to main memory the block from [mini] to [mini+cache_block_size-1]        
    /*{
      if(dirty[u][mini]==true && access_type==1)
      {
        //block has to be written to MM



        if(access_type==0 || access_type==1)
        {copies_back[u]+=words_per_block;}
        else if(access_type==2)
        {copies_back[u]+=words_per_block;}     


		//block write to MM from here 
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//write_data=c[u][i];
			//write_to_MM(pre|j);
		} 
		//block write to MM till here
	 //
		block_write(u,mini);


      }

    }*/


    //read the block to cache from main memory
    if(access_type==0 || access_type==2)  
    {

	/*	//block read from MM from here
		unsigned pre=(tag[u][mini]<<(indxbits[u]+blockbits[u]));
		pre=pre|(indx<<blockbits[u]); 
		for(unsigned i=mini,j=0 ; i<(mini+cache_block_size) ; ++i,++j)
		{
			//read_from_MM(pre|j);
			//c[u][i]=read_data;
		}
		//block read from MM till here
	*/
		block_read(u,mini);


		tag[u][mini]=atag;
		v[u][mini]=true;
		//dirty[u][i]=true;
		lru[u][mini]=curr[u];

      if(access_type==0 || access_type==1)
      {demand_fetches[u]+=words_per_block;}
      else if(access_type==2)
      {demand_fetches[u]+=words_per_block;}

    }


      if(access_type==0)// d rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);
	
        ++numdmiss;
        dirty[u][mini]=false;

        v[u][mini]=true;

      }
      else if(access_type==1)// d wr miss 
      {

		//write byte to cache at c[u][mini+bo];
		//write_to_cache(u,mini+bo);
    	
        
        //write byte to MM;
        //write_to_MM(addr);

		++numdmiss;
        
		//dirty[u][mini]=false;
        copies_back[u]+=1;
		//demand_fetches[u]+=1;
        //v[u][mini]=true;
        

      }
      else if(access_type==2)// i rd miss
      {

		//read from c[u][i+bo];
		//read_from_cache(u,i+bo);

        ++numimiss;

        dirty[u][mini]=false;
        v[u][mini]=true;

      }



  }

}

/************************************************************/
//I is 0 and D is 1
//Instruction is 0 and Data is 1
//0 is also for unified cache
void perform_access(unsigned addr,unsigned access_type)
{
	int u=0;
	if(cache_split==1)
	{

		if(access_type==2)//instruction
		{u=0;}		
		else if(access_type==0 || access_type==1)//data
		{u=1;}

	}
	else if(cache_split==0)
	{u=0;}


	++curr[u];


	t1=(addr<<tagbits[u]);
	t1=(t1>>(tagbits[u]+blockbits[u]));
	indx=t1-1;//indx of the set


	t2=(addr<<(tagbits[u]+indxbits[u]));
	t2=(t2>>(tagbits[u]+indxbits[u]));
	bo=t2;//block offset


	atag=(addr>>(indxbits[u]+blockbits[u]));


	end=(indx+2)*setsize[u];


	if(access_type==0 || access_type==1)
	{++numdref;}
	else if(access_type==2)
	{++numiref;}

	

	if(cache_writeback==1 && cache_writealloc==1)
	{WB_WA(addr,access_type,u);}
	else if(cache_writeback==1 && cache_writealloc==0)
	{WB_WNA(addr,access_type,u);}
	else if(cache_writeback==0 && cache_writealloc==1)
	{WT_WA(addr,access_type,u);}
	else if(cache_writeback==0 && cache_writealloc==0)
	{WT_WNA(addr,access_type,u);}

			
}

/************************************************************/

/************************************************************/
void flush(int u)
{
  unsigned i=0,end=(numsets[u])*setsize[u];

    for(i=0 ; i<end ; i+=cache_block_size)
    {
      if(dirty[u][i]==true)
      {
        unsigned j=i;
        for(j=i ; j<(i+cache_block_size) ; ++j)
        {
          //mem[...]=c[j]
        }

        copies_back[u]+=words_per_block;
      }
    }

}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("*** CACHE SETTINGS ***\n");
  if (cache_split) {
    printf("  Split I- D-cache\n");
    printf("  I-cache size: \t%d\n", cache_isize);
    printf("  D-cache size: \t%d\n", cache_dsize);
  } else {
    printf("  Unified I- D-cache\n");
    printf("  Size: \t%d\n", cache_usize);
  }
  printf("  Associativity: \t%d\n", cache_assoc);
  printf("  Block size: \t%d\n", cache_block_size);
  printf("  Write policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("  Allocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
  fflush(stdout);
}
/************************************************************/

/************************************************************/
void print_stats()
  {

  cache_stat_inst.accesses=numiref;
  cache_stat_inst.misses=numimiss;
  cache_stat_inst.replacements=numireplace;

  cache_stat_data.accesses=numdref;
  cache_stat_data.misses=numdmiss;
  cache_stat_data.replacements=numdreplace;

  printf("\n*** CACHE STATISTICS ***\n");

  printf(" INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  if (!cache_stat_inst.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf(" DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  if (!cache_stat_data.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);


  cache_stat_inst.demand_fetches=demand_fetches[0];
  cache_stat_inst.copies_back=copies_back[0];

  cache_stat_data.demand_fetches=demand_fetches[1];
  cache_stat_data.copies_back=copies_back[1];

  // cache_stat_data.demand_fetches/=32;
  // cache_stat_data.copies_back/=32;

  printf(" TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/
