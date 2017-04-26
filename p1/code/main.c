
#include <stdlib.h>
#include <stdio.h>
#include "cache.h"
#include "main.h"

FILE *traceFile;
int arg_index;

int main(int argc,char ** argv)
{

  //these have to be called only once
    set_default();
    parse_args(argc, argv);
    
    init_vars();//allocate space
    init_cache();
  //don't change the order
      
  
  if(cache_split==0)//unified cache  
  {
    traceFile = fopen(argv[arg_index], "r");
    if(traceFile==NULL)
    {
        fprintf(stderr,"Error - traceFile is NULL \"%s\"\n",argv[arg_index]);
        fflush(stderr);
    }

    play_trace(traceFile);
    print_stats();
  }
  else if(cache_split==1)//split cache
  { 

      traceFile = fopen(argv[arg_index], "r");
      if(traceFile==NULL)
      {
          fprintf(stderr,"Error - traceFile is NULL \"%s\"\n",argv[arg_index]);
          fflush(stderr);
      }

      play_trace(traceFile);
    

    print_stats();
  }
  else
  {printf("Error - cache_split is=%d.\n",cache_split);}

}


/************************************************************/
void parse_args(argc, argv)
  int argc;
  char **argv;
  {
  int i, value;

  if (argc < 2) {
    printf("Error - usage:  sim <options> <trace file>\n");
    //exit(-1);
  }

  /* parse the command line arguments */
  for (i = 0; i < argc; i++)//help
    if (!strcmp(argv[i], "-h")) {
      printf("\t-h:  \t\tthis message\n\n");
      printf("\t-bs <bs>: \tset cache block size to <bs>\n");
      printf("\t-us <us>: \tset unified cache size to <us>\n");
      printf("\t-is <is>: \tset instruction cache size to <is>\n");
      printf("\t-ds <ds>: \tset data cache size to <ds>\n");
      printf("\t-a <a>: \tset cache associativity to <a>\n");
      printf("\t-wb: \t\tset write policy to write back\n");
      printf("\t-wt: \t\tset write policy to write through\n");
      printf("\t-wa: \t\tset allocation policy to write allocate\n");
      printf("\t-nw: \t\tset allocation policy to no write allocate\n");
      exit(0);
    }
    
  arg_index = 1;
  while (arg_index </*!=*/ argc - 1) {

    /* set the cache simulator parameters */

    if (!strcmp(argv[arg_index], "-bs")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_BLOCK_SIZE, value);
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-us")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_USIZE, value);
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-is")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_ISIZE, value);
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-ds")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_DSIZE, value);
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-a")) {
      value = atoi(argv[arg_index+1]);
      set_cache_param(CACHE_PARAM_ASSOC, value);
      arg_index += 2;
      continue;
    }

    if (!strcmp(argv[arg_index], "-wb")) {
      set_cache_param(CACHE_PARAM_WRITEBACK, value);
      arg_index += 1;
      continue;
    }

    if (!strcmp(argv[arg_index], "-wt")) {
      set_cache_param(CACHE_PARAM_WRITETHROUGH, value);
      arg_index += 1;
      continue;
    }

    if (!strcmp(argv[arg_index], "-wa")) {
      set_cache_param(CACHE_PARAM_WRITEALLOC, value);
      arg_index += 1;
      continue;
    }

    if (!strcmp(argv[arg_index], "-nw")) {
      set_cache_param(CACHE_PARAM_NOWRITEALLOC, value);
      arg_index += 1;
      continue;
    }

    printf("error:  unrecognized flag %s\n", argv[arg_index]);
    exit(-1);

  }

  dump_settings();


  return;
}
/************************************************************/

/************************************************************/
void play_trace(FILE * inFile)
  {
  unsigned addr, access_type;
  int num_inst;

  num_inst = 0;

  if(inFile==NULL)
  {
      fprintf(stderr,"Error - inFile is NULL\n");
      fflush(stderr);

        if(traceFile==NULL)
  {
      fprintf(stderr,"Error - traceFile is NULL\n");
      fflush(stderr);
  }
  }

  while(read_trace_element(inFile, &access_type, &addr)) {

    switch (access_type) {
    case TRACE_DATA_LOAD:
    case TRACE_DATA_STORE:
    case TRACE_INST_LOAD:
      perform_access(addr, access_type);
      break;

    default:
      fprintf(stderr,"skipping access, unknown type(%d)\n", access_type);
      fflush(stderr);
    }

    num_inst++;
    if (!(num_inst % PRINT_INTERVAL))
      printf("processed %d references\n", num_inst);
  }


  if(cache_split==0)//unified cache  
  {flush(0);}
  else if(cache_split==1)//split cache  
  {flush(0);flush(1);}
}
/************************************************************/

/************************************************************/
int read_trace_element(inFile, access_type, addr)
  FILE *inFile;
  unsigned *access_type, *addr;
  {
  int result;
  char c;

  result = fscanf(inFile, "%u %x%c", access_type, addr, &c);
  while (c != '\n') {
    result = fscanf(inFile, "%c", &c);
    if (result == EOF) 
      break;
  }
  if (result != EOF)
    return(1);
  else
    return(0);
}
/************************************************************/
