#ifndef __MEM_CACHE_INDEXING_POLICIES_BASE_HH__
#define __MEM_CACHE_INDEXING_POLICIES_BASE_HH__

#include <vector>

#include "params/BaseIndexingPolicy.hh"
#include "sim/sim_object.hh"

class ReplaceableEntry;

/**
 * A common base class for indexing table locations. Classes that inherit
 * from it determine hash functions that should be applied based on the set
 * and way. These functions are then applied to re-map the original values.
 * @sa  \ref gem5MemorySystem "gem5 Memory System"
 */
class BaseIndexingPolicy : public SimObject
{
  protected:
    /**
     * The associativity.
     */
    const unsigned assoc;

    /**
     * The number of sets in the cache.
     */
    const uint32_t numSets;

    /**
     * The amount to shift the address to get the set.
     */
    const int setShift;

    /**
     * Mask out all bits that aren't part of the set index.
     */
    const unsigned setMask;

    /**
     * The cache sets.
     */
    std::vector<std::vector<ReplaceableEntry*>> sets;

    /**
     * The amount to shift the address to get the tag.
     */
    const int tagShift;

  public:
    /**
     * Convenience typedef.
     */
    typedef BaseIndexingPolicyParams Params;

    /**
     * Construct and initialize this policy.
     */
    BaseIndexingPolicy(const Params *p);

    /**
     * Destructor.
     */
    ~BaseIndexingPolicy() {};

    /**
     * Associate a pointer to an entry to its physical counterpart.
     *
     * @param entry The entry pointer.
     * @param index An unique index for the entry.
     */
    void setEntry(ReplaceableEntry* entry, const uint64_t index);

    /**
     * Get an entry based on its set and way. All entries must have been set
     * already before calling this function.
     *
     * @param set The set of the desired entry.
     * @param way The way of the desired entry.
     * @return entry The entry pointer.
     */
    ReplaceableEntry* getEntry(const uint32_t set, const uint32_t way) const;

    /**
     * Generate the tag from the given address.
     *
     * @param addr The address to get the tag from.
     * @return The tag of the address.
     */
    Addr extractTag(const Addr addr) const;

    /**
     * Find all possible entries for insertion and replacement of an address.
     * Should be called immediately before ReplacementPolicy's findVictim()
     * not to break cache resizing.
     *
     * @param addr The addr to a find possible entries for.
     * @return The possible entries.
     */
    virtual std::vector<ReplaceableEntry*> getPossibleEntries(const Addr addr)
                                                                    const = 0;

    /**
     * Regenerate an entry's address from its tag and assigned indexing bits.
     *
     * @param tag The tag bits.
     * @param entry The entry.
     * @return the entry's original address.
     */
    virtual Addr regenerateAddr(const Addr tag, const ReplaceableEntry* entry)
                                                                    const = 0;
};

#endif //__MEM_CACHE_INDEXING_POLICIES_BASE_HH__
