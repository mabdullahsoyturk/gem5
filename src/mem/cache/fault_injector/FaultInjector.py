from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject
from m5.objects.IndexingPolicies import *

class FaultInjector(SimObject):
    type = 'FaultInjector'
    cxx_header = "mem/cache/fault_injector/fault_injector.hh"

    input_path = Param.String("/home/muhammet/Downloads/gem5/inputs/input.txt", "Path of input file")

    assoc = Param.Int(Parent.assoc, "associativity")