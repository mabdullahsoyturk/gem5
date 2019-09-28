import m5
from m5.objects import Cache

# For all options see src/mem/cache/BaseCache.py

class L1Cache(Cache):
    assoc = 2
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

    def __init__(self):
        super(L1Cache, self).__init__()
        pass

    def connectBus(self, bus):
        self.mem_side = bus.slave

    def connectCPU(self, cpu):
        raise NotImplementedError

class L1ICache(L1Cache):
    size = '16kB'

    def __init__(self):
        self.cache_type = 'l1i'
        super(L1ICache, self).__init__()

    def connectCPU(self, cpu):
        self.cpu_side = cpu.icache_port

class L1DCache(L1Cache):
    size = '2kB'

    def __init__(self):
        self.cache_type = 'l1d'
        super(L1DCache, self).__init__()

    def connectCPU(self, cpu):
        self.cpu_side = cpu.dcache_port

class L2Cache(Cache):
    size = '256kB'
    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

    def __init__(self):
        self.cache_type = 'l2'
        super(L2Cache, self).__init__()

    def connectCPUSideBus(self, bus):
        self.cpu_side = bus.master

    def connectMemSideBus(self, bus):
        self.mem_side = bus.slave

class L3Cache(Cache):
    size = '8MB'
    assoc = 8
    tag_latency = 200
    data_latency = 200
    response_latency = 200
    mshrs = 100
    tgts_per_mshr = 4

    def __init__(self):
        self.cache_type = 'l3'
        super(L3Cache, self).__init__()

    def connectCPUSideBus(self, bus):
        self.cpu_side = bus.master

    def connectMemSideBus(self, bus):
        self.mem_side = bus.slave