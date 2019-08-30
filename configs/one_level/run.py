import os
import m5
from m5.objects import *
from caches import *
from optparse import OptionParser

BENCH_BIN_HOME = '/home/muhammet/Downloads/gem5/tests/test-progs'

input_file = "/home/muhammet/Downloads/gem5/inputs/golden.txt"

parser = OptionParser()

parser.add_option("-c", "--bench-path", help="Binary of
                            the program to be simulated")
parser.add_option("--input-path", help="Fault input file", default=input_file)

# Options for blackscholes application : example run: ./blackscholes <inputFile> <outputFile>
parser.add_option("--blackscholes-input", help="Input file for blackscholes application", default="")
parser.add_option("--blackscholes-output", help="Output file for blackscholes application", default="")

# Options for jacobi application : example run: ./jacobi 1000 0.00000001 1 100 $outputFileName
parser.add_option("--jacobi-n", help="Size of matrix", default="")
parser.add_option("--jacobi-itol", help="Itol", default="")
parser.add_option("--jacobi-dominant", help="Is diagonally dominant?", default="")
parser.add_option("--jacobi-maxiters", help="Maximum iteration", default="")
parser.add_option("--jacobi-output", help="Output file", default="")

# Options for kmeans application : example run: seq_main -o -b -n 4 -i Image_data/color17695.bin
parser.add_option("--kmeans-o", action="store_true", help="output timing results")
parser.add_option("--kmeans-b", action="store_true", help="input file is in binary format")
parser.add_option("--kmeans-n", help="number of clusters")
parser.add_option("--kmeans-i", help="file containing data to be clustered")

# Options for monte carlo application : example run: ./monte_carlo  5 5 50 5 out.bin
parser.add_option("--monte-x", help="Size of X", default="")
parser.add_option("--monte-y", help="Size of Y", default="")
parser.add_option("--monte-walks", help="Is diagonally dominant?", default="")
parser.add_option("--monte-tasks", help="Maximum iteration", default="")
parser.add_option("--monte-output", help="Output file", default="")

# Options for sobel application : example run: ./sobel 'input file' 'output file' 
parser.add_option("--sobel-input", help="Input file", default="")
parser.add_option("--sobel-output", help="Output file", default="")

# Options for matrix multiplication application :
#example run: ./matrix_mul 'output file'
parser.add_option("--matrix-output", help="Output file", default="output.txt")

(opts, args) = parser.parse_args()

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'atomic'               # Use timing accesses
system.mem_ranges = [AddrRange('512MB')] # Create an address range

system.cpu = AtomicSimpleCPU()

system.membus = SystemXBar()

system.cpu.icache = L1ICache(opts)
system.cpu.dcache = L1DCache(opts)

system.cpu.icache.fault_injector = FaultInjector(input_path=opts.input_path)
system.cpu.dcache.fault_injector = FaultInjector(input_path=opts.input_path)

system.cpu.icache.connectCPU(system.cpu)
system.cpu.dcache.connectCPU(system.cpu)

system.cpu.icache.connectBus(system.membus)
system.cpu.dcache.connectBus(system.membus)

system.cpu.createInterruptController()

# For x86 only, make sure the interrupts are connected to the memory
# Note: these are directly connected to the memory bus and are not cached
if m5.defines.buildEnv['TARGET_ISA'] == "x86":
    system.cpu.interrupts[0].pio = system.membus.master
    system.cpu.interrupts[0].int_master = system.membus.slave
    system.cpu.interrupts[0].int_slave = system.membus.master

# Connect the system up to the membus
system.system_port = system.membus.slave

# Create a DDR3 memory controller
system.mem_ctrl = DDR3_1600_8x8()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master

process = Process()

if(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/blackscholes/blackscholes')):
    process.cmd = [opts.bench_path] + [opts.blackscholes_input, opts.blackscholes_output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/jacobi/jacobi')):
    process.cmd = [opts.bench_path] + [opts.jacobi_n, opts.jacobi_itol, opts.jacobi_dominant, opts.jacobi_maxiters, opts.jacobi_output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/Kmeans/seq_main')):
    process.cmd = [opts.bench_path] + ["-o" if opts.kmeans_o else "", "-b" if opts.kmeans_b else "", opts.kmeans_i, opts.kmeans_n]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/monteCarlo/monte_carlo')):
    process.cmd = [opts.bench_path] + [opts.monte_x, opts.monte_y, opts.monte_walks, opts.monte_tasks, opts.monte_output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/sobel/sobel')):
    process.cmd = [opts.bench_path] + [opts.sobel_input, opts.sobel_output]
else:
    process.cmd = [opts.bench_path] + [opts.matrix_output]
# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
