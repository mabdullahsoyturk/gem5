/**
 * @file
 * Definitions of a set associative indexing policy.
 */

#include "mem/cache/tags/indexing_policies/set_associative.hh"

#include "mem/cache/replacement_policies/replaceable_entry.hh"

SetAssociative::SetAssociative(const Params *p)
    : BaseIndexingPolicy(p)
{
}

uint32_t
SetAssociative::extractSet(const Addr addr) const
{
    return (addr >> setShift) & setMask;
}

Addr
SetAssociative::regenerateAddr(const Addr tag, const ReplaceableEntry* entry)
                                                                        const
{
    return (tag << tagShift) | (entry->getSet() << setShift);
}

std::vector<ReplaceableEntry*>
SetAssociative::getPossibleEntries(const Addr addr) const
{
    return sets[extractSet(addr)];
}

SetAssociative*
SetAssociativeParams::create()
{
    return new SetAssociative(this);
}
