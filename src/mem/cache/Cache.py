from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

from m5.objects.ClockedObject import ClockedObject
from m5.objects.Compressors import BaseCacheCompressor
from m5.objects.Prefetcher import BasePrefetcher
from m5.objects.ReplacementPolicies import *
from m5.objects.Tags import *

# Enum for cache clusivity, currently mostly inclusive or mostly
# exclusive.
class Clusivity(Enum): vals = ['mostly_incl', 'mostly_excl']

class WriteAllocator(SimObject):
    type = 'WriteAllocator'
    cxx_header = "mem/cache/cache.hh"

    # Control the limits for when the cache introduces extra delays to
    # allow whole-line write coalescing, and eventually switches to a
    # write-no-allocate policy.
    coalesce_limit = Param.Unsigned(2, "Consecutive lines written before "
                                    "delaying for coalescing")
    no_allocate_limit = Param.Unsigned(12, "Consecutive lines written before"
                                       " skipping allocation")

    delay_threshold = Param.Unsigned(8, "Number of delay quanta imposed on an "
                                     "MSHR with write requests to allow for "
                                     "write coalescing")

    block_size = Param.Int(Parent.cache_line_size, "block size in bytes")


class BaseCache(ClockedObject):
    type = 'BaseCache'
    abstract = True
    cxx_header = "mem/cache/base.hh"

    size = Param.MemorySize("Capacity")
    assoc = Param.Unsigned("Associativity")

    tag_latency = Param.Cycles("Tag lookup latency")
    data_latency = Param.Cycles("Data access latency")
    response_latency = Param.Cycles("Latency for the return path on a miss");

    warmup_percentage = Param.Percent(0,
        "Percentage of tags to be touched to warm up the cache")

    max_miss_count = Param.Counter(0,
        "Number of misses to handle before calling exit")

    mshrs = Param.Unsigned("Number of MSHRs (max outstanding requests)")
    demand_mshr_reserve = Param.Unsigned(1, "MSHRs reserved for demand access")
    tgts_per_mshr = Param.Unsigned("Max number of accesses per MSHR")
    write_buffers = Param.Unsigned(8, "Number of write buffers")

    is_read_only = Param.Bool(False, "Is this cache read only (e.g. inst)")

    prefetcher = Param.BasePrefetcher(NULL,"Prefetcher attached to cache")
    prefetch_on_access = Param.Bool(False,
         "Notify the hardware prefetcher on every access (not just misses)")

    tags = Param.BaseTags(BaseSetAssoc(), "Tag store")
    replacement_policy = Param.BaseReplacementPolicy(LRURP(),
        "Replacement policy")

    compressor = Param.BaseCacheCompressor(NULL, "Cache compressor.")

    sequential_access = Param.Bool(False,
        "Whether to access tags and data sequentially")

    cpu_side = SlavePort("Upstream port closer to the CPU and/or device")
    mem_side = MasterPort("Downstream port closer to memory")

    addr_ranges = VectorParam.AddrRange([AllMemory],
         "Address range for the CPU-side port (to allow striping)")

    system = Param.System(Parent.any, "System we belong to")

    # Determine if this cache sends out writebacks for clean lines, or
    # simply clean evicts. In cases where a downstream cache is mostly
    # exclusive with respect to this cache (acting as a victim cache),
    # the clean writebacks are essential for performance. In general
    # this should be set to True for anything but the last-level
    # cache.
    writeback_clean = Param.Bool(False, "Writeback clean lines")

    # Control whether this cache should be mostly inclusive or mostly
    # exclusive with respect to upstream caches. The behaviour on a
    # fill is determined accordingly. For a mostly inclusive cache,
    # blocks are allocated on all fill operations. Thus, L1 caches
    # should be set as mostly inclusive even if they have no upstream
    # caches. In the case of a mostly exclusive cache, fills are not
    # allocating unless they came directly from a non-caching source,
    # e.g. a table walker. Additionally, on a hit from an upstream
    # cache a line is dropped for a mostly exclusive cache.
    clusivity = Param.Clusivity('mostly_incl',
                                "Clusivity with upstream cache")

    # The write allocator enables optimizations for streaming write
    # accesses by first coalescing writes and then avoiding allocation
    # in the current cache. Typically, this would be enabled in the
    # data cache.
    write_allocator = Param.WriteAllocator(NULL, "Write allocator")

    cache_type = Param.String("Type of cache")

    golden_run = Param.Bool(False, "Without fault")
    
    fault_injector = Param.FaultInjector(NULL, "A fault injector")

class Cache(BaseCache):
    type = 'Cache'
    cxx_header = 'mem/cache/cache.hh'

class NoncoherentCache(BaseCache):
    type = 'NoncoherentCache'
    cxx_header = 'mem/cache/noncoherent_cache.hh'

    # This is typically a last level cache and any clean
    # writebacks would be unnecessary traffic to the main memory.
    writeback_clean = False

