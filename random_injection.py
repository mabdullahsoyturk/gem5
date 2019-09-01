import os
import sys
import glob
import subprocess
from functools import partial
import filecmp
import helpers

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/'
BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'
BENCH_BIN_DIR = helpers.BENCH_BIN_DIR
BENCH_BINARY = helpers.BENCH_BINARY
BENCH_GOLDEN = helpers.BENCH_GOLDEN

GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/X86/gem5.opt')
GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/one_level/run.py')

class ExperimentManager:
    ##
    #  example gem5 run:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    ##
    def __init__(self, args, input_name):
        self.args = args
        self.input_name = input_name

    @staticmethod
    def run_golden(args):
        redirection = '-re'
        outdir = '--outdir=' + args.bench_name + '_results/golden'
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'
        debug_flags = ''

        if args.flags and len(args.flags) > 0:
            all_flags = ','.join(args.flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file, debug_flags])

        bench_binary_path = '-c ' + BENCH_BINARY[args.bench_name]

        bench_binary_options = helpers.get_binary_options(args, is_golden = True, is_random = True)

        input_path = '--input-path=' + BENCH_INPUT_HOME + "golden.txt"

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        try:    
            subprocess.check_call(gem5_command, shell=True)
        except Exception as e:
            print(str(e))
            sys.exit(str(e))
    

    def is_crash(self):
        grep_crash = 'grep "exiting with last active thread context" ' + WHERE_AM_I + '/' + self.args.bench_name + '_results/faulty/' + self.input_name + "/output.txt"
        result = ""
        decoded_result = ""

        try:
            result = subprocess.Popen(grep_crash, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
            decoded_result = result.decode('utf-8')
        except Exception as e:
            print(str(e))

        if decoded_result and len(decoded_result) > 0:
            return False
        else:
            return True

    def is_correct(self):
        if(self.args.bench_name != "Kmeans"):
            golden_path = BENCH_BIN_DIR[self.args.bench_name] + "/golden.bin"
            output_path = BENCH_BIN_DIR[self.args.bench_name] + "/outputs/" + self.input_name

            try:
                if(filecmp.cmp(output_path, golden_path, shallow=False)):
                    return True
                else:
                    return False
            except Exception as e:
                print(str(e))
                sys.exit(str(e))
        elif(self.args.bench_name == "Kmeans"):
            golden_path = BENCH_BIN_DIR[self.args.bench_name] + "/golden.bin.membership"
            output_path = BENCH_BIN_DIR[self.args.bench_name] + "/outputs/" + self.input_name + ".membership"

            try:
                if(filecmp.cmp(output_path, golden_path, shallow=False)):
                    return True
                else:
                    return False
            except Exception as e:
                print(str(e))
                sys.exit(str(e))

    def inject(self):
        redirection = '-re'
        outdir = '--outdir=' + self.args.bench_name + '_results/faulty/' + self.input_name
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'
        debug_flags = ''

        if self.args.flags and len(self.args.flags) > 0:
            all_flags = ','.join(self.args.flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file, debug_flags])

        bench_binary_path = '-c ' + BENCH_BINARY[self.args.bench_name]

        bench_binary_options = helpers.get_binary_options(self.args, False, self.input_name, is_random=True)

        input_path = '--input-path ' + BENCH_INPUT_HOME + "random/" + self.input_name

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])
        
        try:
            subprocess.check_call(gem5_command, shell=True, timeout=1800)
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError):
            return "Crash"

        if self.is_crash():
            return "Crash"

        if(self.is_correct()):
            return "Correct"
        else:
            return "Incorrect"


if __name__ == '__main__':
    args = helpers.get_arguments()
    
    open(BENCH_INPUT_HOME + "golden.txt","w").close() # Empty file for golden run
   
    helpers.compileBench(args.bench_name)      # Compile benchmarks
    helpers.removeDirectories(args.bench_name) # Remove the results of previous experiments
    helpers.makeDirectories(args.bench_name, False)   # Make new directories for these experiments