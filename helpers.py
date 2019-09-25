import argparse
import os
import glob
from shutil import rmtree
import sys
import subprocess
import random
import concurrent.futures

#voltages = ["0.54V", "0.55V", "0.56V", "0.57V", "0.58V", "0.59V", "0.60V"]
voltages = ["0.54V"]

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__)) #  Absolute Path to *THIS* Script

BENCH_INPUT_HOME = WHERE_AM_I + '/inputs/'
RANDOM_PATH = BENCH_INPUT_HOME + "random"

BENCH_BIN_HOME = WHERE_AM_I + '/tests/test-progs'

BENCH_BIN_DIR = {
    'matrix_mul' : os.path.abspath(BENCH_BIN_HOME + '/matrix_multiplication'),
    'blackscholes': os.path.abspath(BENCH_BIN_HOME + '/blackscholes'),
    'jacobi' : os.path.abspath(BENCH_BIN_HOME + '/jacobi'),
    'Kmeans' : os.path.abspath(BENCH_BIN_HOME + '/Kmeans'),
    'monteCarlo' : os.path.abspath(BENCH_BIN_HOME + '/monteCarlo'),
    'sobel' : os.path.abspath(BENCH_BIN_HOME + '/sobel'),
    'dct' : os.path.abspath(BENCH_BIN_HOME + '/dct')
}

BENCH_BINARY = {
    'matrix_mul' : os.path.abspath(BENCH_BIN_DIR["matrix_mul"] + '/matrix_mul'),
    'blackscholes': os.path.abspath(BENCH_BIN_DIR["blackscholes"] + '/blackscholes'),
    'jacobi' : os.path.abspath(BENCH_BIN_DIR["jacobi"] + '/jacobi'),
    'Kmeans' : os.path.abspath(BENCH_BIN_DIR["Kmeans"] + '/seq_main'),
    'monteCarlo' : os.path.abspath(BENCH_BIN_DIR["monteCarlo"] + '/monte_carlo'),
    'sobel' : os.path.abspath(BENCH_BIN_DIR["sobel"] + '/sobel'),
    'dct' : os.path.abspath(BENCH_BIN_DIR["dct"] + '/dct')
}

BENCH_QUALITY = {
    'blackscholes': os.path.abspath(BENCH_BIN_DIR["blackscholes"] + '/error '),
    'jacobi': os.path.abspath(BENCH_BIN_DIR["jacobi"] + '/error '),
    'Kmeans' : os.path.abspath(BENCH_BIN_DIR["Kmeans"] + '/compare '),
    'monteCarlo' : os.path.abspath(BENCH_BIN_DIR["monteCarlo"] + '/calc_errors '),
    'sobel' : os.path.abspath(BENCH_BIN_DIR["sobel"] + '/psnr '),
    'dct' : os.path.abspath(BENCH_BIN_DIR["dct"] + '/quality ')
}

def makeDirectories(bench_name, is_deterministic):

    bench_out_dir = getBenchOutDir(bench_name)

    if (os.path.exists(bench_out_dir) == False):
        os.mkdir(bench_out_dir)

    for v in voltages:
        if (os.path.exists(bench_out_dir + "/" + v ) == False):
            os.mkdir(bench_out_dir + "/" + v)
    
    if(not is_deterministic):
        if(os.path.exists(RANDOM_PATH) == False):
            os.mkdir(RANDOM_PATH)

        if (os.path.exists(RANDOM_PATH + "/" + bench_name) == False):
            os.mkdir(RANDOM_PATH + "/" + bench_name)

        for v in voltages:
            if (os.path.exists(RANDOM_PATH + "/" + bench_name + "/" + v) == False):
                os.mkdir(RANDOM_PATH + "/" + bench_name + "/" + v)

def removeDirectories(bench_name):
    if (os.path.exists(WHERE_AM_I + '/' + bench_name + '_results')):
        rmtree(WHERE_AM_I + '/' + bench_name + '_results', ignore_errors=True)

    if(os.path.exists(getBenchOutDir(bench_name))):
        rmtree(getBenchOutDir(bench_name), ignore_errors=True)

    if(os.path.exists(RANDOM_PATH + "/" + bench_name)):
        rmtree(RANDOM_PATH + "/" + bench_name , ignore_errors=True)

def compileBench(bench_name):
    if bench_name not in BENCH_BIN_DIR:
        print ( "Directory is not indexed" )
        sys.exit(-1)
    os.chdir(BENCH_BIN_DIR[bench_name])        

    try:    
        subprocess.check_call(["make clean"], shell=True)
    except Exception as e:
        sys.exit(str(e))

    try:    
        subprocess.check_call(["make CFLAGS=-DFI"], shell=True)
    except Exception as e:
        sys.exit(str(e))
    os.chdir(WHERE_AM_I)

def getBenchGoldenOut(bench_name):
    return BENCH_BIN_DIR[bench_name] + "/golden.bin"

def getBenchFaultyOut(bench_name, voltage, input_name):
    return getBenchOutDir(bench_name) + "/" + voltage + "/" + input_name

def getBenchOutDir(bench_name):
    return BENCH_BIN_DIR[bench_name] + "/outputs"

def getSimOutDir(bench_name, voltage, input_name):
    return WHERE_AM_I + "/" + bench_name + "_results/faulty/" + voltage + "/" + input_name

def getNumberOfErrors(input_path, voltage):
    grep_number_of_lines = 'grep ' + '"[0-9]" -c ' + input_path
    
    try:
        result = subprocess.Popen(grep_number_of_lines, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode('utf-8')
    except Exception as e:
        sys.exit(str(e))

    return int(result)

def createRandomInput(input_path, voltage, number_of_errors, bench_name):
    input_name = input_path.split("/")[-1]

    with open(RANDOM_PATH + "/" + bench_name + "/" + voltage + "/" + input_name, "w") as input_file:
        for _ in range(number_of_errors):
            fault_type = 0
            fault_set = random.randint(0,31)
            fault_byte_offset = random.randint(0,63)
            fault_bit_offset = random.randint(0,7)
            fault_cache_type = "l1d"

            line = ' '.join([str(fault_type), str(fault_set), str(fault_byte_offset), str(fault_bit_offset), fault_cache_type + "\n"])

            input_file.write(line)

def createRandomInputs(bench_name):
    for voltage in voltages:
        input_paths = glob.glob(BENCH_INPUT_HOME + voltage + "/BRAM_*.txt")

        for input_path in input_paths:
            number_of_errors = getNumberOfErrors(input_path, voltage)
            createRandomInput(input_path, voltage, number_of_errors, bench_name)

def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c','--bench-name', help='Benchmark\'s name', default='matrix_mul')
    parser.add_argument('-f', '--flags', action='store', nargs='*', help='All gem5 debug flags')
    parser.add_argument('-r', '--random', action='store_true')

    # Options for dct application : example run: ./dct <inputFile> <outputFile>
    parser.add_argument("--dct-input", help="Input file for dct application", default="")
    parser.add_argument("--dct-output", help="Output file for dct application", default="")

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
    parser.add_argument("--kmeans-output", help="Output file", default="")

    # Options for monte carlo application : example run: ./monte_carlo  5 5 50 5 out.bin
    parser.add_argument("--monte-x", help="Size of X", default="")
    parser.add_argument("--monte-y", help="Size of Y", default="")
    parser.add_argument("--monte-walks", help="Is diagonally dominant?", default="")
    parser.add_argument("--monte-tasks", help="Maximum iteration", default="")
    parser.add_argument("--monte-output", help="Output file", default="")

    # Options for sobel application : example run: ./sobel 'input file' 'output file' 
    parser.add_argument("--sobel-input", help="Input file", default="")
    parser.add_argument("--sobel-output", help="Output file", default="")

    # Options for matrix multiplication application : example run: ./matrix_mul 'output file'
    parser.add_argument("--matrix-output", help="Output file", default="output.txt")

    return parser.parse_args()

def get_binary_options(args, voltage="", is_golden = False, input_name=""):
        bench_binary_options = ''
        golden_option = "--output=" + getBenchGoldenOut(args.bench_name)
        faulty_option = "--output=" + getBenchFaultyOut(args.bench_name,voltage,input_name)

        output = golden_option if is_golden else faulty_option

        if(args.bench_name == "blackscholes"):
            blackscholes_input = "--blackscholes-input=" + args.blackscholes_input
            
            blackscholes_options = ' '.join([blackscholes_input, output])
            bench_binary_options = blackscholes_options
        elif(args.bench_name == "dct"):
            dct_input = "--dct-input=" + args.dct_input
            
            dct_options = ' '.join([dct_input, output])
            bench_binary_options = dct_options
        elif(args.bench_name == "jacobi"):
            jacobi_n = "--jacobi-n=" + args.jacobi_n
            jacobi_itol = "--jacobi-itol=" + args.jacobi_itol
            jacobi_dominant = "--jacobi-dominant=" + args.jacobi_dominant
            jacobi_maxiters = "--jacobi-maxiters=" + args.jacobi_maxiters

            jacobi_options = ' '.join([jacobi_n, jacobi_itol, jacobi_dominant, jacobi_maxiters, output])
            bench_binary_options = jacobi_options
        elif(args.bench_name == "Kmeans"):
            kmeans_o = "--kmeans-o" if args.kmeans_o else ""
            kmeans_b = "--kmeans-b" if args.kmeans_b else ""
            kmeans_n = "--kmeans-n=" + args.kmeans_n
            kmeans_i = "--kmeans-i=" + args.kmeans_i

            kmeans_options = ' '.join([kmeans_o, kmeans_b, kmeans_n, kmeans_i, output])
            bench_binary_options = kmeans_options
        elif(args.bench_name == "monteCarlo"):
            monte_x = "--monte-x=" + args.monte_x
            monte_y = "--monte-y=" + args.monte_y
            monte_walks = "--monte-walks=" + args.monte_walks
            monte_tasks = "--monte-tasks=" + args.monte_tasks

            monte_options = ' '.join([monte_x, monte_y, monte_walks, monte_tasks, output])
            bench_binary_options = monte_options
        elif(args.bench_name == "sobel"):
            sobel_input = "--sobel-input=" + args.sobel_input

            sobel_options = ' '.join([sobel_input, output])
            bench_binary_options = sobel_options
        elif(args.bench_name == "matrix_mul"):
            matrix_options = ' '.join([output])
            bench_binary_options = matrix_options

        return bench_binary_options

def write_results(input_name, args, voltage, result):

    with open(getSimOutDir(args.bench_name, voltage, input_name) + "/result.txt", "w") as result_file:
        if(args.bench_name == "blackscholes" or args.bench_name == "jacobi"):
            RE = "1.0"
            ABSE = "1.0"

            if(result != "Crash"):
                output_path = args.blackscholes_output if args.bench_name == "blackscholes" else args.jacobi_output
                error_command = BENCH_QUALITY[args.bench_name] + output_path + " " + getBenchFaultyOut(args.bench_name,voltage,input_name) 
                
                try:
                    error_string = subprocess.Popen(error_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(str(e))

                output = error_string.split(",")
                RE = output[0].strip()
                ABSE = output[1].strip()

            line = ",".join([input_name[:-4], result, RE, ABSE + "\n"])
        elif(args.bench_name == "dct"):
            quality = "0.0"
            
            if(result != "Crash"):
                quality_command = BENCH_QUALITY[args.bench_name] + args.dct_input + " " + getBenchGoldenOut(args.bench_name) + " " + getBenchFaultyOut(args.bench_name,voltage,input_name) 

                try:
                    quality_string = subprocess.Popen(quality_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(e)

                output = quality_string.split(",")
                quality = output[1].strip()

            line = ",".join([input_name[:-4], result, quality + "\n"])

        elif(args.bench_name == "Kmeans"):
            cluster_relative_error = "inf"
            cluster_absolute_error = "inf"
            correct_membership_percentage = "inf"

            if(result != "Crash"):
                grep_number_of_lines = 'grep "[0-9]" ' + args.kmeans_i + " -c"

                try:
                    number_of_lines = subprocess.Popen(grep_number_of_lines, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(str(e))
                
                compare_command = BENCH_QUALITY[args.bench_name] + getBenchGoldenOut(args.bench_name) + " " + getBenchFaultyOut(args.bench_name,voltage,input_name)  + " " + number_of_lines
                
                try:
                    compare_string = subprocess.Popen(compare_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(str(e))

                output = compare_string.rstrip().split("\n")
                res = output[-1].split(",")
                cluster_relative_error = res[0]
                cluster_absolute_error = res[1]
                correct_membership_percentage = res[2]

            line = ",".join([input_name[:-4], result, cluster_relative_error, cluster_absolute_error, correct_membership_percentage + "\n"])

        elif(args.bench_name == "monteCarlo"):
            MSE = "1.0"
            RE = "1.0"

            if(result != "Crash"):
                calc_errors_command = BENCH_QUALITY[args.bench_name] + args.monte_output + " " + getBenchFaultyOut(args.bench_name,voltage,input_name) 
                
                try:
                    calc_errors_string = subprocess.Popen(calc_errors_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(str(e))

                output = calc_errors_string.split(",")
                MSE = output[0].strip()
                RE = output[1].strip()

            line = ",".join([input_name[:-4], result, MSE, RE + "\n"])
        elif(args.bench_name == "sobel"):
            psnr = "0.0"

            if(result != "Crash"):
                psnr_command = BENCH_QUALITY[args.bench_name] + getBenchFaultyOut(args.bench_name,voltage,input_name)  + " " + getBenchGoldenOut(args.bench_name)

                try:
                    psnr_string = subprocess.Popen(psnr_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0].decode("utf-8")
                except Exception as e:
                    sys.exit(str(e))

                psnr = psnr_string.split(":")[1].strip()

            line = ",".join([input_name[:-4], result, psnr + "\n"])
        elif(args.bench_name == "matrix_mul"):
            line = ",".join([input_name[:-4], result + "\n"])

        result_file.write(line)

def mergeResults(bench_name, voltage):
    output_paths = glob.glob(WHERE_AM_I + "/" + bench_name + "_results/faulty/" + voltage + "/BRAM_*.txt")

    with open(WHERE_AM_I + "/" + bench_name + "_results/" + voltage + "_results.txt","a") as results_file:
        for out_path in output_paths:
            with open(out_path + "/result.txt") as result_file:
                result = result_file.read()
                results_file.write(result)
