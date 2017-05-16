#!/usr/bin/env python
from __future__ import print_function, division
from sys import stdout, stderr
from random import randint, choice
from numpy import linspace
from struct import unpack
from ctypes import c_double, c_int
from time import strftime
import io, os, re, sys
import subprocess, multiprocessing

lock = multiprocessing.Lock()
result_file = 'time-evolution.csv'

def make_graph(topology):
    cmdopts = []
    if(topology == 'rrg'):
        cmdopts = ['-t', 'rr', '-k', '4', '-f', '0.9']
    if(topology == 'sl'):
        cmdopts = ['-t', 'sl']
    if(topology == 'ba'):
        cmdopts = ['-t', 'ba', '-k', '4']
    if(not cmdopts):
        print('topology %s not recoganized'%topology, file=stderr)
        exit(-1)
    cmd = ['./graphgen.out', '-N', '1000',
           '-s', str(randint(0, 2**32-1))] + cmdopts

    graphcache = subprocess.check_output(cmd)
    (population,) = unpack('i', graphcache[:4])
    return topology, graphcache, population

def run_experiment_part(args):
    graph, benefit, beta, mutation_rate, payoff, init_cfrac, sim = args
    topology, graphcache, population = graph
    init_c = int(init_cfrac * population)
    seed = randint(0, 2**32-1)

    exec_file = 'time-evolution-acc.out'
    if payoff == 'avr':
        exec_file = 'time-evolution-avr.out'

    cmd = list(map(str, ['./%s'%exec_file,
                         '-B', benefit, '-b', beta, '-m', mutation_rate,
                         '-t', '1000000', '-c', init_c, '-s', seed]))
    p = subprocess.Popen(cmd,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE)
    result_raw = p.communicate(input=graphcache)[0]
    result = unpack('='+'d'*((1000000-1)//population+1), result_raw)

    with lock:
        with open(result_file, 'a') as f:
            for t, c in enumerate(result):
                row = (topology, payoff, benefit, beta, mutation_rate,
                       init_cfrac, sim, t*population, c)
                f.write(','.join(map(str, row))+'\n')

if __name__ == '__main__':
    with open(result_file, 'w') as f:
        f.write('topology,payoff,benefit,beta,mutation,init_c,sim,t,c\n')
    nsim = 1
    benefit_of = lambda t: 1.25 if t == 'ba' else 1.005
    beta_of = lambda t: 0.1 if t == 'ba' else 10
    topology = ['rrg', 'sl', 'ba']
    args_list = [(n[0], n[1], n[2], m, p, f, s)
                 for n in [(make_graph(t), benefit_of(t), beta_of(t))
                           for t in topology]
                 for m in (0, 0.01)
                 for p in ('acc', 'avr')
                 for f in (0.2, 0.8)
                 for s in range(1, nsim+1)]
    pool = multiprocessing.Pool()
    pool.map(run_experiment_part, args_list)
    pool.close()
