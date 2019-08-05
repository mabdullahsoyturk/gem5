from m5.params import *
from m5.proxy import *
from m5.objects.MemObject import MemObject

class MyMemobj(MemObject):
    type = 'MyMemobj'
    cxx_header = 'learning_gem5/my_memobj/my_memobj.hh'

    inst_port = SlavePort("CPU Side Port, receives requests")
    data_port = SlavePort("CPU Side Port, receives requests")
    mem_side_port = MasterPort("Memory side port, sends requests")
