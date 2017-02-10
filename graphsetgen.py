#!/usr/bin/env python
from __future__ import print_function, division
import os
from random import randint
from subprocess import call

N = 1000
k = 4

"""
for random regular,
  './graphgen.out -t rr -N 1000 -k 4 -f 0.9 -o <filename> -s <seed>'
for barabasi albert,
  './graphgen.out -t ba -N 1000 -k 4 -o <filename> -s <seed>
for square lattice (only one graph),
  './graphgen.out -t sl -N 1000 -k 4 -o <filename> -s <seed>

<filename> ::= topology=(rr|ba|sl).<seed>.graph
"""

graphsetdir = 'graphset'

def generate_rrg_set(n=10):
    seeds = [randint(0, 2**31-1) for _ in range(n)]
    filenames = ['topology.rrg.%08X.graph'%seed for seed in seeds]
    filenames = [os.path.join(graphsetdir, filename)
                 for filename in filenames]
    for seed, filename in zip(seeds, filenames):
        call('./graphgen.out -t rr -N 1000 -k 4 -f 0.9 -o %s -s %d'\
             %(filename, seed), shell=True)

def generate_ba_set(n=10):
    seeds = [randint(0, 2**31-1) for _ in range(n)]
    filenames = ['topology.ba.%08X.graph'%seed for seed in seeds]
    filenames = [os.path.join(graphsetdir, filename)
                 for filename in filenames]
    for seed, filename in zip(seeds, filenames):
        call('./graphgen.out -t ba -N 1000 -k 4 -o %s -s %d'\
             %(filename, seed), shell=True)

def generate_lattice():
    seed = 0
    filename = os.path.join(graphsetdir, 'topology.sl.00000000.graph')
    call('./graphgen.out -t sl -N 1000 -k 4 -o %s -s %d'%(filename, seed),
         shell=True)

if __name__ == '__main__':
    try: os.makedirs(graphsetdir)
    except: pass
    generate_rrg_set()
    generate_ba_set()
    generate_lattice()
