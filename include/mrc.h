
/*
 * Header file describing the 
 * miss ratio curve algorithm by Hu et al (USENIX ATC '16)
 *
 * The mrc_t is main structure describing the miss ratio curve
 *
 */
#include "uthash.h"
#include <pthread.h>
#include <stdint.h>

#define domain 256
#define MAXH 100003
#define MAXL 1000000

typedef struct ref_t
{
    uint64_t reference;
    uint64_t time;
    uint8_t pf;
    UT_hash_handle hh; //make it hashable for UT HASH

} ref_t;

typedef struct ru_t
{
    uint64_t time;
	uint64_t count;
    UT_hash_handle hh; //make it hashable for UT HASH

} ru_t;

typedef struct rpt_t
{
    uint64_t pc;
	uint64_t curr_addr;
	uint64_t prev_addr;
	int state;
	int64_t stride;
    uint64_t time;
    UT_hash_handle hh; //make it hashable for UT HASH

} rpt_t;


typedef struct mrc_t 
{
    uint64_t sample_rate;
    uint64_t mrc_interval;
    uint64_t mrc_size; //length of mrc

    double *mrc; //the actual mrc
    uint64_t wss; //working set size
    uint64_t max_rt; //max reuse time recorded

    //AET specific parameters
    uint64_t n; //total number of references
    double N; //sum
    uint64_t m; //number of cold misses
    uint64_t node_cnt;
    uint64_t cm; //actual cold misses
    uint64_t last_cm; //actual cold misses
    uint64_t pfs; //number of prefetches
    uint64_t node_max;
    uint64_t loc; //sample location
    uint64_t tott; //total in hash table

    uint64_t *rtd; //reuse time dist
    ref_t *hash; //last use of a reference (hash table)
    ref_t *hash_pf; //table of prefetched addrs
    
    rpt_t *rpt; //last use of a reference (hash table)

    ru_t *reuse_times;


} mrc_t;


mrc_t* init_mrc(uint64_t a_sample_rate, 
                uint64_t a_mrc_interval); 

void take_sample(mrc_t *a_mrc, uint64_t a_reference, int64_t stride, int multi);
void take_sample_pf(mrc_t *a_mrc, uint64_t a_reference, int64_t stride, int multi);
int64_t get_stride(mrc_t *a_mrc, uint64_t pc, uint64_t addr);
void solve_mrc(mrc_t *a_mrc);
void delete_mrc(mrc_t *a_mrc);

void output_rtd(mrc_t *a_mrc, int fd);
void output_ht(mrc_t *a_mrc, int fd);


