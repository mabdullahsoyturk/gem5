import os
import m5
from os.path import dirname as up
from m5.objects import *
from caches import *
from options import get_opts
from optparse import OptionParser

GEM5_PATH = up(up(up(__file__)))

BENCH_BIN_HOME = GEM5_PATH + '/tests/test-progs'

GOLDEN_INPUT_PATH = GEM5_PATH + "inputs/golden.txt"

(opts, args) = get_opts()

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'atomic'               # Use timing accesses
system.mem_ranges = [AddrRange('512MB')] # Create an address range

system.cpu = AtomicSimpleCPU()

system.membus = SystemXBar()

system.cpu.dcache = L1DCache(opts)
system.cpu.icache = L1ICache(opts)

fault_injector = FaultInjector(input_path=opts.input_path)

system.cpu.dcache.fault_injector = fault_injector
system.cpu.icache.fault_injector = fault_injector

system.cpu.dcache.connectCPU(system.cpu)
system.cpu.icache.connectCPU(system.cpu)

if opts.cache_level == "1":
    system.cpu.dcache.connectBus(system.membus)
    system.cpu.icache.connectBus(system.membus)
elif opts.cache_level == "2":
    system.l2bus = L2XBar()

    system.cpu.dcache.connectBus(system.l2bus)
    system.cpu.icache.connectBus(system.l2bus)

    system.l2cache = L2Cache(opts)
    system.l2cache.connectCPUSideBus(system.l2bus)
    system.l2cache.fault_injector = fault_injector
    system.l2cache.connectMemSideBus(system.membus)
elif opts.cache_level == "3":
    system.l2bus = L2XBar()

    system.cpu.dcache.connectBus(system.l2bus)
    system.cpu.icache.connectBus(system.l2bus)

    system.l2cache = L2Cache(opts)
    system.l2cache.connectCPUSideBus(system.l2bus)
    system.l2cache.fault_injector = fault_injector

    system.l3bus = L3XBar()

    system.l2cache.connectMemSideBus(system.l3bus)

    system.l3cache = L3Cache(opts)
    system.l3cache.connectCPUSideBus(system.l3bus)
    system.l3cache.connectMemSideBus(system.membus)
    system.l3cache.fault_injector = faultInjector

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
    process.cmd = [opts.bench_path] + [opts.blackscholes_input, opts.output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/dct/dct')):
    process.cmd = [opts.bench_path] + [opts.dct_input, opts.output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/jacobi/jacobi')):
    process.cmd = [opts.bench_path] + [opts.jacobi_n, opts.jacobi_itol, opts.jacobi_dominant, opts.jacobi_maxiters, opts.output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/Kmeans/seq_main')):
    process.cmd = [opts.bench_path] + ["-o", "-b" if opts.kmeans_b else "", "-i", opts.kmeans_i, "-n", opts.kmeans_n, "-w", opts.output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/monteCarlo/monte_carlo')):
    process.cmd = [opts.bench_path] + [opts.monte_x, opts.monte_y, opts.monte_walks, opts.monte_tasks, opts.output]
elif(opts.bench_path == os.path.abspath(BENCH_BIN_HOME + '/sobel/sobel')):
    process.cmd = [opts.bench_path] + [opts.sobel_input, opts.output]
else:
    process.cmd = [opts.bench_path] + [opts.output]
# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
