import os
import glob
import concurrent.futures 
from functools import partial
import random
import re
import argparse
import subprocess
import filecmp

voltages = ["0.54V", "0.55V", "0.56V", "0.57V", "0.58V", "0.59V", "0.60V"]

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'
BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/'

BENCH_BINARY = {
    'matrix_mul' : os.path.abspath(BENCH_BIN_HOME + '/matrix_multiplication/matrix_mul'),
    'blackscholes': os.path.abspath(BENCH_BIN_HOME + '/blackscholes/blackscholes'),
    'jacobi' : os.path.abspath(BENCH_BIN_HOME + '/jacobi/jacobi'),
    'Kmeans' : os.path.abspath(BENCH_BIN_HOME + '/Kmeans/seq_main'),
    'monteCarlo' : os.path.abspath(BENCH_BIN_HOME + '/monteCarlo/monte_carlo'),
    'sobel' : os.path.abspath(BENCH_BIN_HOME + '/sobel/sobel')
}

GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/X86/gem5.opt')
GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/one_level/run.py')

class ExperimentManager:
    ##
    #  example gem5 run:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    ##
    def __init__(self, args, input_name, voltage):
        self.args = args
        self.voltage = voltage
        self.input_name = input_name

    @staticmethod
    def get_binary_options(args, voltage, is_golden = False, input_name=""):
        bench_binary_options = ''
        blackscholes_input = "--blackscholes-input=" + args.blackscholes_input

        if(args.bench_name == "blackscholes"):
            if(is_golden):
                blackscholes_output = "--blackscholes-output=" + args.blackscholes_output
            else:
                blackscholes_output = "--blackscholes-output=" + BENCH_BIN_HOME + "/blackscholes/outputs/" + voltage + "/" + input_name

            blackscholes_options = ' '.join([blackscholes_input, blackscholes_output])
            bench_binary_options = blackscholes_options
        elif(args.bench_name == "jacobi"):
            jacobi_n = "--jacobi-n=" + args.jacobi_n
            jacobi_itol = "--jacobi-itol=" + args.jacobi_itol
            jacobi_dominant = "--jacobi-dominant=" + args.jacobi_dominant
            jacobi_maxiters = "--jacobi-maxiters=" + args.jacobi_maxiters
            jacobi_output = "--jacobi-output=" + args.jacobi_output

            jacobi_options = ' '.join([jacobi_n, jacobi_itol, jacobi_dominant, jacobi_maxiters, jacobi_output])
            bench_binary_options = jacobi_options
        elif(args.bench_name == "Kmeans"):
            kmeans_o = "--kmeans-o" if args.kmeans_o else ""
            kmeans_b = "--kmeans-b" if args.kmeans_b else ""
            kmeans_n = "--kmeans-n=" + args.kmeans_n
            kmeans_i = "--kmeans-i=" + args.kmeans_i

            kmeans_options = ' '.join([kmeans_o, kmeans_b, kmeans_n, kmeans_i])
            bench_binary_options = kmeans_options
        elif(args.bench_name == "monteCarlo"):
            monte_x = "--monte-x=" + args.monte_x
            monte_y = "--monte-y=" + args.monte_y
            monte_walks = "--monte-walks=" + args.monte_walks
            monte_tasks = "--monte_tasks=" + args.monte_tasks
            monte_output = "--monte-output=" + args.monte_output

            monte_options = ' '.join([monte_x, monte_y, monte_walks, monte_tasks, monte_output])
            bench_binary_options = monte_options
        elif(args.bench_name == "sobel"):
            sobel_input = "--sobel-input=" + args.sobel_input
            sobel_output = "--sobel-output=" + args.sobel_output

            sobel_options = ' '.join([sobel_input, sobel_output])
            bench_binary_options = sobel_options

        return bench_binary_options

    @staticmethod
    def run_golden(args, voltage):
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

        bench_binary_options = ExperimentManager.get_binary_options(args, voltage, True)

        input_path = '--input-path=' + BENCH_INPUT_HOME + "golden.txt"

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        subprocess.call(gem5_command, shell=True)

    def is_crash(self):
        grep_crash = 'cd ' + WHERE_AM_I + '/' + self.args.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name + ';grep "exiting with last active thread context" output.txt'
        result = subprocess.Popen(grep_crash, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
        decoded_result = result.decode('utf-8')

        if decoded_result and len(decoded_result) > 0:
            return False
        else:
            return True

    def get_correct_result(self):
        grep_correct_result = 'cd ' + WHERE_AM_I + '/results/' + self.bench_name + '_results/golden;grep -oh "^[0-9].*" output.txt'
        correct_result = subprocess.Popen(grep_correct_result, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]

        lines = correct_result.decode('utf-8').splitlines()

        result = []

        for i in range(len(lines)):
            result.extend(lines[i].strip().split(" "))

        return result
    
    def is_correct(self):
        if(self.args.bench_name == "blackscholes"):
            
        elif(self.args.bench_name == "jacobi"):

        elif(self.args.bench_name == "Kmeans"):

        elif(self.args.bench_name == "monteCarlo"):

        elif(self.args.bench_name == "sobel"):
            output_path = self.args.sobel_output
            golden_path = BENCH_BIN_HOME + "/sobel/figs/golden.grey"

            if(filecmp.cmp(output_path, golden_path, shallow=False)):
                return True
            else:
                return False
        elif(self.args.bench_name == "matrix_mul"):
            grep_result = 'cd ' + WHERE_AM_I + '/results/' + self.bench_name + '_results/faulty/' + self.voltage + "/" + self.input_name + ';grep -oh "^[0-9].*" output.txt'
            sim_result = subprocess.Popen(grep_result, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]

            lines = sim_result.decode('utf-8').splitlines()

            result = []

            for i in range(len(lines)):
                result.extend(lines[i].strip().split(" "))

            if correct_result == result:
                return True
            else:
                return False

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

        bench_binary_options = ExperimentManager.get_binary_options(self.args, self.voltage, self.input_name)

        input_path = '--input-path ' + BENCH_INPUT_HOME + voltage + "/" + self.input_name

        gem5_script_option = ' '.join([bench_binary_path, bench_binary_options, input_path])

        gem5_command = ' '.join([GEM5_BINARY, gem5_option, GEM5_SCRIPT, gem5_script_option])

        try:
            subprocess.call(gem5_command, shell=True, timeout=30)
        except subprocess.TimeoutExpired:
            return "Crash"

        if self.is_crash():
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

    with open(WHERE_AM_I + "/" + args.bench_name + "_results" + "/" + voltage + "_results.txt", "a") as result_file:
            line = ",".join([input_name[:-4], result + "\n"])
            result_file.write(line)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-c','--bench-name', help='Benchmark\'s name', default='matrix_mul')
    parser.add_argument('-f', '--flags', action='store', nargs='*', help='All gem5 debug flags', default=["Cache"])

    # Options for blackscholes application : example run: ./blackscholes <inputFile> <outputFile>
    parser.add_argument("--blackscholes-input", help="Input file for blackscholes application", default="")
    parser.add_argument("--blackscholes-output", help="Output file for blackscholes application", default="")

    # Options for jacobi application : example run: ./jacobi 1000 0.00000001 1 100 $outputFileName
    parser.add_argument("--jacobi-n", help="Size of matrix", default="")
    parser.add_argument("--jacobi-itol", help="Itol", default="")
    parser.add_argument("--jacobi-dominant", help="Is diagonally dominant?", default="")
    parser.add_argument("--jacobi-maxiters", help="Maximum iteration", default="")
    parser.add_argument("--jacobi-output", help="Output file", default="")

    # Options for kmeans application : example run: seq_main -o -b -n 4 -i Image_data/color17695.bin
    parser.add_argument("--kmeans-o", action="store_true", help="output timing results")
    parser.add_argument("--kmeans-b", action="store_true", help="input file is in binary format")
    parser.add_argument("--kmeans-n", help="number of clusters")
    parser.add_argument("--kmeans-i", help="file containing data to be clustered")

    # Options for monte carlo application : example run: ./monte_carlo  5 5 50 5 out.bin
    parser.add_argument("--monte-x", help="Size of X", default="")
    parser.add_argument("--monte-y", help="Size of Y", default="")
    parser.add_argument("--monte-walks", help="Is diagonally dominant?", default="")
    parser.add_argument("--monte-tasks", help="Maximum iteration", default="")
    parser.add_argument("--monte-output", help="Output file", default="")

    # Options for sobel application : example run: ./sobel 'input file' 'output file' 
    parser.add_argument("--sobel-input", help="Input file", default="")
    parser.add_argument("--sobel-output", help="Output file", default="")

    args = parser.parse_args()
    
    open(BENCH_INPUT_HOME + "golden.txt","w").close() # Empty file for golden run

    ExperimentManager.run_golden(args, voltage)

    for voltage in voltages:
        with concurrent.futures.ProcessPoolExecutor() as executor:
            input_paths = glob.glob(WHERE_AM_I + "/inputs/" + voltage + "/BRAM_*.txt")

            method_with_params = partial(run_experiment, args=args, voltage=voltage)

            executor.map(method_with_params, input_paths)
