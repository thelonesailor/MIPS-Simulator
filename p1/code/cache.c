
#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
// static Pcache icache;
// static Pcache dcache;
// static cache c1;
// static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
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

void init_vars()
{

  c=(char**)calloc((2),sizeof(char*));
  tag=(int**)calloc((2),sizeof(int*));
  lru=(int**)calloc((2),sizeof(int*));
  v=(bool**)calloc((2),sizeof(bool*));
  dirty=(bool**)calloc((2),sizeof(bool*));

  c[0]=(char*)calloc((cache_usize+10),sizeof(char));
  c[1]=(char*)calloc((cache_usize+10),sizeof(char));
  tag[0]=(int*)calloc((cache_usize+10),sizeof(int));
  tag[1]=(int*)calloc((cache_usize+10),sizeof(int));
  lru[0]=(int*)calloc((cache_usize+10),sizeof(int));
  lru[1]=(int*)calloc((cache_usize+10),sizeof(int));
  v[0]=(bool*)calloc((cache_usize+10),sizeof(bool));
  v[1]=(bool*)calloc((cache_usize+10),sizeof(bool));
  dirty[0]=(bool*)calloc((cache_usize+10),sizeof(bool));
  dirty[1]=(bool*)calloc((cache_usize+10),sizeof(bool));


  numiref=numdref=0;
  numimiss=numdmiss=0;
  numireplace=numdreplace=0;
  demand_fetches[0]=copies_back[0]=0;
  demand_fetches[1]=copies_back[1]=0;

  curr[0]=curr[1]=1;

}

void init_cache()
{
  init_vars();

  if( cache_usize % (cache_assoc*cache_block_size)!=0)
  {
    fprintf(stderr,"Error - cache_size is not divisible by assocciativity*block_size\n");
    fflush(stderr);exit(-1);
  }

  blockbits[0]=LOG2(cache_block_size);

  setsize[0]=cache_assoc*cache_block_size;
  numsets[0]=cache_usize/(setsize[0]);
  
  indexbits[0]=LOG2(numsets[0]);
  tagbits[0]=32-(blockbits[0]+indexbits[0]);

  // printf("%d %d %d \n",tagbits[0],indexbits[0],blockbits[0]);
}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{

  ++curr[0];

  // printf("5\n");fflush(stdout);  

  unsigned t1=(addr<<tagbits[0]);
  t1=(t1>>(tagbits[0]+blockbits[0]));
  unsigned index=t1;

  // printf("6\n");fflush(stdout);  

  unsigned t2=(addr<<(tagbits[0]+indexbits[0]));
  t2=(t2>>(tagbits[0]+indexbits[0]));
  unsigned bo=t2;

  // printf("7\n");fflush(stdout);  

  unsigned atag=(addr>>(indexbits[0]+blockbits[0]));


  unsigned i=0,end=(index+2)*setsize[0],hit=0;
  

  if(access_type==0 || access_type==1)
  {++numdref;}
  else if(access_type==2)
  {++numiref;}


  // v[i] and tag[i] are valid only for i which are multiples of cache_block_size 
  for(i=(index+1)*setsize[0] ; i<end ; i+=cache_block_size)
  {
    if(v[0][i]==true && tag[0][i]==atag)//hit
    {
      hit=1;
      
      if(access_type==0)// d rd hit
      {
        //read from [i+bo]
      }
      else if(access_type==1)// d wr hit 
      {
        //write in c[i+bo]
        //if(cache_writeback==1)
        {dirty[0][i]=true;}

      }
      else if(access_type==2)// i rd hit 
      {
        //read from [i+bo]
        int mem=c[0][i+bo];
        mem=mem+1;

      }

        break;//have found the memory location
    }
  }

  if(hit==0)//miss
  {

    //choose which block to remove
    unsigned minlru=curr[0]+10,mini=0;
    for(i=(index+1)*setsize[0] ; i<end ; i+=cache_block_size)
    {
      if(lru[0][i]<minlru)
      {minlru=lru[0][i];mini=i;}
    }

      if(v[0][mini]==true)
      {    

        if(access_type==0 || access_type==1)
        {++numdreplace;}
        else if(access_type==2)
        {++numireplace;}     
      }  

    //write to main memory the block from [mini] to [mini+cache_block_size-1]        
    {
      if(dirty[0][mini]==true)
      {
        //block has to be written to MM



        if(access_type==0 || access_type==1)
        {copies_back[1]+=words_per_block;}
        else if(access_type==2)
        {copies_back[0]+=words_per_block;}     


        for(i=mini ; i<(mini+cache_block_size) ; ++i)
        {
          //mem=
        } 

      }

    }

    //write the block to cache from main memory
    {
      for(i=mini ; i<(mini+cache_block_size) ; ++i)
      {c[0][i]=1;/*from memory*/

      tag[0][i]=atag;
      v[0][i]=true;
//      dirty[0][i]=true;
      lru[0][i]=curr[0];}

      if(access_type==0 || access_type==1)
      {demand_fetches[1]+=words_per_block;}
      else if(access_type==2)
      {demand_fetches[0]+=words_per_block;}

    }


      if(access_type==0)// d rd miss
      {

        //read from [mini+bo]
        ++numdmiss;
        dirty[0][mini]=false;

        v[0][mini]=true;

      }
      else if(access_type==1)// d wr miss 
      {

        //if(cache_writeback==1)
        {dirty[0][mini]=true;}
        
        v[0][mini]=true;
        //write to cache at [mini+bo]
        
        ++numdmiss;
      }
      else if(access_type==2)// i rd miss
      {

        //read from [mini+bo]
        ++numimiss;
        dirty[0][mini]=false;

        v[0][mini]=true;

      }



  }

}

/************************************************************/

/************************************************************/
void flush()
{
  unsigned i=0,end=(numsets[0])*setsize[0];

    for(i=0 ; i<end ; i+=cache_block_size)
    {
      if(dirty[0][i]==true)
      {
        unsigned j=i;
        for(j=i ; j<(i+cache_block_size) ; ++j)
        {
          //mem[...]=c[j]
        }

        copies_back[0]+=words_per_block;
      }
    }

}
/************************************************************/

/************************************************************/
void delete(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
  {
    if (item->LRU_prev) {
      item->LRU_prev->LRU_next = item->LRU_next;
    } else {
      /* item at head */
      *head = item->LRU_next;
    }

    if (item->LRU_next) {
      item->LRU_next->LRU_prev = item->LRU_prev;
    } else {
      /* item at tail */
      *tail = item->LRU_prev;
    }
  }
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
  {
    item->LRU_next = *head;
    item->LRU_prev = (Pcache_line)NULL;

    if (item->LRU_next)
      item->LRU_next->LRU_prev = item;
    else
      *tail = item;

    *head = item;
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
