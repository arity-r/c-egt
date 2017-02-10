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
simulations = 1

def run_experiment(args):
    global Gsum, Gnum
    (n, N), (benefit, init_c, seed) = args
    result_raw = subprocess.check_output(
        './agos-common.out test.graph %f 10.0 100000 %d 0 %d'\
        %(benefit, init_c, seed), shell=True)
    result = unpack('='+'i'*(population+1)+'d'*(population+1), result_raw)
    gosnum_part = result[:population+1]
    gossum_part = result[population+1:]
    for i, x in enumerate(gosnum_part):
        Gnum[benefit][i] += x
    for i, x in enumerate(gossum_part):
        Gsum[benefit][i] += x
    with lock:
        print('%5d/%5d simulation finished'%(n, N), file=stderr)

if __name__ == '__main__':
    #benefits = [1.005, 1.015]
    benefits = [1.005]
    init_cs = list(range(0, population+1))
    seeds = [randint(0, 2**32-1) for _ in range(simulations)]
    args_list = [(benefit, init_c, randint(0, 2**32-1))
                 for benefit in benefits
                 for init_c in init_cs
                 for seed in seeds]
    N = len(args_list)
    args_list = zip([(n, N) for n in range(1, N+1)], args_list)

    global Gsum, Gnum
    Gsum = {}
    Gnum = {}
    for benefit in benefits:
        Gsum[benefit] = multiprocessing.Array(c_double, population+1)
        Gnum[benefit] = multiprocessing.Array(c_int, population+1)

    pool = multiprocessing.Pool()
    pool.map(run_experiment, args_list)
    pool.close()

    print(','.join(['numB=%1.3f,sumB=%1.3f'\
                    %(benefit, benefit)
                    for benefit in benefits]))
    for i in range(population+1):
        print(','.join(['%d,%f'%(Gnum[benefit][i], Gsum[benefit][i])
                        for benefit in benefits]))
