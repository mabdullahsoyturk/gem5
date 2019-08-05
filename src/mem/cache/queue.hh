/** @file
 * Declaration of a high-level queue structure
 */

#ifndef __MEM_CACHE_QUEUE_HH__
#define __MEM_CACHE_QUEUE_HH__

#include <cassert>
#include <string>
#include <type_traits>

#include "base/logging.hh"
#include "base/trace.hh"
#include "base/types.hh"
#include "debug/Drain.hh"
#include "mem/cache/queue_entry.hh"
#include "mem/packet.hh"
#include "sim/core.hh"
#include "sim/drain.hh"

/**
 * A high-level queue interface, to be used by both the MSHR queue and
 * the write buffer.
 */
template<class Entry>
class Queue : public Drainable
{
    static_assert(std::is_base_of<QueueEntry, Entry>::value,
        "Entry must be derived from QueueEntry");

  protected:
    /** Local label (for functional print requests) */
    const std::string label;

    /**
     * The total number of entries in this queue. This number is set
     * as the number of entries requested plus any reserve. This
     * allows for the same number of effective entries while still
     * maintaining an overflow reserve.
     */
    const int numEntries;

    /**
     * The number of entries to hold as a temporary overflow
     * space. This is used to allow temporary overflow of the number
     * of entries as we only check the full condition under certain
     * conditions.
     */
    const int numReserve;

    /**  Actual storage. */
    std::vector<Entry> entries;
    /** Holds pointers to all allocated entries. */
    typename Entry::List allocatedList;
    /** Holds pointers to entries that haven't been sent downstream. */
    typename Entry::List readyList;
    /** Holds non allocated entries. */
    typename Entry::List freeList;

    typename Entry::Iterator addToReadyList(Entry* entry)
    {
        if (readyList.empty() ||
            readyList.back()->readyTime <= entry->readyTime) {
            return readyList.insert(readyList.end(), entry);
        }

        for (auto i = readyList.begin(); i != readyList.end(); ++i) {
            if ((*i)->readyTime > entry->readyTime) {
                return readyList.insert(i, entry);
            }
        }
        panic("Failed to add to ready list.");
    }

    /** The number of entries that are in service. */
    int _numInService;

    /** The number of currently allocated entries. */
    int allocated;

  public:

    /**
     * Create a queue with a given number of entries.
     *
     * @param num_entries The number of entries in this queue.
     * @param reserve The extra overflow entries needed.
     */
    Queue(const std::string &_label, int num_entries, int reserve) :
        label(_label), numEntries(num_entries + reserve),
        numReserve(reserve), entries(numEntries), _numInService(0),
        allocated(0)
    {
        for (int i = 0; i < numEntries; ++i) {
            freeList.push_back(&entries[i]);
        }
    }

    bool isEmpty() const
    {
        return allocated == 0;
    }

    bool isFull() const
    {
        return (allocated >= numEntries - numReserve);
    }

    int numInService() const
    {
        return _numInService;
    }

    /**
     * Find the first entry that matches the provided address.
     *
     * @param blk_addr The block address to find.
     * @param is_secure True if the target memory space is secure.
     * @param ignore_uncacheable Should uncacheables be ignored or not
     * @return Pointer to the matching WriteQueueEntry, null if not found.
     */
    Entry* findMatch(Addr blk_addr, bool is_secure,
                     bool ignore_uncacheable = true) const
    {
        for (const auto& entry : allocatedList) {
            // we ignore any entries allocated for uncacheable
            // accesses and simply ignore them when matching, in the
            // cache we never check for matches when adding new
            // uncacheable entries, and we do not want normal
            // cacheable accesses being added to an WriteQueueEntry
            // serving an uncacheable access
            if (!(ignore_uncacheable && entry->isUncacheable()) &&
                entry->matchBlockAddr(blk_addr, is_secure)) {
                return entry;
            }
        }
        return nullptr;
    }

    bool trySatisfyFunctional(PacketPtr pkt)
    {
        pkt->pushLabel(label);
        for (const auto& entry : allocatedList) {
            if (entry->matchBlockAddr(pkt) &&
                entry->trySatisfyFunctional(pkt)) {
                pkt->popLabel();
                return true;
            }
        }
        pkt->popLabel();
        return false;
    }

    /**
     * Find any pending requests that overlap the given request of a
     * different queue.
     *
     * @param entry The entry to be compared against.
     * @return A pointer to the earliest matching entry.
     */
    Entry* findPending(const QueueEntry* entry) const
    {
        for (const auto& ready_entry : readyList) {
            if (ready_entry->conflictAddr(entry)) {
                return ready_entry;
            }
        }
        return nullptr;
    }

    /**
     * Returns the WriteQueueEntry at the head of the readyList.
     * @return The next request to service.
     */
    Entry* getNext() const
    {
        if (readyList.empty() || readyList.front()->readyTime > curTick()) {
            return nullptr;
        }
        return readyList.front();
    }

    Tick nextReadyTime() const
    {
        return readyList.empty() ? MaxTick : readyList.front()->readyTime;
    }

    /**
     * Removes the given entry from the queue. This places the entry
     * on the free list.
     *
     * @param entry
     */
    void deallocate(Entry *entry)
    {
        allocatedList.erase(entry->allocIter);
        freeList.push_front(entry);
        allocated--;
        if (entry->inService) {
            _numInService--;
        } else {
            readyList.erase(entry->readyIter);
        }
        entry->deallocate();
        if (drainState() == DrainState::Draining && allocated == 0) {
            // Notify the drain manager that we have completed
            // draining if there are no other outstanding requests in
            // this queue.
            DPRINTF(Drain, "Queue now empty, signalling drained\n");
            signalDrainDone();
        }
    }

    DrainState drain() override
    {
        return allocated == 0 ? DrainState::Drained : DrainState::Draining;
    }
};

#endif //__MEM_CACHE_QUEUE_HH__
