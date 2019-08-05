import os

WHERE_AM_I = os.path.dirname(os.path.realpath(__file__))

class CacheFault:
    def __init__(self):
        self.read_fault()

    def read_fault(self):
        list_of_data = []
        with open(WHERE_AM_I + '/input.txt') as csv_file:
            data = csv_file.read()
            list_of_data = [column for column in data.split(',')]

        self.type = list_of_data[0]
        self.addr = int(list_of_data[1], 16)
        self.byte_offset = int(list_of_data[2])
        self.bit_offset  = int(list_of_data[3])
        self.tick_start = int(list_of_data[4])
        self.tick_end   = int(list_of_data[5]) if self.type == "intermittent" else 0
