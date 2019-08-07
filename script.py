import os
import random
import re
import argparse
import subprocess

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'
BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/input.txt'

BENCH_BINARY = {
    'matrix_mul' : os.path.abspath(BENCH_BIN_HOME + '/matrix_multiplication/matrix_mul')
}

class ExperimentManager:
    ##
    #  example gem5 run:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    ##
    GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/x86/gem5.opt')
    GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/one_level/run.py')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('bench_name', help='Benchmark\'s name', default=BENCH_BIN_HOME + BENCH_BINARY['matrix_mul'])
    parser.add_argument('-g', '--golden', action='store_true', help='No fault injection to get golden output')
    parser.add_argument('-f', '--flag', action='store', nargs='*', help='All gem5 debug flags')