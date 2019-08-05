#include "mem/cache/mshr_queue.hh"
#include <cassert>
#include "mem/cache/mshr.hh"

MSHRQueue::MSHRQueue(const std::string &_label, int num_entries, int reserve, int demand_reserve)
    : Queue<MSHR>(_label, num_entries, reserve), demandReserve(demand_reserve)
{}

MSHR * MSHRQueue::allocate(Addr blk_addr, unsigned blk_size, PacketPtr pkt,
                    Tick when_ready, Counter order, bool alloc_on_fill)
{
    assert(!freeList.empty());
    MSHR *mshr = freeList.front();
    assert(mshr->getNumTargets() == 0);
    freeList.pop_front();

    mshr->allocate(blk_addr, blk_size, pkt, when_ready, order, alloc_on_fill);
    mshr->allocIter = allocatedList.insert(allocatedList.end(), mshr);
    mshr->readyIter = addToReadyList(mshr);

    allocated += 1;
    return mshr;
}

void MSHRQueue::moveToFront(MSHR *mshr)
{
    if (!mshr->inService) {
        assert(mshr == *(mshr->readyIter));
        readyList.erase(mshr->readyIter);
        mshr->readyIter = readyList.insert(readyList.begin(), mshr);
    }
}

void MSHRQueue::delay(MSHR *mshr, Tick delay_ticks)
{
    mshr->delay(delay_ticks);
    auto it = std::find_if(mshr->readyIter, readyList.end(),
                            [mshr] (const MSHR* _mshr) {
                                return mshr->readyTime >= _mshr->readyTime;
                            });
    readyList.splice(it, readyList, mshr->readyIter);
}

void MSHRQueue::markInService(MSHR *mshr, bool pending_modified_resp)
{
    mshr->markInService(pending_modified_resp);
    readyList.erase(mshr->readyIter);
    _numInService += 1;
}

void MSHRQueue::markPending(MSHR *mshr)
{
    assert(mshr->inService);
    mshr->inService = false;
    --_numInService;

    mshr->readyIter = addToReadyList(mshr);
}

bool MSHRQueue::forceDeallocateTarget(MSHR *mshr)
{
    bool was_full = isFull();
    assert(mshr->hasTargets());
    
    mshr->popTarget(); // Pop the prefetch off of the target list
    
    if (!mshr->hasTargets() && !mshr->promoteDeferredTargets()) { // Delete mshr if no remaining targets
        deallocate(mshr);
    }

    return was_full && !isFull(); // Notify if MSHR queue no longer full
}
