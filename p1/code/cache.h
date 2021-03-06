#ifndef CACHE_H
#define CACHE_H


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/* default cache parameters--can be changed */
#define WORD_SIZE 4
#define WORD_SIZE_OFFSET 2
#define DEFAULT_CACHE_SIZE (8 * 1024)
#define DEFAULT_CACHE_BLOCK_SIZE 16
#define DEFAULT_CACHE_ASSOC 1
#define DEFAULT_CACHE_WRITEBACK TRUE
#define DEFAULT_CACHE_WRITEALLOC TRUE

/* constants for settting cache parameters */
#define CACHE_PARAM_BLOCK_SIZE 0
#define CACHE_PARAM_USIZE 1
#define CACHE_PARAM_ISIZE 2
#define CACHE_PARAM_DSIZE 3
#define CACHE_PARAM_ASSOC 4
#define CACHE_PARAM_WRITEBACK 5
#define CACHE_PARAM_WRITETHROUGH 6
#define CACHE_PARAM_WRITEALLOC 7
#define CACHE_PARAM_NOWRITEALLOC 8


int cache_split;
int cache_usize;
int cache_isize; 
int cache_dsize;
int cache_block_size;
int words_per_block;
int cache_assoc;
int cache_writeback;
int cache_writealloc;

/* structure definitions */
typedef struct cache_line_ {
  unsigned tag;
  int dirty;

  struct cache_line_ *LRU_next;
  struct cache_line_ *LRU_prev;
} cache_line, *Pcache_line;

typedef struct cache_ {
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  unsigned index_mask;		/* mask to find cache index */
  int index_mask_offset;	/* number of zero bits in mask */
  Pcache_line *LRU_head;	/* head of LRU list for each set */
  Pcache_line *LRU_tail;	/* tail of LRU list for each set */
  int *set_contents;		/* number of valid entries in set */
  int contents;			/* number of valid entries in cache */
} cache, *Pcache;

typedef struct cache_stat_ {
  int accesses;			/* number of memory references */
  int misses;			/* number of cache misses */
  int replacements;		/* number of misses that cause replacments */
  int demand_fetches;		/* number of fetches */
  int copies_back;		/* number of write backs */
} cache_stat, *Pcache_stat;


char** c;
int** tag;
bool** v;
bool** dirty;
int** lru;
int tagbits[2],indxbits[2],blockbits[2];//number of bits for tag,index,block offset.
int numsets[2],setsize[2];


int numiref,numdref;
int numimiss,numdmiss;
int curr[2];
int numireplace,numdreplace;
int demand_fetches[2],copies_back[2];



unsigned t1, indx;
unsigned t2, bo;
unsigned atag;
unsigned end;



/* function prototypes */
void set_default();
void set_cache_param(int param,int value);
void init_vars();
void init_cache();
void perform_access(unsigned addr,unsigned access_type);
void flush(int u);
void delete();
void insert();
void dump_settings();
void print_stats();


void write_to_cache(int u,int idx);
void read_from_cache(int u,int idx);
void write_to_MM(unsigned addr);
void read_from_MM(unsigned addr);
void block_write(int u,int mini);
void block_read(int u,int mini);


char* MMi;
char* MMd;
char read_data,write_data;

/* macros */
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))


#endif