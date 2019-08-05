import m5
from m5.objects import *
from caches import *
from optparse import OptionParser

binary = "/home/muhammet/Downloads/gem5/tests/test-progs/matrix_multiplication/matrix_mul"
input_file = "/home/muhammet/Downloads/gem5/inputs/input.txt"

parser = OptionParser()

parser.add_option("--golden-run", action='store_true', help="No faults")
parser.add_option("-p", "--program", help="Program to run", default=binary)
parser.add_option("-i", "--input", help="Fault input file", default=input_file)

(opts, args) = parser.parse_args()

system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'timing'               # Use timing accesses
system.mem_ranges = [AddrRange('512MB')] # Create an address range

system.cpu = TimingSimpleCPU()

system.membus = SystemXBar()

system.cpu.icache = L1ICache(opts)
system.cpu.dcache = L1DCache(opts)

system.cpu.icache.fault_injector = FaultInjector(input_path=input_file)
system.cpu.dcache.fault_injector = FaultInjector(input_path=input_file)

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
process.cmd = [binary]
# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
