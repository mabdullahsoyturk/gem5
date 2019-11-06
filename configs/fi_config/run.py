import m5
from m5.objects import *
from caches import *
from options import get_opts, get_process_cmd

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

process.cmd = get_process_cmd(opts)

# Set the cpu to use the process as its workload and create thread contexts
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause()))
