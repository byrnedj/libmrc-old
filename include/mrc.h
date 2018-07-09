
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
    UT_hash_handle hh; //make it hashable for UT HASH

} ref_t;

typedef struct mrc_t 
{
    uint64_t sample_rate;
    uint64_t mrc_interval;
    uint64_t mrc_size; //length of mrc

    double *mrc; //the actual mrc
    uint64_t wss; //working set size

    //AET specific parameters
    uint64_t n; //total number of references
    uint64_t m; //number of cold misses
    uint64_t node_cnt;
    uint64_t node_max;
    uint64_t loc; //sample location
    uint64_t tott; //total in hash table

    uint64_t *rtd; //reuse time dist
    ref_t *hash; //last use of a reference (hash table)

    //Future work: implement threaded sampling
    //pthread_t* sampler;

    //pthread_mutex_t s_mutex;
    //pthread_cond_t s_condc, s_condp;
    //uint64_t reference;

} mrc_t;


mrc_t* init_mrc(uint64_t a_sample_rate, 
                uint64_t a_mrc_interval); 

void take_sample(mrc_t *a_mrc, uint64_t a_reference);
void solve_mrc(mrc_t *a_mrc);
void delete_mrc(mrc_t *a_mrc);

//void take_sample_thread(mrc_t *a_mrc, uint64_t a_reference);
//void thread_sample(mrc_t *a_mrc);


