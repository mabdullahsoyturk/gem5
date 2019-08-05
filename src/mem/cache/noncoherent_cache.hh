/**
 * @file
 * Specifies a non-coherent cache. The non-coherent cache is expected to be located below the point of coherency. 
 * All valid blocks in the non-coherent cache can always be written to without any prior invalidations or snoops.
 */

#ifndef __MEM_CACHE_NONCOHERENT_CACHE_HH__
#define __MEM_CACHE_NONCOHERENT_CACHE_HH__

#include "base/logging.hh"
#include "base/types.hh"
#include "mem/cache/base.hh"
#include "mem/packet.hh"

class CacheBlk;
class MSHR;
struct NoncoherentCacheParams;

class NoncoherentCache : public BaseCache
{
  protected:
    bool access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat, PacketList &writebacks) override;

    void handleTimingReqMiss(PacketPtr pkt, CacheBlk *blk, Tick forward_time, Tick request_time) override;

    void recvTimingReq(PacketPtr pkt) override;

    void doWritebacks(PacketList& writebacks,Tick forward_time) override;

    void doWritebacksAtomic(PacketList& writebacks) override;

    void serviceMSHRTargets(MSHR *mshr, const PacketPtr pkt, CacheBlk *blk) override;

    void recvTimingResp(PacketPtr pkt) override;

    void recvTimingSnoopReq(PacketPtr pkt) override {
        panic("Unexpected timing snoop request %s", pkt->print());
    }

    void recvTimingSnoopResp(PacketPtr pkt) override {
        panic("Unexpected timing snoop response %s", pkt->print());
    }

    Cycles handleAtomicReqMiss(PacketPtr pkt, CacheBlk *&blk, PacketList &writebacks) override;

    Tick recvAtomic(PacketPtr pkt) override;

    Tick recvAtomicSnoop(PacketPtr pkt) override {
        panic("Unexpected atomic snoop request %s", pkt->print());
    }

    void functionalAccess(PacketPtr pkt, bool from_cpu_side) override;

    void satisfyRequest(PacketPtr pkt, CacheBlk *blk,bool deferred_response = false,bool pending_downgrade = false) override;

    /*
     * Creates a new packet with the request to be send to the memory below. The noncoherent cache is below the point of 
     * coherence and therefore all fills bring in writable, therefore the needs_writeble parameter is ignored.
     */
    PacketPtr createMissPacket(PacketPtr cpu_pkt, CacheBlk *blk, bool needs_writable, bool is_whole_line_write) const override;

    M5_NODISCARD PacketPtr evictBlock(CacheBlk *blk) override;

  public:
    NoncoherentCache(const NoncoherentCacheParams *p);
};

#endif // __MEM_CACHE_NONCOHERENTCACHE_HH__
