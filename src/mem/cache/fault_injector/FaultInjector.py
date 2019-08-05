from m5.params import *
from m5.SimObject import SimObject

class FaultInjector(SimObject):
    type = 'FaultInjector'
    cxx_header = "mem/cache/fault_injector/fault_injector.hh"

    input_path = Param.String("/home/muhammet/Downloads/gem5/input.txt", "Path of input file")