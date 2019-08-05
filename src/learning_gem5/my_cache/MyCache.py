from m5.params import *
from m5.proxy import *
from m5.objects.MemObject import MemObject

class MyCache(MemObject):
    type = 'MyCache'
    cxx_header = 'learning_gem5/my_cache/my_cache.hh'

    cpu_side = VectorSlavePort("CPU Side Port, receives requests")
    mem_side = MasterPort("Memory side port, sends requests")

    latency = Param.Cycles(1, "Cycles taken on a hit or to resolve a miss")
    size = Param.MemorySize('16kB', "The size of cache")
    system = Param.System(Parent.any, "The system this cache is part of") # Neeeded to get block size

    golden_run = Param.Bool(False, "Is it golden run?")
    fault_type = Param.String('permanent', "Fault type")
    fault_tick = Param.Int(-1, "Fault tick")
    fault_addr = Param.Int(123, "Fault address")
    fault_bit = Param.Int(1, "Which bit")

    def __init__(self, opts):
        super(MyCache, self).__init__()

        self.golden_run = opts.golden_run if opts.golden_run else False
        self.fault_type = opts.fault_type if opts.fault_type else 'permanent'
        self.fault_tick = opts.fault_tick if opts.fault_tick else 0
        self.fault_addr = opts.fault_addr if opts.fault_addr else -1
        self.fault_bit  = opts.fault_bit  if opts.fault_bit else 0
