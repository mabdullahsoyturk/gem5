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
    GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/X86/gem5.opt')
    GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/one_level/run.py')
    FAULT_INPUT_PATH = os.path.abspath(WHERE_AM_I + '/inputs/input.txt')
    
    def run_golden(self, bench_name, flags):
        open(BENCH_INPUT_HOME, 'w').close() # Empty fault file.

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

        bench_binary = '-c ' + BENCH_BINARY[bench_name]

        gem5_script_option = ' '.join([bench_binary])

        gem5_command = ' '.join([self.GEM5_BINARY, gem5_option, self.GEM5_SCRIPT, gem5_script_option])
        subprocess.call(gem5_command, shell=True)

    def get_block_addresses(self, bench_name):
        prepare_addresses = 'cd ' + WHERE_AM_I + '/' + bench_name + '_results/golden;' + 'grep "being updated in Cache" log.txt | grep -oh "0x[0-9a-z]*" > block_addresses'
        subprocess.call(prepare_addresses, shell=True)

        addresses = []

        filepath = WHERE_AM_I + '/' + bench_name + '_results/golden/block_addresses'
        with open(filepath) as fp:
            line = fp.readline()
            while line:
                line = fp.readline()
                line = line.strip()
                
                if(line):
                    addresses.append(line)
            fp.close()
        
        return addresses 

    def get_last_cycle(self, bench_name):
        get_cycle = 'cd ' + WHERE_AM_I + '/' + bench_name + '_results/golden;' + 'tail -1 log.txt | egrep -o "^[0-9]*"'
        last_cycle = subprocess.Popen(get_cycle, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]

        return last_cycle

    def create_random_faults(self,bench_name, number_of_faults):
        addresses = self.get_block_addresses(bench_name)
        last_cycle = self.get_last_cycle(bench_name).strip()

        with open(WHERE_AM_I + "/inputs/input.txt", "w") as input_file:
            for i in range(int(number_of_faults)):
                fault_type = random.randint(0, 2)
                fault_block_address = int(addresses[random.randint(0, len(addresses) - 1)], 16)
                fault_byte_offset = random.randint(0,63)
                fault_bit_offset = random.randint(0, 7)
                fault_tick_start = 0
                fault_tick_end = 0
    
                if(fault_type == 0):
                    fault_tick_start = random.randint(0, int(last_cycle))
                elif(fault_type == 1):
                    fault_tick_start = random.randint(0, int(last_cycle))
                    fault_tick_end = random.randint(fault_tick_start + 1, int(last_cycle))
                
                fault_stuck_at = random.randint(0,1)
                fault_cache_type = "l1d" # for now

                line = " ".join([str(fault_type),str(fault_block_address),str(fault_byte_offset),str(fault_bit_offset),str(fault_tick_start),str(fault_tick_end),str(fault_stuck_at),fault_cache_type]) + "\n"

                input_file.write(line)


    def inject_random(self, bench_name, flags, iteration, number_of_faults=1):
        self.create_random_faults(bench_name, number_of_faults)

        redirection = '-re'
        outdir = '--outdir=' + bench_name + '_results/faulty/faulty' + str(iteration + 1)
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'

        debug_flags = ''

        if flags and len(flags) > 0:
            all_flags = ','.join(flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file, debug_flags])

        bench_binary = '-c ' + BENCH_BINARY[bench_name]

        gem5_script_option = ' '.join([bench_binary])

        gem5_command = ' '.join([self.GEM5_BINARY, gem5_option, self.GEM5_SCRIPT, gem5_script_option])
        subprocess.call(gem5_command, shell=True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-c','--bench-name', help='Benchmark\'s name', default='matrix_mul')
    parser.add_argument('-f', '--flags', action='store', nargs='*', help='All gem5 debug flags')
    parser.add_argument('-n', '--number-of-faults', help='Number of faults', default=1)
    parser.add_argument('-e', '--number-of-experiments', help='Number of experiments', default=1)

    args = parser.parse_args()

    experiment_manager = ExperimentManager()

    experiment_manager.run_golden(args.bench_name, args.flags)
    
    for iteration in range(int(args.number_of_experiments)):
        experiment_manager.inject_random(args.bench_name, args.flags, iteration, args.number_of_faults)