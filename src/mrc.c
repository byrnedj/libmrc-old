
#include "../include/mrc.h"
#include "../include/uthash.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* Helper methods for AET 
 * - domain_value_to_index - takes a reuse time and 
 *   applies the histogram compression done by Xiang et al. in
 *   Linear Time Modeling of WSS paper (PACT'11)
 * - domain_index_value_to_domain - is the inverse of value_to_index
 *
 * - add_ref - adds a ref_t (reference,time) to a hash table
 * - find_ref - returns the pointer to ref_t with the given reference
 *   else, NULL
 * - delete_ref - deletes a ref_t from the hash table
 * - delete_all - deletes entire table
 *
 * */

void delete_hash_all(mrc_t* a_mrc) {
  ref_t *current_ref, *tmp;

  HASH_ITER(hh, a_mrc->hash, current_ref, tmp) {
    HASH_DEL(a_mrc->hash,current_ref);  
    free(current_ref);            
  }
}

void delete_hash_ref(mrc_t *a_mrc, ref_t* a_ref) {
    HASH_DEL(a_mrc->hash, a_ref);  
    free(a_ref); 
}

ref_t *find_ref(mrc_t *a_mrc, uint64_t a_reference) {
    ref_t *s;

    HASH_FIND_UINT64( a_mrc->hash, &a_reference, s );  
    return s;
}

void add_ref(mrc_t *a_mrc, uint64_t a_reference, uint64_t a_time) 
{
    ref_t *s  = (ref_t*)malloc(sizeof(ref_t));
    s->reference = a_reference;
    s->time = a_time; 
    HASH_ADD_UINT64( a_mrc->hash, reference, s );
    
}

uint64_t domain_value_to_index(uint64_t value)
{
    uint64_t loc = 0, step = 1;
    int index = 0;
    while (loc+(step*domain) < value) 
    {
        loc += step*domain;
        step *= 2;
        index += domain;
    }
    while (loc < value) 
    {
        index++;
        loc += step;
    }
    return index;
}

uint64_t domain_index_to_value(uint64_t index)
{
    uint64_t value = 0, step = 1;
    while (index > domain) 
    {
        value += step*domain;
        step *= 2;
        index -= domain;
    }
    while (index > 0) 
    {
        value += step;
        index--;
    }
    return value;
}

/* Init AET structures */


mrc_t* init_mrc(uint64_t a_sample_rate, 
                uint64_t a_mrc_interval)
{
    mrc_t* a_mrc = (mrc_t*)malloc(sizeof(mrc_t));
    a_mrc->sample_rate = a_sample_rate;
    a_mrc->mrc_interval = a_mrc_interval;

    a_mrc->wss = 0;

    a_mrc->n = 0;
    a_mrc->m = 0;
    a_mrc->node_cnt = 0;
    a_mrc->node_max = 0;
    a_mrc->tott = 0;

    srand(128);
    a_mrc->loc = rand()%(a_sample_rate*2)+1;
    
    a_mrc->rtd = (uint64_t*)calloc(MAXH,sizeof(uint64_t));
    a_mrc->hash = NULL;

    return a_mrc;
}

void delete_mrc(mrc_t* a_mrc)
{
    delete_hash_all(a_mrc);
    free(a_mrc->rtd);
    free(a_mrc->mrc);
    free(a_mrc);
}

void take_sample(mrc_t *a_mrc, uint64_t kid)
{

    a_mrc->n++;
    ref_t *it = find_ref(a_mrc,kid);
    //if found get the time
    if (it != NULL)
    {

        uint64_t t = it->time;
        a_mrc->rtd[domain_value_to_index(a_mrc->n-t)]++;
        a_mrc->tott++;
        //remove it from table
        delete_hash_ref(a_mrc,it);
        a_mrc->node_cnt--;
    }
    //check if we should add this sample to hash table
    if (a_mrc->n == a_mrc->loc)
    {
        add_ref(a_mrc,kid,a_mrc->n);
        
        //int num_refs = HASH_COUNT(a_mrc->hash);
        //printf("there are %d refs\n", num_refs);
        
        a_mrc->loc += rand()%(a_mrc->sample_rate*2)+1;
        a_mrc->node_cnt++;
    }
    if (a_mrc->node_cnt > a_mrc->node_max)
        a_mrc->node_max = a_mrc->node_cnt;
    
}


void solve_mrc(mrc_t *a_mrc)
{

    //get number of cold misses
    a_mrc->m = a_mrc->node_cnt*a_mrc->sample_rate;


    double sum = 0; 
    uint64_t T = 0;
    double tot = 0;

    //tott is total references in HT
    //m is number of cold misses
    //N should be total references including cold misses
    double N = a_mrc->tott+1.0*a_mrc->tott/
        (a_mrc->n-a_mrc->m)*a_mrc->m;

    //sanity check to make sure we actually have refs
    if (N < 1) return;


    uint64_t step = 1; 
    int dom = 0;
    int dT = 0; 
    uint64_t loc = 0;
    
    uint64_t c = 1;
    double cmiss_rate = 0;

    a_mrc->mrc_size = (a_mrc->m / a_mrc->mrc_interval) + 1;
    a_mrc->mrc = (double*)calloc(a_mrc->mrc_size,sizeof(double));
    
    while (c <= a_mrc->m)
    {
        while (T <= a_mrc->n && tot/N < c) 
        {
            tot += N-sum;
            T++;
            if (T > loc) 
            {
                if (++dom > domain) 
                {
                    dom = 1;
                    step *= 2;
                }
                loc += step;
                dT++;
            }
            sum += 1.0*a_mrc->rtd[dT]/step;
        }
        //how to get AET(c), we know mr(c) = P(AET(c))
        //so (N-sum)/N must be P(AET(c))
        //T == AET(c)
        if (c % a_mrc->mrc_interval == 0)
        {
            cmiss_rate = (N-sum)/N;
            
            a_mrc->mrc[c/a_mrc->mrc_interval] = cmiss_rate;

        }
        c++;
    }
    a_mrc->wss = c;
}
