import os
import glob
import concurrent.futures 
from functools import partial
import random
import re
import argparse
import subprocess

voltages = ["0.54V", "0.55V", "0.56V", "0.57V", "0.58V", "0.59V", "0.60V"]

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'
BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/'

BENCH_BINARY = {
    'matrix_mul' : os.path.abspath(BENCH_BIN_HOME + '/matrix_multiplication/matrix_mul')
}

GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/X86/gem5.opt')
GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/one_level/run.py')

class ExperimentManager:
    ##
    #  example gem5 run:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    ##
    def __init__(self, bench_name, flags, input_name, voltage):
        self.bench_name = bench_name
        self.flags = flags
        self.input_name = input_name
        self.voltage = voltage

    @staticmethod
    def run_golden(bench_name, flags):
        redirection = '-re'
        outdir = '--outdir=' + bench_name + '_results/golden'
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'
        debug_flags = ''

        if flags and len(flags) > 0:
            all_flags = ','.join(flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file, debug_flags])

        bench_binary_path = '-c ' + BENCH_BINARY[bench_name]

        input_path = '-i ' + BENCH_INPUT_HOME + "golden.txt"

        gem5_script_option = ' '.join([bench_binary_path, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        subprocess.call(gem5_command, shell=True)

    def is_crash(self):
        grep_crash = 'cd ' + WHERE_AM_I + '/' + self.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name + ';grep "exiting with last active thread context" output.txt'
        result = subprocess.Popen(grep_crash, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
        decoded_result = result.decode('utf-8')

        if decoded_result and len(decoded_result) > 0:
            return False
        else:
            return True
    
    def is_correct(self, correct_result):
        grep_result = 'cd ' + WHERE_AM_I + '/' + self.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name + ';grep -oh "^[0-9].*" output.txt'
        sim_result = subprocess.Popen(grep_result, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]

        lines = sim_result.decode('utf-8').splitlines()

        result = []

        for i in range(len(lines)):
            result.extend(lines[i].strip().split(" "))

        if correct_result == result:
            return True
        else:
            return False

    def get_correct_result(self):
        grep_correct_result = 'cd ' + WHERE_AM_I + '/' + self.bench_name + '_results/golden;grep -oh "^[0-9].*" output.txt'
        correct_result = subprocess.Popen(grep_correct_result, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]

        lines = correct_result.decode('utf-8').splitlines()

        result = []

        for i in range(len(lines)):
            result.extend(lines[i].strip().split(" "))

        return result
        

    def inject(self):
        redirection = '-re'
        outdir = '--outdir=' + self.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'
        debug_flags = ''

        if self.flags and len(self.flags) > 0:
            all_flags = ','.join(self.flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file])

        bench_binary_path = '-c ' + BENCH_BINARY[self.bench_name]

        input_path = '-i ' + BENCH_INPUT_HOME + voltage + "/" + self.input_name

        gem5_script_option = ' '.join([bench_binary_path, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        try:
            subprocess.call(gem5_command, shell=True, timeout=30)
        except subprocess.TimeoutExpired:
            return "Crash"

        if self.is_crash():
            return "Crash"

        correct_result = self.get_correct_result()

        if(self.is_correct(correct_result)):
            return "Correct"
        else:
            return "Incorrect"

def run_experiment(input_path, bench_name, flags, voltage):
    input_name = input_path.split("/")[-1]
    experiment_manager = ExperimentManager(bench_name, flags, input_name, voltage)

    result = experiment_manager.inject()

    with open(WHERE_AM_I + "/" + bench_name + "_results" + "/" + voltage + "_results.txt", "a") as result_file:
            line = ",".join([input_name[:-4], result + "\n"])
            result_file.write(line)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-c','--bench-name', help='Benchmark\'s name', default='matrix_mul')
    parser.add_argument('-f', '--flags', action='store', nargs='*', help='All gem5 debug flags', default=["Cache"])

    args = parser.parse_args()
    
    open(BENCH_INPUT_HOME + "golden.txt","w").close() # Empty file for golden run

    ExperimentManager.run_golden(args.bench_name, args.flags)

    for voltage in voltages:
        with concurrent.futures.ProcessPoolExecutor() as executor:
            input_paths = glob.glob(WHERE_AM_I + "/inputs/" + voltage + "/BRAM_*.txt")

            method_with_params = partial(run_experiment, bench_name=args.bench_name, flags=args.flags, voltage=voltage)

            executor.map(method_with_params, input_paths)