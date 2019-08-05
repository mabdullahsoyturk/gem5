from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class BaseIndexingPolicy(SimObject):
    type = 'BaseIndexingPolicy'
    abstract = True
    cxx_header = "mem/cache/tags/indexing_policies/base.hh"

    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")

    # Get the entry size from the parent (tags)
    entry_size = Param.Int(Parent.entry_size, "entry size in bytes")

    # Get the associativity
    assoc = Param.Int(Parent.assoc, "associativity")

class SetAssociative(BaseIndexingPolicy):
    type = 'SetAssociative'
    cxx_class = 'SetAssociative'
    cxx_header = "mem/cache/tags/indexing_policies/set_associative.hh"

class SkewedAssociative(BaseIndexingPolicy):
    type = 'SkewedAssociative'
    cxx_class = 'SkewedAssociative'
    cxx_header = "mem/cache/tags/indexing_policies/skewed_associative.hh"
