#include "mem/cache/cache_blk.hh"

#include "base/cprintf.hh"

void
CacheBlk::insert(const Addr tag, const bool is_secure, const int src_master_ID, const uint32_t task_ID)
{
    assert(status == 0); // Make sure that the block has been properly invalidated

    this->tag = tag; // Set block tag
    srcMasterId = src_master_ID; // Set source requestor ID
    task_id = task_ID; // Set task ID
    tickInserted = curTick(); // Set insertion tick as current tick
    refCount = 1; // Insertion counts as a reference to the block

    if (is_secure) { // Set secure state
        setSecure();
    }

    setValid(); // Validate block
}

void
CacheBlkPrintWrapper::print(std::ostream &os, int verbosity, const std::string &prefix) const
{
    ccprintf(os, "%sblk %c%c%c%c\n", prefix,
             blk->isValid()    ? 'V' : '-',
             blk->isWritable() ? 'E' : '-',
             blk->isDirty()    ? 'M' : '-',
             blk->isSecure()   ? 'S' : '-');
}

