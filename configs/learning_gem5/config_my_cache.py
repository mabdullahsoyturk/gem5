import m5

from optparse import OptionParser
from m5.objects import *

GEM5_PATH = "/home/muhammet/Downloads/gem5"

parser = OptionParser()
parser.add_option("--golden-run", action='store_true', help="Is golden run?")
parser.add_option("--fault-type", help="Type of fault")
parser.add_option("--fault-tick", help="which tick")
parser.add_option("--fault-addr", help="address of faulty line")
parser.add_option("--fault-bit", help="which bit is faulty")
parser.add_option("-p", "--program", help="Program to run", default=GEM5_PATH + "/tests/test-progs/matrix_multiplication/matrix_mul")
(options, args) = parser.parse_args()

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]

system.cpu = TimingSimpleCPU()

system.cache = MyCache(opts=options)

system.cpu.icache_port = system.cache.cpu_side
system.cpu.dcache_port = system.cache.cpu_side

system.membus = SystemXBar()

system.cache.mem_side = system.membus.slave

system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.master
system.cpu.interrupts[0].int_master = system.membus.slave
system.cpu.interrupts[0].int_slave = system.membus.master

system.mem_ctrl = DDR3_1600_8x8()
system.mem_ctrl.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.master

system.system_port = system.membus.slave

process = Process()
process.cmd = [options.program]
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
