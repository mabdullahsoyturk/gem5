from m5.params import *

from m5.objects.BaseSimpleCPU import BaseSimpleCPU

class TimingSimpleCPU(BaseSimpleCPU):
    type = 'TimingSimpleCPU'
    cxx_header = "cpu/simple/timing.hh"

    @classmethod
    def memory_mode(cls):
        return 'timing'

    @classmethod
    def support_take_over(cls):
        return True
