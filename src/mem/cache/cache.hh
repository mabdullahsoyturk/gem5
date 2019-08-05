#ifndef __MEM_CACHE_CACHE_HH__
#define __MEM_CACHE_CACHE_HH__

#include <cstdint>
#include <unordered_set>

#include "base/types.hh"
#include "mem/cache/base.hh"
#include "mem/packet.hh"

class CacheBlk;
struct CacheParams;
class MSHR;

class Cache : public BaseCache
{
  protected:
    /**
     * This cache should allocate a block on a line-sized write miss.
     */
    const bool doFastWrites;
    
    /**
     * Store the outstanding requests that we are expecting snoop
     * responses from so we can determine which snoop responses we
     * generated and which ones were merely forwarded.
     */
    std::unordered_set<RequestPtr> outstandingSnoop;

  protected:
    /**
     * Turn line-sized writes into WriteInvalidate transactions.
     */
    void promoteWholeLineWrites(PacketPtr pkt);

    bool access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,PacketList &writebacks) override;

    void handleTimingReqHit(PacketPtr pkt, CacheBlk *blk,Tick request_time) override;

    void handleTimingReqMiss(PacketPtr pkt, CacheBlk *blk,Tick forward_time,Tick request_time) override;

    void recvTimingReq(PacketPtr pkt) override;

    void doWritebacks(PacketList& writebacks, Tick forward_time) override;

    void doWritebacksAtomic(PacketList& writebacks) override;

    void serviceMSHRTargets(MSHR *mshr, const PacketPtr pkt,CacheBlk *blk) override;

    void recvTimingSnoopReq(PacketPtr pkt) override;

    void recvTimingSnoopResp(PacketPtr pkt) override;

    Cycles handleAtomicReqMiss(PacketPtr pkt, CacheBlk *&blk,PacketList &writebacks) override;

    Tick recvAtomic(PacketPtr pkt) override;

    Tick recvAtomicSnoop(PacketPtr pkt) override;

    void satisfyRequest(PacketPtr pkt, CacheBlk *blk,bool deferred_response = false,bool pending_downgrade = false) override;

    void doTimingSupplyResponse(PacketPtr req_pkt, const uint8_t *blk_data,bool already_copied, bool pending_inval);

    /**
     * Perform an upward snoop if needed, and update the block state
     * (possibly invalidating the block). Also create a response if required.
     *
     * @param pkt Snoop packet
     * @param blk Cache block being snooped
     * @param is_timing Timing or atomic for the response
     * @param is_deferred Is this a deferred snoop or not?
     * @param pending_inval Do we have a pending invalidation?
     *
     * @return The snoop delay incurred by the upwards snoop
     */
    uint32_t handleSnoop(PacketPtr pkt, CacheBlk *blk,bool is_timing, bool is_deferred, bool pending_inval);

    M5_NODISCARD PacketPtr evictBlock(CacheBlk *blk) override;

    /**
     * Create a CleanEvict request for the given block.
     *
     * @param blk The block to evict.
     * @return The CleanEvict request for the block.
     */
    PacketPtr cleanEvictBlk(CacheBlk *blk);

    PacketPtr createMissPacket(PacketPtr cpu_pkt, CacheBlk *blk,bool needs_writable,bool is_whole_line_write) const override;

    /**
     * Send up a snoop request and find cached copies. If cached copies are
     * found, set the BLOCK_CACHED flag in pkt.
     */
    bool isCachedAbove(PacketPtr pkt, bool is_timing = true);

  public:
    Cache(const CacheParams *p);

    /**
     * Take an MSHR, turn it into a suitable downstream packet, and
     * send it out. This construct allows a queue entry to choose a suitable
     * approach based on its type.
     *
     * @param mshr The MSHR to turn into a packet and send
     * @return True if the port is waiting for a retry
     */
    bool sendMSHRQueuePacket(MSHR* mshr) override;
};

#endif // __MEM_CACHE_CACHE_HH__
