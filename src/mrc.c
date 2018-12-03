
#include "mrc.h"
#include "uthash.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

/* Helper methods for AET 
 * - domain_value_to_index - takes a reuse time and 
 *   applies the histogram compression done by Xiang et al. in
 *   Linear Time Modeling of WSS paper (PACT'11)
 * - domain_index_value_to_domain - is the inverse of value_to_index
 *
 * - add_ref - adds a ref_t (reference,time,prefetch) to a hash table
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

ref_t *find_pf(mrc_t *a_mrc, uint64_t a_reference) {
    ref_t *s;

    HASH_FIND_UINT64( a_mrc->hash_pf, &a_reference, s );  
    return s;
}

ru_t *find_time(mrc_t *a_mrc, uint64_t a_time) {
    ru_t *t;

    HASH_FIND_UINT64( a_mrc->reuse_times, &a_time, t );  
    return t;
}

rpt_t *find_stride(mrc_t *a_mrc, uint64_t a_pc) {
    rpt_t *rpt_entry;

    HASH_FIND_UINT64( a_mrc->rpt, &a_pc, rpt_entry );  
    return rpt_entry;
}

void add_ref(mrc_t *a_mrc, uint64_t a_reference, uint64_t a_time, uint8_t a_pf) 
{
    ref_t *s  = (ref_t*)malloc(sizeof(ref_t));
    s->reference = a_reference;
    s->time = a_time;
    s->pf = a_pf;	

    /*
    //if this referenced was prefetched at any point
    //then it should not count towards cold miss
    //state == 2 since, we have seen this reference
    //at least 1 time in the access pattern
    ref_t *ref = find_ref(a_mrc,a_reference);
    if (pf != NULL)
    {
        ref->time = a_mrc->n;
    }

    //pf is null (not previsouly pf'd) and its not to be prefetched
    if (pf == NULL && a_pf != 1)
    {
        HASH_ADD_UINT64( a_mrc->hash, reference, s );
        a_mrc->node_cnt++;
    }
    else if (pf == NULL && a_pf == 1)
    {
        HASH_ADD_UINT64( a_mrc->hash, reference, s );
        a_mrc->pfs++;

    }

    
    // if this prefetch addr is in the hash table already
    // don't bother adding it
    //pf = find_ref(a_mrc,a_reference);
    //if (pf == NULL && a_pf == 1)
    //{
    //    HASH_ADD_UINT64( a_mrc->hash, reference, s );
    //    a_mrc->pfs++;
    //}
    //else if (pf == NULL && a_pf == 0)
    //{
    //    HASH_ADD_UINT64( a_mrc->hash, reference, s );
    //}
    */
    if (a_pf == 1)
    {
        HASH_ADD_UINT64( a_mrc->hash, reference, s );
        a_mrc->pfs++;
    }
    else
    {
        HASH_ADD_UINT64( a_mrc->hash, reference, s );
        a_mrc->node_cnt++;
    }
    
}

void add_pf(mrc_t *a_mrc, uint64_t a_reference) 
{
    ref_t *s  = (ref_t*)malloc(sizeof(ref_t));
    s->reference = a_reference;
    s->time = 1;
    s->pf = 1;	


    HASH_ADD_UINT64( a_mrc->hash_pf, reference, s );
    
}

void add_time(mrc_t *a_mrc, uint64_t a_time) 
{
    ru_t *t  = (ru_t*)malloc(sizeof(ru_t));
    t->time = a_time;
    t->count = 1;
    HASH_ADD_UINT64( a_mrc->reuse_times, time, t );
    //record the max for outputing reuse times
    if (a_time > a_mrc->max_rt)
	a_mrc->max_rt = a_time;
    
}

void add_stride(mrc_t *a_mrc, uint64_t a_pc, uint64_t a_addr) 
{
    rpt_t *rpt_entry = (rpt_t*)malloc(sizeof(rpt_t));
    rpt_entry->state = 1;
    rpt_entry->prev_addr = a_addr;
    rpt_entry->stride = 0;	
    rpt_entry->pc = a_pc;
	
    HASH_ADD_UINT64( a_mrc->rpt, pc, rpt_entry );
}

void output_rtd(mrc_t *a_mrc, int fd)
{
	char buf[256];
	sprintf(buf,"rt,count\n");
	write(fd,buf,strlen(buf));
	for (uint64_t i = 1; i < a_mrc->max_rt+1; i++)
	{
		ru_t *t = find_time(a_mrc,i);
		if (t != NULL)
		{
			sprintf(buf,"%lu,%lu\n",i,t->count);
			write(fd,buf,strlen(buf));
		}
	}
}

void output_ht(mrc_t *a_mrc, int fd)
{
	char buf[256];
	sprintf(buf,"addr,time\n");
	write(fd,buf,strlen(buf));

	ref_t *ref;
    	for(ref = a_mrc->hash; ref != NULL; ref = ref->hh.next) 
	{
	    sprintf(buf,"%lu,%lu\n",ref->reference,ref->time);
	    write(fd,buf,strlen(buf));
	}
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
	a_mrc->max_rt = 0;

    a_mrc->n = 0;
    a_mrc->m = 0;
    a_mrc->node_cnt = 0;
    a_mrc->node_max = 0;
    a_mrc->tott = 0;
    a_mrc->pfs = 0;
    a_mrc->cm = 0;
    a_mrc->last_cm = 0;

    srand(128);
    a_mrc->loc = rand()%(a_sample_rate*2-1)+1;
    
    a_mrc->rtd = (uint64_t*)calloc(MAXH,sizeof(uint64_t));
    a_mrc->hash = NULL;
    a_mrc->rpt = NULL;
    a_mrc->hash_pf = NULL;

    a_mrc->reuse_times = NULL;

    return a_mrc;
}

void delete_mrc(mrc_t* a_mrc)
{
    delete_hash_all(a_mrc);
    free(a_mrc->rtd);
    free(a_mrc->mrc);
    free(a_mrc);
}


int take_rtime(mrc_t *a_mrc, uint64_t kid)
{

    ref_t *it = find_ref(a_mrc,kid);
    int added = 1;
    if (it == NULL)
        added = 0;
    //if found get the time
    //if ((it != NULL && it->pf == 0) || (it != NULL && it->pf == 2))
    if (it != NULL)
    {

        uint64_t t = it->time;
        a_mrc->rtd[domain_value_to_index(a_mrc->n-t)]++;
	ru_t *entry = find_time(a_mrc,a_mrc->n-t);

        if (entry != NULL)
	{
	    entry->count++;
	}
	else
	{
	    add_time(a_mrc,a_mrc->n-t);
	}

        a_mrc->tott++;

        if (it->pf == 0)
            a_mrc->node_cnt--;


        //remove it from table
        delete_hash_ref(a_mrc,it);

    }
    else
    {
        a_mrc->cm++;
    }
    return added;
}

void record_ref(mrc_t *a_mrc, uint64_t kid, int64_t stride, int multi, int added)
{
    
    add_ref(a_mrc,kid,a_mrc->n,0);

    if (multi)
    {
        add_ref(a_mrc,kid+1,a_mrc->n,0);
        if (added == 0)
        {
            add_ref(a_mrc,(kid+1)+stride,a_mrc->n,1);
            
            //add_pf(a_mrc,(kid+1)+stride);
        }
    }
    else
    {
        //if the kid was not found in hash table (so no reuse time
        //was added to the rtd), then it was a *miss* so the next
        //address (kid+stride) should be added as a prefetched instruction
        if (added == 0)
        {
            add_ref(a_mrc,kid+stride,a_mrc->n,1);
            //add_pf(a_mrc,(kid)+stride);
        }
    }
        
}


void take_sample(mrc_t *a_mrc, uint64_t kid, int64_t stride, int multi)
{
    a_mrc->n++;

    take_rtime(a_mrc,kid);
    if (multi)
        take_rtime(a_mrc,kid+1);

    if (a_mrc->n == a_mrc->loc)
    {
        //in no prefetch mode, we always assume that the addr was
        //added (if it was not added then it would imply miss and go
        //prefetch). 
        //so added always == 1
	record_ref(a_mrc,kid,stride,multi,1);	
        a_mrc->loc += rand()%(a_mrc->sample_rate*2-1)+1;
        
    }
    if (a_mrc->node_cnt > a_mrc->node_max)
        a_mrc->node_max = a_mrc->node_cnt;

}
void take_sample_pf(mrc_t *a_mrc, uint64_t kid, int64_t stride, int multi)
{

    a_mrc->n++;

    int added = take_rtime(a_mrc,kid);
    if (multi)
        take_rtime(a_mrc,kid+1);

    if (a_mrc->n == a_mrc->loc)
    {
	record_ref(a_mrc,kid,stride,multi,added);	
        a_mrc->loc += rand()%(a_mrc->sample_rate*2-1)+1;
        
    }
    if (a_mrc->node_cnt > a_mrc->node_max)
        a_mrc->node_max = a_mrc->node_cnt;
    
}

int64_t get_stride(mrc_t *a_mrc, uint64_t pc, uint64_t addr)
{
        int64_t pred_stride = LONG_MAX;	
	rpt_t *rpt_entry = find_stride(a_mrc, pc); // will implement looking up the reference
	//if (rpt_entry == NULL || rpt_entry->state == 0) // no prefetch
	//{
	//	take_sample(a_mrc, addr, 0, 0); //not checking if multi
	//}
	//else
	//{
	//	take_sample_pf(a_mrc, addr, rpt_entry->prev_addr + rpt_entry->stride, 0); // not doing multi
	//}
	if (rpt_entry != NULL) // then it was found
	{
                pred_stride = rpt_entry->stride;
		if (addr == rpt_entry->prev_addr + rpt_entry->stride) // if prediction correct
		{
			int curr_state = rpt_entry->state;
			if (curr_state == 1 || curr_state == 2 || curr_state == 3) 
			{
				rpt_entry->prev_addr = addr;
				rpt_entry->state = 3; // set state to steady
			}
			else
			{	
				rpt_entry->prev_addr = addr;
				rpt_entry->state = 2; // transient state
			}
		}
		else // when incorrect prediction
		{
			if (rpt_entry->state == 1) // if state is in intitial
			{
				rpt_entry->stride = addr - rpt_entry->prev_addr;  
				rpt_entry->prev_addr = addr;
				rpt_entry->state = 2; // transient state
			}
			else if (rpt_entry->state == 3) // staedy state 
			{
				rpt_entry->prev_addr = addr;
				rpt_entry->state = 1; // initial state
			}
			else if (rpt_entry->state == 2)
			{
				rpt_entry->stride = addr - rpt_entry->prev_addr;
				rpt_entry->prev_addr = addr;
				rpt_entry->state = 0; // state = no prediction
			}
			else // state no prediction
			{
				rpt_entry->stride = addr - rpt_entry->prev_addr;
				rpt_entry->prev_addr = addr;
			}
		}

	}
	else // not found
	{
		add_stride(a_mrc, pc, addr); 
	}

        return pred_stride;
}


void solve_mrc(mrc_t *a_mrc)
{

    //get number of cold misses
    //a_mrc->m = a_mrc->node_cnt*a_mrc->sample_rate;

    a_mrc->m = a_mrc->cm/a_mrc->sample_rate;


    double tot = 0;
    double sum = 0;
    uint64_t T = 0;

    //tott is total references in HT
    //m is number of cold misses
    //N should be total references including cold misses
    double N = a_mrc->tott+1.0*a_mrc->tott/(a_mrc->n-(a_mrc->m))*(a_mrc->m);
    //N = N + 1.0*2/(4.0-2.0)*2;
	
    a_mrc->N = N;
    //sanity check to make sure we actually have refs
    if (N < 1) return;


    uint64_t step = 1; 
    int dom = 0;
    int dT = 0; 
    uint64_t loc = 0;
    
    uint64_t c = 1;
    double cmiss_rate = 0;

    a_mrc->mrc_size = ((a_mrc->N + a_mrc->mrc_interval)/
						(a_mrc->mrc_interval));
    a_mrc->mrc = (double*)calloc(a_mrc->mrc_size,sizeof(double));
    uint64_t mrc_i = 1;
    
    while (c <= N)
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
            //double nprime = N - (( ( cmiss_rate * (a_mrc->n/a_mrc->sample_rate) - (a_mrc->cm/a_mrc->sample_rate) ) ) );

            //added in cold misses
            //double nprime = a_mrc->tott + (1.0*a_mrc->tott/(a_mrc->n-(a_mrc->m))*(a_mrc->m));
            ////find number of pfs at this size
            //double t = cmiss_rate * (a_mrc->n/a_mrc->sample_rate);
            //nprime = nprime -   t/(a_mrc->n-(a_mrc->m)*a_mrc->m);
            //double newmr = (nprime-sum)/nprime;
            //printf("c %lu, mr %.3f, N %.3f, nprime %.3f, newmr %.3f\n",c,cmiss_rate,N,nprime,newmr);
            a_mrc->mrc[mrc_i] = cmiss_rate;
			mrc_i += 1;

        }
        c++;
    }
    a_mrc->wss = (a_mrc->mrc_size-a_mrc->mrc_interval)*a_mrc->mrc_interval;
}
