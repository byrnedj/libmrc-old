
#!/usr/bin/python

#kv workload generator, this generates
#an access pattern based with several distribution
#options
#1. Zipfian (a=1)
#3. Latest (a=10)
#4. Uniform (1/N)

import numpy as np
import sys


def zipfian(k,s,N):
    rank = np.arange(1,N+1)
    n = 1.0 / np.power(rank,s)
    zeta = np.sum(n)
    return (1.0/np.power(k,s))/zeta
    





# Trace name, alpha, size
# DEC 0.77 3.5m
# UPisa 0.78 1.9m
# FuNet 0.83 2.8m
# UCB 0.69 2.9m
# Questnet 0.73 1.8m
# NLANR 0.64 4.8m
#params = [ ('dec',0.77, 3500000), ('upisa',0.78, 1900000), ('funet',0.83,2800000), 
#           ('ucb',0.69, 2900000), ('quest',0.73, 1800000), ('nlanr',0.64,4800000) ]


# Test trace is alpha = 1, 1M reqs, N = 100K
params = [ ('ycsb',1,100000) ]

#N is number of unique keys
appid = 1
c = 10000000
for p in params:

    N = p[2]
    print p[0]
    nreqs = 1000000
    ws = np.arange(1,N+1)
    dist = zipfian(ws,p[1],N)
    
    keys = np.random.choice(ws,nreqs,replace=True,p=dist)
    keys = keys + appid*c
    
    app = np.full(nreqs,appid)
    get = np.full(nreqs,1)
    ksize = np.full(nreqs,8)
    vsize = np.full(nreqs,200)
    out = np.stack((app,get,ksize,vsize,keys),axis=1)
    #np.savetxt(p[0],out,'%d,%d,%d,%d,%d')
    np.savetxt(p[0],keys,'%d')
    appid = appid + 1


#workloads = ['zf','lat','uni']

#for w in workloads:
#    low = 0
#    high = 5000000
#    if w == 'uni':
#        keys = np.random.uniform(low,high,N)
#    elif w == 'zf':
#        keys = np.zipf(1,N)
#    elif w == 'lat':
#        keys = np.zif(10,N)


