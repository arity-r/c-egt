#!/usr/bin/env python
from __future__ import print_function, division
from sys import stdout, stderr
from random import randint
from numpy import linspace
from struct import unpack
from ctypes import c_double, c_int
import subprocess, multiprocessing
lock = multiprocessing.Lock()

# fixed parameters
population = 1000
generations = 25
simulations = 10

def run_experiment(args):
    global Gsum, Gnum
    (n, N), (beta, init_c, seed) = args
    result_raw = subprocess.check_output(
        './agos-time-homogeneous-random.out %f %d %d'\
        %(beta, init_c, seed), shell=True)
    result = unpack(
        '='+('i'*(population+1)+'d'*(population+1))*generations,
        result_raw)
    for generation in range(generations):
        gosnum_part = result[
            ((2*generation)*(population+1)):((2*generation+1)*(population+1))]
        gossum_part = result[
            ((2*generation+1)*(population+1)):((2*generation+2)*(population+1))]
        for i, x in enumerate(gosnum_part):
            Gnum[beta][generation][i] += x
        for i, x in enumerate(gossum_part):
            Gsum[beta][generation][i] += x
    with lock:
        print('%5d/%5d simulation finished'%(n, N), file=stderr)

if __name__ == '__main__':
    betas = [1, 10]
    init_cs = list(range(1, population))
    seeds = [randint(0, 2**32-1) for _ in range(simulations)]
    args_list = [(beta, init_c, seed)
                 for beta in betas
                 for init_c in init_cs
                 for seed in seeds]
    N = len(args_list)
    args_list = zip([(n, N) for n in range(1, N+1)], args_list)

    global Gsum, Gnum
    Gsum = {}
    Gnum = {}
    for beta in betas:
        Gsum[beta] = [multiprocessing.Array(c_double, population+1)
                      for _ in range(generations)]
        Gnum[beta] = [multiprocessing.Array(c_int, population+1)
                      for _ in range(generations)]

    pool = multiprocessing.Pool()
    pool.map(run_experiment, args_list)
    pool.close()

    print(','.join(['num.beta=%02.f.gen=%d,sum.beta=%02.f.gen=%d'\
                    %(beta, gen, beta, gen)
                    for beta in betas
                    for gen in range(1, generations+1)]))
    for i in range(population+1):
        print(','.join(['%d,%f'%(Gnum[beta][gen][i], Gsum[beta][gen][i])
                        for beta in betas
                        for gen in range(generations)]))
