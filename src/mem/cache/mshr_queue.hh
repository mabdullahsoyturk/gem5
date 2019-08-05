/** @file
 * Declaration of a structure to manage MSHRs.
 */

#ifndef __MEM_CACHE_MSHR_QUEUE_HH__
#define __MEM_CACHE_MSHR_QUEUE_HH__

#include <string>

#include "base/types.hh"
#include "mem/cache/mshr.hh"
#include "mem/cache/queue.hh"
#include "mem/packet.hh"

/**
 * A Class for maintaining a list of pending and allocated memory requests.
 */
class MSHRQueue : public Queue<MSHR>
{
  private:

    /**
     * The number of entries to reserve for future demand accesses.
     * Prevent prefetcher from taking all mshr entries
     */
    const int demandReserve;

  public:

    /**
     * Create a queue with a given number of entries.
     * @param num_entrys The number of entries in this queue.
     * @param reserve The minimum number of entries needed to satisfy
     * any access.
     * @param demand_reserve The minimum number of entries needed to satisfy
     * demand accesses.
     */
    MSHRQueue(const std::string &_label, int num_entries, int reserve,
              int demand_reserve);

    /**
     * Allocates a new MSHR for the request and size. This places the request
     * as the first target in the MSHR.
     *
     * @param blk_addr The address of the block.
     * @param blk_size The number of bytes to request.
     * @param pkt The original miss.
     * @param when_ready When should the MSHR be ready to act upon.
     * @param order The logical order of this MSHR
     * @param alloc_on_fill Should the cache allocate a block on fill
     *
     * @return The a pointer to the MSHR allocated.
     *
     * @pre There are free entries.
     */
    MSHR *allocate(Addr blk_addr, unsigned blk_size, PacketPtr pkt,
                   Tick when_ready, Counter order, bool alloc_on_fill);

    /**
     * Moves the MSHR to the front of the pending list if it is not
     * in service.
     * @param mshr The entry to move.
     */
    void moveToFront(MSHR *mshr);

    /**
     * Adds a delay to the provided MSHR and moves MSHRs that will be
     * ready earlier than this entry to the top of the list
     *
     * @param mshr that needs to be delayed
     * @param delay_ticks ticks of the desired delay
     */
    void delay(MSHR *mshr, Tick delay_ticks);

    /**
     * Mark the given MSHR as in service. This removes the MSHR from the
     * readyList or deallocates the MSHR if it does not expect a response.
     *
     * @param mshr The MSHR to mark in service.
     * @param pending_modified_resp Whether we expect a modified response
     *                              from another cache
     */
    void markInService(MSHR *mshr, bool pending_modified_resp);

    /**
     * Mark an in service entry as pending, used to resend a request.
     * @param mshr The MSHR to resend.
     */
    void markPending(MSHR *mshr);

    /**
     * Deallocate top target, possibly freeing the MSHR
     * @return if MSHR queue is no longer full
     */
    bool forceDeallocateTarget(MSHR *mshr);

    /**
     * Returns true if the pending list is not empty.
     * @return True if there are outstanding requests.
     */
    bool havePending() const
    {
        return !readyList.empty();
    }

    /**
     * Returns true if sufficient mshrs for prefetch.
     * @return True if sufficient mshrs for prefetch.
     */
    bool canPrefetch() const
    {
        // @todo we may want to revisit the +1, currently added to
        // keep regressions unchanged
        return (allocated < numEntries - (numReserve + 1 + demandReserve));
    }
};

#endif //__MEM_CACHE_MSHR_QUEUE_HH__
