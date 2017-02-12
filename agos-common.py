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
resultdir = 'result'
current_topology = ''

def set_graph(topology):
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

    global current_topology
    current_topology = topology
    global graphcache, population
    graphcache = subprocess.check_output(cmd)
    (population,) = unpack('i', graphcache[:4])

def result_path(**kwargs):
    return os.path.join(
        resultdir,
        ';'.join(map(lambda kv: '='.join(map(str, kv)),
                     sorted(kwargs.items(), key=lambda t: t[0])))\
        + '.' + strftime('%Y%m%d%H%M%S') + '.csv')

def run_experiment_part(args):
    global graphcache, population
    global Gsum, Gnum
    global total_sims, current_sim
    benefit, beta, mutation_rate, init_c, seed = args
    cmd = list(map(str, ['./agos-common.out',
                         '-B', benefit, '-b', beta, '-m', mutation_rate,
                         '-t', '100000', '-c', init_c, '-s', seed]))
    p = subprocess.Popen(cmd,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE)
    result_raw = p.communicate(input=graphcache)[0]
    result = unpack('='+'i'*(population+1)+'d'*(population+1), result_raw)
    Gnum_part = result[:population+1]
    Gsum_part = result[population+1:]
    for i, x in enumerate(Gnum_part):
        Gnum[i] += x
    for i, x in enumerate(Gsum_part):
        Gsum[i] += x

    current_sim.value += 1
    with lock:
        print('%5d/%5d simulation finished @ %s (params = %s)'\
              %(current_sim.value, total_sims.value,
                current_topology, str(args[:-1])))

def run_experiment(benefit, beta, mutation_rate):
    if(not current_topology):
        return
    # building parameters
    global population
    global total_sims, current_sim
    init_cs = list(range(0, population+1, 100))
    args_list = [(benefit, beta, mutation_rate,
                  init_c, randint(0, 2**32-1))
                 for init_c in init_cs]
    total_sims = multiprocessing.Value('i', len(args_list))
    current_sim = multiprocessing.Value('i', 0)

    # make global array for result
    global Gsum, Gnum
    Gsum = multiprocessing.Array(c_double, population+1)
    Gnum = multiprocessing.Array(c_int, population+1)

    pool = multiprocessing.Pool()
    map(run_experiment_part, args_list)
    pool.close()

    j = linspace(0, 1, population+1)
    outfilename = result_path(topology=current_topology, benefit=benefit,
                              beta=beta, mutation=mutation_rate)
    try: os.makedirs(resultdir)
    except: pass
    with open(outfilename, 'w') as f:
        f.write('\n'.join(
            map(lambda t: ','.join(map(str, t)), zip(j, Gnum, Gsum))))

if __name__ == '__main__':
    # topology, benefit, beta, mutation_rate
    set_graph('rrg')
    for benefit in [1.005, 1.015]:
        run_experiment(benefit, 10, 1e-3)
    exit(0)
    set_graph('sl')
    for benefit in [1.005, 1.015]:
        run_experiment(benefit, 10, 1e-3)
    set_graph('ba')
    for benefit in [1.15, 1.25, 1.35]:
        run_experiment(benefit, 0.1, 1e-3)

    # topology, benefit, beta, mutation_rate
    set_graph('rrg')
    for _ in range(2):
        for mutation in [1e-5, 1e-4, 1e-3, 1e-2]:
            if(_ == 1 and mutation == 1e-3): continue
            run_experiment(1.005, 10, mutation)
    set_graph('sl')
    for _ in range(2):
        for mutation in [1e-5, 1e-4, 1e-3, 1e-2]:
            if(_ == 1 and mutation == 1e-3): continue
            run_experiment(1.005, 10, mutation)
    set_graph('ba')
    for _ in range(2):
        for mutation in [1e-5, 1e-4, 1e-3, 1e-2]:
            if(_ == 1 and mutation == 1e-3): continue
            run_experiment(1.25, 0.1, mutation)

