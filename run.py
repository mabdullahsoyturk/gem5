import os
import sys
import glob
import subprocess
from functools import partial
import concurrent.futures 
import filecmp
import helpers

voltages = helpers.voltages

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/'
BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'
BENCH_BIN_DIR = helpers.BENCH_BIN_DIR
BENCH_BINARY = helpers.BENCH_BINARY

GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/X86/gem5.opt')
GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/three_level/run.py')

class ExperimentManager:
    ##
    #  example gem5 run:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    ##
    def __init__(self, args, input_name, voltage):
        self.args = args
        self.input_name = input_name
        self.voltage = voltage

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

        bench_binary_options = helpers.get_binary_options(args, is_golden = True)

        input_path = '--input-path=' + BENCH_INPUT_HOME + "golden.txt"

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        try:    
            subprocess.check_call(gem5_command, shell=True)
        except Exception as e:
            sys.exit(str(e))

    def is_crash(self):
        grep_crash = 'grep "Error" ' + WHERE_AM_I + '/' + self.args.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name + "/output.txt"

        try:
            decoded_result = subprocess.Popen(grep_crash, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode('utf-8')
        except Exception as e:
            sys.exit(str(e))

        if decoded_result and len(decoded_result) > 0:
            return True
        else:
            return False

    def is_correct(self):
        if(self.args.bench_name == "Kmeans"):
            grep_number_of_lines = 'grep "[0-9]" ' + self.args.kmeans_i + " -c"
            
            try:
                number_of_lines = subprocess.Popen(grep_number_of_lines, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
            except Exception as e:
                sys.exit(str(e))

            compare_command = BENCH_BIN_DIR["Kmeans"] + "/compare " + helpers.getBenchGoldenOut(self.args.bench_name) + " " + BENCH_BIN_DIR["Kmeans"] + "/outputs/" + voltage + "/" + self.input_name + " " + number_of_lines

            try:
                compare_string = subprocess.Popen(compare_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
            except Exception as e:
                sys.exit(str(e))

            output = compare_string.rstrip().split("\n")
            res = output[-1].split(",")

            if(res == ["0","0","1"]):
                return True
            else:
                return False
        elif(self.args.bench_name == "dct"):
            quality_command = BENCH_BIN_DIR["dct"] + "/quality " + helpers.getBenchGoldenOut(self.args.bench_name) + " " + BENCH_BIN_DIR["dct"] + "/outputs/" + self.voltage + "/" + self.input_name
            
            try:
                quality_string = subprocess.Popen(quality_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
            except Exception as e:
                sys.exit(str(e))

            output = quality_string.split(",")
            is_correct = output[0].strip()
            
            if(is_correct == "Correct"):
                return True
            else:
                return False
        else:
            golden_path = helpers.getBenchGoldenOut(self.args.bench_name)
            output_path = BENCH_BIN_DIR[self.args.bench_name] + "/outputs/" + self.voltage + "/" + self.input_name

            try:
                if(filecmp.cmp(output_path, golden_path, shallow=False)):
                    return True
                else:
                    return False
            except Exception as e:
                sys.exit(str(e))

    def inject(self):
        redirection = '-re'
        outdir = '--outdir=' + self.args.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name
        stdout_file = '--stdout-file=output.txt'
        stderr_file = '--stderr-file=error.txt'
        debug_file = '--debug-file=log.txt'
        debug_flags = ''

        if self.args.flags and len(self.args.flags) > 0:
            all_flags = ','.join(self.args.flags)
            debug_flags = '--debug-flags=' + all_flags

        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, debug_file, debug_flags])

        bench_binary_path = '-c ' + BENCH_BINARY[self.args.bench_name]

        bench_binary_options = helpers.get_binary_options(self.args, self.voltage, False, self.input_name)

        input_path = '--input-path=' + BENCH_INPUT_HOME + (("random/" + self.args.bench_name + "/") if args.random else "") + self.voltage + "/" + self.input_name

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])
        
        try:
            subprocess.check_call(gem5_command, shell=True, timeout=1800)
        except subprocess.SubprocessError as e:
            print("Crashed because " + str(e))
            return "Crash"

        if(self.is_crash()):
            return "Crash"

        if(self.is_correct()):
            return "Correct"
        else:
            return "Incorrect"

def run_experiment(input_path, args, voltage):
    input_name = input_path.split("/")[-1]
    experiment_manager = ExperimentManager(args, input_name, voltage)

    result = experiment_manager.inject()
    print("Voltage: " + voltage + ", Fault input: " + input_name + ", Result: " + result)

    helpers.write_results(input_name, args, voltage, result)

if __name__ == '__main__':
    args = helpers.get_arguments()
    
    open(BENCH_INPUT_HOME + "golden.txt","w").close() # Empty file for golden run
   
    helpers.compileBench(args.bench_name)      # Compile benchmarks
    helpers.removeDirectories(args.bench_name) # Remove the results of previous experiments
    helpers.makeDirectories(args.bench_name, False)   # Make new directories for these experiments

    ExperimentManager.run_golden(args)

    if(args.random):
        helpers.createRandomInputs(args.bench_name)

    for voltage in voltages:
        with concurrent.futures.ProcessPoolExecutor(max_workers=4) as executor: 
            inputs = BENCH_INPUT_HOME + (("random/" + args.bench_name + "/") if args.random else "") + voltage + "/BRAM_*.txt"

            input_paths = glob.glob(inputs)

            method_with_params = partial(run_experiment, args=args, voltage=voltage)

            executor.map(method_with_params, input_paths)
        helpers.mergeResults(args.bench_name, voltage)