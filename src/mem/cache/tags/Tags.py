from m5.params import *
from m5.proxy import *
from m5.objects.ClockedObject import ClockedObject
from m5.objects.IndexingPolicies import *

class BaseTags(ClockedObject):
    type = 'BaseTags'
    abstract = True
    cxx_header = "mem/cache/tags/base.hh"

    # Get system to which it belongs
    system = Param.System(Parent.any, "System we belong to")

    # Get the size from the parent (cache)
    size = Param.MemorySize(Parent.size, "capacity in bytes")

    # Get the block size from the parent (system)
    block_size = Param.Int(Parent.cache_line_size, "block size in bytes")

    # Get the tag lookup latency from the parent (cache)
    tag_latency = Param.Cycles(Parent.tag_latency,
                               "The tag lookup latency for this cache")

    # Get the warmup percentage from the parent (cache)
    warmup_percentage = Param.Percent(Parent.warmup_percentage,
        "Percentage of tags to be touched to warm up the cache")

    sequential_access = Param.Bool(Parent.sequential_access,
        "Whether to access tags and data sequentially")

    # Get indexing policy
    indexing_policy = Param.BaseIndexingPolicy(SetAssociative(),
        "Indexing policy")

    # Set the indexing entry size as the block size
    entry_size = Param.Int(Parent.cache_line_size,
                           "Indexing entry size in bytes")

class BaseSetAssoc(BaseTags):
    type = 'BaseSetAssoc'
    cxx_header = "mem/cache/tags/base_set_assoc.hh"

    # Get the cache associativity
    assoc = Param.Int(Parent.assoc, "associativity")

    # Get replacement policy from the parent (cache)
    replacement_policy = Param.BaseReplacementPolicy(
        Parent.replacement_policy, "Replacement policy")

class SectorTags(BaseTags):
    type = 'SectorTags'
    cxx_header = "mem/cache/tags/sector_tags.hh"

    # Get the cache associativity
    assoc = Param.Int(Parent.assoc, "associativity")

    # Number of sub-sectors (data blocks) per sector
    num_blocks_per_sector = Param.Int(1, "Number of sub-sectors per sector");

    # The indexing entry now is a sector block
    entry_size = Parent.cache_line_size * Self.num_blocks_per_sector

    # Get replacement policy from the parent (cache)
    replacement_policy = Param.BaseReplacementPolicy(
        Parent.replacement_policy, "Replacement policy")

class CompressedTags(SectorTags):
    type = 'CompressedTags'
    cxx_header = "mem/cache/tags/compressed_tags.hh"

    # Maximum number of compressed blocks per tag
    max_compression_ratio = Param.Int(2,
        "Maximum number of compressed blocks per tag.")

    # We simulate superblock as sector blocks
    num_blocks_per_sector = Self.max_compression_ratio

    # We virtually increase the number of data blocks per tag by multiplying
    # the cache size by the compression ratio
    size = Parent.size * Self.max_compression_ratio

class FALRU(BaseTags):
    type = 'FALRU'
    cxx_class = 'FALRU'
    cxx_header = "mem/cache/tags/fa_lru.hh"

    min_tracked_cache_size = Param.MemorySize("128kB", "Minimum cache size for"
                                              " which we track statistics")

    # This tag uses its own embedded indexing
    indexing_policy = NULL
