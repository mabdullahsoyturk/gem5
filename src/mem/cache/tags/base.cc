#include "mem/cache/tags/base.hh"

#include <cassert>

#include "base/types.hh"
#include "mem/cache/replacement_policies/replaceable_entry.hh"
#include "mem/cache/tags/indexing_policies/base.hh"
#include "mem/request.hh"
#include "sim/core.hh"
#include "sim/sim_exit.hh"
#include "sim/system.hh"
#include "debug/Cache.hh"

BaseTags::BaseTags(const Params *p)
    : ClockedObject(p), blkSize(p->block_size), blkMask(blkSize - 1),
      size(p->size), lookupLatency(p->tag_latency),
      system(p->system), indexingPolicy(p->indexing_policy),
      warmupBound((p->warmup_percentage/100.0) * (p->size / p->block_size)),
      warmedUp(false), numBlocks(p->size / p->block_size),
      dataBlks(new uint8_t[p->size]) // Allocate data storage in one big chunk
{
}

ReplaceableEntry*
BaseTags::findBlockBySetAndWay(int set, int way) const
{
    return indexingPolicy->getEntry(set, way);
}

CacheBlk*
BaseTags::findBlock(Addr addr, bool is_secure) const
{
    // Extract block tag
    Addr tag = extractTag(addr);

    // Find possible entries that may contain the given address
    const std::vector<ReplaceableEntry*> entries =
        indexingPolicy->getPossibleEntries(addr);

    // Search for block
    for (const auto& location : entries) {
        CacheBlk* blk = static_cast<CacheBlk*>(location);
        if ((blk->tag == tag) && blk->isValid() &&
            (blk->isSecure() == is_secure)) {
            return blk;
        }
    }

    // Did not find block
    return nullptr;
}

void
BaseTags::insertBlock(const PacketPtr pkt, CacheBlk *blk)
{
    assert(!blk->isValid());

    // Previous block, if existed, has been removed, and now we have
    // to insert the new one

    // Deal with what we are bringing in
    MasterID master_id = pkt->req->masterId();
    assert(master_id < system->maxMasters());
    occupancies[master_id]++;

    // Insert block with tag, src master id and task id
    blk->insert(extractTag(pkt->getAddr()), pkt->isSecure(), master_id,
                pkt->req->taskId());

    // Check if cache warm up is done
    if (!warmedUp && tagsInUse.value() >= warmupBound) {
        warmedUp = true;
        warmupCycle = curTick();
    }

    // We only need to write into one tag and one data block.
    tagAccesses += 1;
    dataAccesses += 1;
}

Addr
BaseTags::extractTag(const Addr addr) const
{
    return indexingPolicy->extractTag(addr);
}

void
BaseTags::cleanupRefsVisitor(CacheBlk &blk)
{
    if (blk.isValid()) {
        totalRefs += blk.refCount;
        ++sampledRefs;
    }
}

void
BaseTags::cleanupRefs()
{
    forEachBlk([this](CacheBlk &blk) { cleanupRefsVisitor(blk); });
}

void
BaseTags::computeStatsVisitor(CacheBlk &blk)
{
    if (blk.isValid()) {
        assert(blk.task_id < ContextSwitchTaskId::NumTaskId);
        occupanciesTaskId[blk.task_id]++;
        assert(blk.tickInserted <= curTick());
        Tick age = curTick() - blk.tickInserted;

        int age_index;
        if (age / SimClock::Int::us < 10) { // <10us
            age_index = 0;
        } else if (age / SimClock::Int::us < 100) { // <100us
            age_index = 1;
        } else if (age / SimClock::Int::ms < 1) { // <1ms
            age_index = 2;
        } else if (age / SimClock::Int::ms < 10) { // <10ms
            age_index = 3;
        } else
            age_index = 4; // >10ms

        ageTaskId[blk.task_id][age_index]++;
    }
}

void
BaseTags::computeStats()
{
    for (unsigned i = 0; i < ContextSwitchTaskId::NumTaskId; ++i) {
        occupanciesTaskId[i] = 0;
        for (unsigned j = 0; j < 5; ++j) {
            ageTaskId[i][j] = 0;
        }
    }

    forEachBlk([this](CacheBlk &blk) { computeStatsVisitor(blk); });
}

std::string
BaseTags::print()
{
    std::string str;

    auto print_blk = [&str](CacheBlk &blk) {
        //if (blk.isValid())
            str += csprintf("\tBlock: %s\n", blk.print());
    };
    forEachBlk(print_blk);

    if (str.empty())
        str = "no valid tags\n";

    return str;
}

void
BaseTags::regStats()
{
    ClockedObject::regStats();

    using namespace Stats;

    tagsInUse
        .name(name() + ".tagsinuse")
        .desc("Cycle average of tags in use")
        ;

    totalRefs
        .name(name() + ".total_refs")
        .desc("Total number of references to valid blocks.")
        ;

    sampledRefs
        .name(name() + ".sampled_refs")
        .desc("Sample count of references to valid blocks.")
        ;

    avgRefs
        .name(name() + ".avg_refs")
        .desc("Average number of references to valid blocks.")
        ;

    avgRefs = totalRefs/sampledRefs;

    warmupCycle
        .name(name() + ".warmup_cycle")
        .desc("Cycle when the warmup percentage was hit.")
        ;

    occupancies
        .init(system->maxMasters())
        .name(name() + ".occ_blocks")
        .desc("Average occupied blocks per requestor")
        .flags(nozero | nonan)
        ;
    for (int i = 0; i < system->maxMasters(); i++) {
        occupancies.subname(i, system->getMasterName(i));
    }

    avgOccs
        .name(name() + ".occ_percent")
        .desc("Average percentage of cache occupancy")
        .flags(nozero | total)
        ;
    for (int i = 0; i < system->maxMasters(); i++) {
        avgOccs.subname(i, system->getMasterName(i));
    }

    avgOccs = occupancies / Stats::constant(numBlocks);

    occupanciesTaskId
        .init(ContextSwitchTaskId::NumTaskId)
        .name(name() + ".occ_task_id_blocks")
        .desc("Occupied blocks per task id")
        .flags(nozero | nonan)
        ;

    ageTaskId
        .init(ContextSwitchTaskId::NumTaskId, 5)
        .name(name() + ".age_task_id_blocks")
        .desc("Occupied blocks per task id")
        .flags(nozero | nonan)
        ;

    percentOccsTaskId
        .name(name() + ".occ_task_id_percent")
        .desc("Percentage of cache occupancy per task id")
        .flags(nozero)
        ;

    percentOccsTaskId = occupanciesTaskId / Stats::constant(numBlocks);

    tagAccesses
        .name(name() + ".tag_accesses")
        .desc("Number of tag accesses")
        ;

    dataAccesses
        .name(name() + ".data_accesses")
        .desc("Number of data accesses")
        ;

    registerDumpCallback(new BaseTagsDumpCallback(this));
    registerExitCallback(new BaseTagsCallback(this));
}
